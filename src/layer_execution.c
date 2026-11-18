#include "main.h"
#include "network_desc.h"
#include "input.h"
#include "weights.h"
#include "output.h"
#include "kernel.h"

int layer_init()
{
#ifdef DEBUG
    printf("-> Entering Layer Initialization...\n");
#endif

    network_layers[0].input_data = input;
    if (network_layers[0].input_data == NULL) {
        return -L2_ERROR;
    }

    network_layers[0].param_data = weights;
    if (network_layers[0].param_data == NULL) {
        return -L2_ERROR;
    }

    network_layers[0].output_data =
        pi_l2_malloc((unsigned int)(network_layers[0].layer_dim.x_out * network_layers[0].layer_dim.y_out *
                                    network_layers[0].layer_dim.c_out * sizeof(unsigned char)));
    if (network_layers[0].output_data == NULL) {
        return -L2_ERROR;
    }

#ifdef DEBUG
    printf("-> Exiting Layer Initialization...\n");
#endif

    return 0;
}

/*
 * Cluster application entry point
 */
void layer_run()
{
#ifdef DEBUG
    printf("--> Entering Layer Running...\n");
#endif

    pi_cl_dma_cmd_t dma_cmd;

    int dma_copy_size =
        l1_layer.layer_dim.x_ker * l1_layer.layer_dim.y_ker * l1_layer.layer_dim.c_in * l1_layer.layer_dim.c_out;

    /*
     * Parameters copy: Let's keep it simple, avoid the tiling along channels. Move all the parameters in L1
     */
#ifdef USE_L1_MEM
    pi_cl_dma_cmd((unsigned int)network_layers[0].param_data,
                  (unsigned int)l1_layer.param_data,
                  dma_copy_size,
                  PI_CL_DMA_DIR_EXT2LOC,
                  &dma_cmd);
    pi_cl_dma_cmd_wait(&dma_cmd);
#else
    l1_layer.param_data = network_layers[0].param_data;
#endif

#ifndef USE_L1_MEM
    l1_layer.output_data = network_layers[0].output_data;
#endif

    /*
     * Tile loop bounds
     */
    int nb_h_tile = 0;
    int nb_w_tile = 0;

    /*
     * Cluster performance counters
     */
#ifdef PERFORMANCE
    /* EXERCISE 1: COMPLETE WITH THE PERFORMANCE COUNTERS INITIALIZATION
     * Look at the PMSIS performance API (pi_perf_*). Count active cycles, cycles and instructions. */
    pi_perf_conf(1 << /* YOUR CODE HERE */   // active cycles
                 | 1 << /* YOUR CODE HERE */ // cycles
                 | 1 << /* YOUR CODE HERE */ // instructions
    );
    /* YOUR CODE HERE */; // reset the counters
    /* YOUR CODE HERE */; // start the counters
#endif

#ifdef ENABLE_TILING
    nb_h_tile = network_layers[0].layer_dim.y_out / l1_layer.layer_dim.y_out;
    nb_w_tile = network_layers[0].layer_dim.x_out / l1_layer.layer_dim.x_out;
#else
    nb_h_tile = 1;
    nb_w_tile = 1;
#endif

    /*
     * Summary of the functions you can use (see inc/kernel.h and src/kernel.c):
     *  - void tile_load(int h_tile_idx, int w_tile_idx, int c_tile_idx, int buffer_idx)  -->  starts DMA transfer (L2->L1) for a specific tile (you can select which one with the indexes)
     *  - void kernel_run(int buffer_idx)                                                 -->  Runs inference for the selected buffer_idx
     *  - void tile_store(int h_tile_idx, int w_tile_idx, int c_tile_idx, int buffer_idx) -->  starts DMA transfer (L1->L2) of the output of a specific tile
     *  - void tile_load_store_wait()                                                     -->  waits until all started loads and stores are complete
     * We do not tile along the channels, so c_tile_idx is always 0.
     */

#if !defined(ENABLE_TILING)
    /*
     * No tiling, the whole layer is a single tile.
     * Load -> wait -> compute -> store -> wait.
     *
     * Arguments of tile_load / tile_store: (h_tile_idx, w_tile_idx, c_tile_idx, buffer_idx)
     *   - h_tile_idx = 0, w_tile_idx = 0: there is only one tile, the one at row 0, column 0
     *     (the tile indexes select WHERE in L2 the tile starts; with a single tile that is the start)
     *   - c_tile_idx = 0: channels are never tiled
     *   - buffer_idx = 0: only one L1 buffer is used (it becomes 0 or 1 only with double buffering)
     */
    tile_load(0, 0, 0, 0);
    tile_load_store_wait();
    kernel_run(0);
    tile_store(0, 0, 0, 0);
    tile_load_store_wait();

#elif !defined(DOUBLE_BUFFERING)
    /** EXERCISE 2: implement single-buffered tiling
     *
     *  Same steps as in EXERCISE 1 (see above), but repeated for every tile.
     *  There is only one L1 buffer, so buffer_idx is always 0.
     *
     *    for each tile (h, w):
     *        load tile (h, w), wait
     *        kernel_run
     *        store tile (h, w), wait
     */
    for (int h_tile_idx = 0; h_tile_idx < nb_h_tile; h_tile_idx++) {
        for (int w_tile_idx = 0; w_tile_idx < nb_w_tile; w_tile_idx++) {
            /* YOUR CODE HERE */;
        }
    }

#else
    /** EXERCISE 3: implement double buffering
     *
     *  Double buffering main steps:
     *
     *    load tiles[0] -> current_buffer
     *    for i in range(len(tiles)):
     *        wait for previous loads and stores
     *        load tiles[i+1] -> next_buffer
     *        kernel_run current_buffer
     *        store current_buffer -> tiles[i]
     *    wait for previous loads and stores before exiting
     */
    const int nb_tiles = nb_h_tile * nb_w_tile;
    int current_buffer_idx     = 0;

    /*
     * Reminder of arguments for tile_load / tile_store: (h_tile_idx, w_tile_idx, c_tile_idx, buffer_idx)
     *   - h_tile_idx, w_tile_idx: row and column of the tile to load/store (they select WHERE in L2 it starts)
     *   - c_tile_idx = 0: channels are never tiled
     *   - buffer_idx: which of the two L1 buffers to use (0 or 1). It alternates between consecutive tiles:
     *     while the kernel works on current_buffer_idx, the next tile is loaded into next_buffer_idx
     */
    // Load tile 0's input data (outside the for loop)
    /* YOUR CODE HERE */;

    for (int h_tile_idx = 0; h_tile_idx < nb_h_tile; h_tile_idx++) {
        for (int w_tile_idx = 0; w_tile_idx < nb_w_tile; w_tile_idx++) {
            // Compute the next buffer index, relative to the current one (0 or 1)
            const int next_buffer_idx = (current_buffer_idx + 1) % 2;

            // Compute the next tile indexes, relative to the current ones (h_tile_idx, w_tile_idx)
            const int next_w_tile_idx = (w_tile_idx + 1) % nb_w_tile;

            // If the next_w_tile is 0, that means we went into a new row, so the next_h_tile is h_tile + 1
            const int next_h_tile_idx = h_tile_idx + (next_w_tile_idx == 0 ? 1 : 0);

            // Compute the next tile index, relative to the current one (h_tile_idx, w_tile_idx)
            const int next_tile_idx   = next_h_tile_idx * nb_w_tile + next_w_tile_idx;

            /*
             * Wait for the current tiles data
             * Note: we are waiting here and not right before the kernel, because
             * otherwise we would wait for the next tile too and ruin the point
             * of double buffering.
             */
            /* YOUR CODE HERE */;

            /*
             * Load next tiles input data from L2 into L1
             */
            // Check if there exists a 'next' tile
            if (next_tile_idx < nb_tiles) {
                /* YOUR CODE HERE */;
            }

            /*
             * Kernel execution
             */
            /* YOUR CODE HERE */;

            /*
             * Store output data back into L2 from L1
             */
            /* YOUR CODE HERE */;

            current_buffer_idx = next_buffer_idx;
        }
    }

    /*
     * Last wait for the DMA transfers
     */
    /* YOUR CODE HERE */;
#endif

#ifdef PERFORMANCE
    /* EXERCISE 1: COMPLETE WITH THE PERFORMANCE COUNTERS READING */

    /* YOUR CODE HERE */;  // stop the counters

    uint32_t instr_cnt      = /* YOUR CODE HERE */;
    uint32_t cycles_cnt     = /* YOUR CODE HERE */;
    uint32_t act_cycles_cnt = /* YOUR CODE HERE */;

    /* Counters of cluster core 0 (the core running layer_run): DMA load + conv + DMA store */
    printf("[Cluster core 0, layer_run]: instructions = %u, tot_cycles = %u, active_cycles = %u\n",
           (unsigned int)instr_cnt,
           (unsigned int)cycles_cnt,
           (unsigned int)act_cycles_cnt);
#endif

#ifdef DEBUG
    printf("--> Exiting Layer Running...\n");
#endif
}

