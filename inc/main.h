/*
 * Copyright (C) 2021 University of Bologna
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 *
 * Authors: Alessio Burrello, UniBo (<alessio.burrello@unibo.it>)
 *          Nazareno Bruschi, UniBo (<nazareno.bruschi@unibo.it>)
 *          Davide Nadalini,  UniBo (<d.nadalini@unibo.it>)
 *          Francesco Conti,  UniBo (<f.conti@unibo.it>)
 */

#ifndef __MAIN_H__
#define __MAIN_H__

#include "pmsis.h"
#include "pulp_nn_kernels.h"
#include "bsp/bsp.h"

/*
 * Select the exercise you are working on (exactly one).
 */
#define EXERCISE1
// #define EXERCISE2
// #define EXERCISE3

/*
 * Each exercise turns on the features it needs:
 *   USE_L1_MEM       - the whole layer (or each tile) is copied into L1 before running the kernel.
 *                      Without it, the kernel reads/writes directly from/to L2 (no tiling possible).
 *   ENABLE_TILING    - the output is split into TILING_PARAMETER x TILING_PARAMETER tiles (see main.c).
 *   DOUBLE_BUFFERING - two L1 buffers for input and output tiles, so DMA transfers overlap with compute.
 */
#if defined(EXERCISE1) + defined(EXERCISE2) + defined(EXERCISE3) != 1
#error "Select exactly one exercise in inc/main.h (EXERCISE1, EXERCISE2 or EXERCISE3)"
#endif

#if defined(EXERCISE1)
#define USE_L1_MEM // EXERCISE 1, steps 4-6: comment out this line to run the layer from L2
#elif defined(EXERCISE2)
#define USE_L1_MEM
#define ENABLE_TILING
#elif defined(EXERCISE3)
#define USE_L1_MEM
#define ENABLE_TILING
#define DOUBLE_BUFFERING
#endif

#ifdef DOUBLE_BUFFERING
#define NB_BUFFERS 2
#else
#define NB_BUFFERS 1
#endif

#define GENERAL_ERROR    1
#define FLASH_ERROR      2
#define FILESYSTEM_ERROR 3
#define RAM_ERROR        4
#define FILE_ERROR       5
#define L2_ERROR         6
#define CLUSTER_ERROR    7
#define CHECK_ERROR      8

void cluster_init();
int layer_init();
int layer_free();
void layer_run();
int layer_check();
int network_init();
int network_free();

#ifndef NB_LAYER
#define NB_LAYER 1
#endif

#ifndef NB_DIM_ARGS
#define NB_DIM_ARGS 14
#endif

#ifndef PERF_MEASUREMENT
#define PERF_MEASUREMENT
// #define DEBUG
#define PERFORMANCE
#endif

/*
 * Useful structures to handle the network objects (i.e. layers)
 */
/* Type of layer */
typedef enum layer_type_e { CONV, DEPTH, LINEAR, POOLING, RELU, SOFTMAX } layer_type_t;

/* Dimensions of the layer */
typedef struct layer_dim_s {
    int x_in;
    int y_in;
    int c_in;
    int x_out;
    int y_out;
    int c_out;
    int x_stride;
    int y_stride;
    int top_pad;
    int bot_pad;
    int lef_pad;
    int rig_pad;
    int x_ker;
    int y_ker;
} layer_dim_t;

/* network information and data pointers */
typedef struct network_layer_s {
    int id;
    layer_type_t type;
    layer_dim_t layer_dim;
    unsigned char *input_data;
    unsigned char *output_data;
    signed char *param_data;
    unsigned char *buffer_0;
    unsigned char *buffer_1;
} network_layer_t;

#define LAYER_ID(x)   NETWORK_IDS[(x)]
#define LAYER_TYPE(x) NETWORK_TYPES[(x)]
#define LAYER_DIM(x)  *(layer_dim_t *)NETWORK_DIMS[(x)]

#endif // __MAIN_H__
