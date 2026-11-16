/*
 * pulp_nn_conv_u8_u8_i8.c
 * Nazareno Bruschi <nazareno.bruschi@unibo.it>
 *
 * Copyright (C) 2019-2020 University of Bologna
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "pulp_nn_kernels.h"
#include "pulp_nn_utils.h"

void pulp_nn_conv(uint8_t *pIn,
                  uint8_t *pIm2ColBuffer,
                  int8_t *pBias,
                  uint8_t *pOut,
                  int8_t *pWeight,
                  uint16_t out_shift,
                  uint16_t dim_in_x,
                  uint16_t dim_in_y,
                  uint16_t ch_in,
                  uint16_t dim_out_x,
                  uint16_t dim_out_y,
                  uint16_t ch_out,
                  uint16_t dim_kernel_x,
                  uint16_t dim_kernel_y,
                  uint16_t padding_y_top,
                  uint16_t padding_y_bottom,
                  uint16_t padding_x_left,
                  uint16_t padding_x_right,
                  uint16_t stride_x,
                  uint16_t stride_y)
{
    uint16_t ch_in_r  = ch_in;
    uint16_t ch_out_r = ch_out;

    int core_id = pi_core_id();

    /*
     * Each core gets its own private slice of the shared im2col
     * buffer. The offset below is (core_id x slice size): the slice holds
     * 2 columns of (ch_in * dim_kernel_x * dim_kernel_y) bytes, i.e. the two
     * output pixels consumed by one call to pulp_nn_matmul().
     * Total buffer size = slice size x NB_CORES.
     */
    uint8_t *pIm2ColBase = pIm2ColBuffer + (2 * core_id * ch_in * dim_kernel_x * dim_kernel_y);
    int i_out_y, i_out_x, i_ker_y, i_ker_x;
    int Log2Core;

    /*
     * Work split across cores: output rows are divided into equal chunks.
     * If dim_out_y is not a multiple of NB_CORES, cores are paired and each
     * core of a pair takes half of the row (section 0 = left half,
     * section 1 = right half) to balance the load better.
     */
    uint8_t extra_chunk = ((dim_out_y & (NB_CORES - 1)) != 0);
    uint8_t extra_chunk_r;
    uint16_t dim_out_x_r;
    uint8_t section;
    int core_id_r;

    if (extra_chunk && dim_out_x > 1) {
        Log2Core      = log2(NB_CORES >> 1);
        core_id_r     = (core_id >> 1);
        dim_out_x_r   = (dim_out_x >> 1);
        section       = (core_id & 0x1);
        extra_chunk_r = ((dim_out_y & ((NB_CORES >> 1) - 1)) != 0);
    } else {
        Log2Core      = log2(NB_CORES);
        core_id_r     = core_id;
        dim_out_x_r   = dim_out_x;
        section       = 0;
        extra_chunk_r = extra_chunk;
        extra_chunk   = 0;
    }

    uint8_t flag_dim_out_x_odd = dim_out_x & 0x01;

    int chunk = (dim_out_y >> Log2Core) + extra_chunk_r;

    int start_pixel = min((chunk * core_id_r), dim_out_y);
    int stop_pixel  = min(start_pixel + chunk, dim_out_y);

    uint8_t *pIm2Col    = pIm2ColBase;
    uint8_t *pOutBuffer = pOut + (start_pixel * ch_out_r * dim_out_x) + (section * ch_out_r * dim_out_x_r);

    /* Loop over the output pixels assigned to this core */
    for (i_out_y = start_pixel; i_out_y < stop_pixel; i_out_y++) {

        for (i_out_x = (section * dim_out_x_r);
             i_out_x < (dim_out_x_r + (section * (dim_out_x_r + flag_dim_out_x_odd)));
             i_out_x++) {

            /*
             * im2col: copy the kernel window of this output pixel into the
             * current im2col column (ch_in * dim_kernel_x * dim_kernel_y bytes).
             * The branches below only differ in how they handle padding:
             *   - top / bottom padded rows: check every (y, x) position
             *   - left / right padded columns: check only x
             *   - interior: no padding, copy whole kernel rows at once
             * (In this lab padding is 0, so only the interior branch runs.)
             * These copies are pure data movement: they cost cycles but
             * perform no MACs.
             */
            if (i_out_y < padding_y_top) {

                for (i_ker_y = ((i_out_y * stride_y) - padding_y_top);
                     i_ker_y < ((i_out_y * stride_y) - padding_y_top + dim_kernel_y);
                     i_ker_y++) {

                    for (i_ker_x = ((i_out_x * stride_x) - padding_x_left);
                         i_ker_x < ((i_out_x * stride_x) - padding_x_left + dim_kernel_x);
                         i_ker_x++) {

                        if ((i_ker_y < 0) || (i_ker_y >= dim_in_y) || (i_ker_x < 0) || (i_ker_x >= dim_in_x)) {
                            pulp_zero_mem(pIm2Col, ch_in);
                        } else {
                            pulp_nn_im2col_u8_to_u8(
                                (uint8_t *)(pIn + ((i_ker_y * dim_in_x + i_ker_x) * ch_in_r)), pIm2Col, ch_in);
                        }

                        pIm2Col += ch_in;
                    }
                }
            } else if (i_out_y < dim_out_y - padding_y_bottom) {

                if (i_out_x < padding_x_left) {

                    for (i_ker_y = ((i_out_y * stride_y) - padding_y_top);
                         i_ker_y < ((i_out_y * stride_y) - padding_y_top + dim_kernel_y);
                         i_ker_y++) {

                        for (i_ker_x = ((i_out_x * stride_x) - padding_x_left);
                             i_ker_x < ((i_out_x * stride_x) - padding_x_left + dim_kernel_x);
                             i_ker_x++) {

                            if ((i_ker_x < 0) || (i_ker_x >= dim_in_x)) {
                                pulp_zero_mem(pIm2Col, ch_in);
                            } else {
                                pulp_nn_im2col_u8_to_u8(
                                    (uint8_t *)(pIn + ((i_ker_y * dim_in_x + i_ker_x) * ch_in_r)), pIm2Col, ch_in);
                            }

                            pIm2Col += ch_in;
                        }
                    }
                } else if (i_out_x < (dim_out_x - padding_x_right)) {

                    for (i_ker_y = ((i_out_y * stride_y) - padding_y_top);
                         i_ker_y < ((i_out_y * stride_y) - padding_y_top + dim_kernel_y);
                         i_ker_y++) {

                        pulp_nn_im2col_u8_to_u8(
                            (uint8_t *)pIn + (i_ker_y * dim_in_x + i_out_x * stride_x - padding_x_left) * ch_in_r,
                            pIm2Col,
                            ch_in * dim_kernel_x);

                        pIm2Col += (ch_in * dim_kernel_x);
                    }
                } else {

                    for (i_ker_y = ((i_out_y * stride_y) - padding_y_top);
                         i_ker_y < ((i_out_y * stride_y) - padding_y_top + dim_kernel_y);
                         i_ker_y++) {

                        for (i_ker_x = i_out_x * stride_x - padding_x_left;
                             i_ker_x < i_out_x * stride_x - padding_x_left + dim_kernel_x;
                             i_ker_x++) {

                            if ((i_ker_x < 0) || (i_ker_x >= dim_in_x)) {
                                pulp_zero_mem(pIm2Col, ch_in);
                            } else {
                                pulp_nn_im2col_u8_to_u8(
                                    (uint8_t *)pIn + (i_ker_y * dim_in_x + i_ker_x) * ch_in_r, pIm2Col, ch_in);
                            }

                            pIm2Col += ch_in;
                        }
                    }
                }
            } else {

                for (i_ker_y = ((i_out_y * stride_y) - padding_y_top);
                     i_ker_y < ((i_out_y * stride_y) - padding_y_top + dim_kernel_y);
                     i_ker_y++) {

                    for (i_ker_x = i_out_x * stride_x - padding_x_left;
                         i_ker_x < i_out_x * stride_x - padding_x_left + dim_kernel_x;
                         i_ker_x++) {

                        if (i_ker_y < 0 || (i_ker_y >= dim_in_y) || i_ker_x < 0 || i_ker_x >= dim_in_x) {
                            pulp_zero_mem(pIm2Col, ch_in);
                        } else {
                            pulp_nn_im2col_u8_to_u8(
                                (uint8_t *)pIn + (i_ker_y * dim_in_x + i_ker_x) * ch_in_r, pIm2Col, ch_in);
                        }

                        pIm2Col += ch_in;
                    }
                }
            }

            /*
             * Two columns ready (pIm2Col reached the end of this core's slice):
             * compute 2 output pixels x ch_out channels. Each output element
             * is a dot product of length ch_in * dim_kernel_x * dim_kernel_y.
             */
            if (pIm2Col == (pIm2ColBase + ((ch_in * dim_kernel_x * dim_kernel_y) << 1))) {
                pOutBuffer = pulp_nn_matmul(pIm2ColBase,
                                            pBias,
                                            pOutBuffer,
                                            pOutBuffer + ch_out_r,
                                            pWeight,
                                            out_shift,
                                            (ch_in * dim_kernel_x * dim_kernel_y),
                                            ch_out);

                pIm2Col = pIm2ColBase;
            }
        }

        /*
         * Leftover: if this core's part of the row had an odd number of pixels,
         * one column is still waiting in the buffer. Compute it here, one
         * output channel at a time (4 MACs per SumDotp4, plus a scalar tail
         * when the dot-product length is not a multiple of 4). This path is
         * less efficient than pulp_nn_matmul(): it reuses each weight load for
         * one pixel only instead of two.
         */
        if (pIm2Col != pIm2ColBase) {
            const int8_t *pA = pWeight;
            int i;

            for (i = 0; i < ch_out; i++) {
                int sum = 0;

                if (pBias != NULL) {
                    sum = ((int)(*pBias++));
                }

                uint8_t *pB             = pIm2ColBase;
                uint16_t col_cnt_im2col = ch_in * dim_kernel_x * dim_kernel_y >> 2;

                for (int j = 0; j < col_cnt_im2col; j++) {
                    v4s inA = *((v4s *)pA);
                    v4u inB = *((v4u *)pB);

                    sum = SumDotp4(inB, inA, sum);

                    pA += 4;
                    pB += 4;
                }

                col_cnt_im2col = (ch_in * dim_kernel_y * dim_kernel_x) & 0x3;
                while (col_cnt_im2col) {
                    int8_t inA1  = *pA++;
                    uint8_t inB1 = *pB++;
                    asm volatile("" : : : "memory");

                    sum += inA1 * inB1;

                    col_cnt_im2col--;
                }

                *pOutBuffer = (uint8_t)clip8(sum >> out_shift);
                pOutBuffer++;
            }
        }

        pOutBuffer += (extra_chunk * ((dim_out_x_r + ((1 - section) * flag_dim_out_x_odd)) * ch_out_r));
        pIm2Col = pIm2ColBase;
    }

    /* Wait for all cores: cores with less work idle here (counts in your cycles!) */
    pi_cl_team_barrier(0);
}