/*
 * Check if the outputs are the same of the golden model
 */
int layer_check()
{
    int tot_layer_out_dim =
        network_layers[0].layer_dim.x_out * network_layers[0].layer_dim.y_out * network_layers[0].layer_dim.c_out;

    int errors = 0;
    for (int i = 0; i < tot_layer_out_dim; i++) {
        if (output[i] != network_layers[0].output_data[i]) {
            if (errors == 0) {
                printf("ERROR: first mismatch at index %d\n", i);
            }
            errors++;
        }
    }

    if (errors != 0) {
        printf("Received %d errors\n", errors);
        return -GENERAL_ERROR;
    }

    printf("Exiting layer with 0 errors\n");

    return 0;
}

int layer_free()
{
    const unsigned int im2col_size = (unsigned int)(2 * l1_layer.layer_dim.x_ker * l1_layer.layer_dim.y_ker *
                                                    l1_layer.layer_dim.c_in * NB_CORES * sizeof(unsigned char));
#ifdef USE_L1_MEM
    pi_l1_free(&cluster, l1_layer.input_data, l1_input_size);
    pi_l1_free(&cluster, l1_layer.param_data, l1_weights_size);
    pi_l1_free(&cluster, l1_layer.output_data, l1_output_size);
    pi_l1_free(&cluster, l1_layer.buffer_0, im2col_size);
#else
    pi_l2_free(l1_layer.buffer_0, im2col_size);
#endif
    pi_cluster_close(&cluster);
    return 0;
}
