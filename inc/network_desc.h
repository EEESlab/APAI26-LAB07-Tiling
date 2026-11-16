/*
 * Copyright (C) 2021 University of Bologna
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 *
 * Authors: Nazareno Bruschi, UniBo (<nazareno.bruschi@unibo.it>)
 *          Francesco Conti,  UniBo (<f.conti@unibo.it>)
 */

#ifndef __NETWORK_DESC_H__
#define __NETWORK_DESC_H__

/*
 * Depends from the model. Let's keep it simple and focus on the first conv layer.
 */

extern int NETWORK_IDS[NB_LAYER];
extern int NETWORK_TYPES[NB_LAYER];
extern int NETWORK_DIMS[NB_LAYER][NB_DIM_ARGS];

extern unsigned int L3_input[NB_LAYER];
extern unsigned int L3_output[NB_LAYER];
extern unsigned int L3_param[NB_LAYER];
extern PI_L2 network_layer_t network_layers[NB_LAYER];
extern PI_L1 network_layer_t l1_layer;
extern struct pi_device cluster;
extern unsigned int l1_input_size;
extern unsigned int l1_weights_size;
extern unsigned int l1_output_size;

#endif // __NETWORK_DESC_H__
