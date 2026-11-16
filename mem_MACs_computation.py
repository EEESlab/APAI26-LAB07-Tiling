# mem_MACs_computation.py
# Helper to check your by-hand computations: memory footprint and MACs of a
# KxK convolution with C input/output channels, output N x N, stride 1, no padding.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
import argparse

NB_CORES = 8


def create_layer(channels, spatial_dim, kernel_shape):
    n_in = spatial_dim + kernel_shape - 1
    inp = channels * n_in * n_in
    out = channels * spatial_dim * spatial_dim
    weights = channels * channels * kernel_shape * kernel_shape
    im2col = 2 * kernel_shape * kernel_shape * channels * NB_CORES
    macs = kernel_shape * kernel_shape * channels * spatial_dim * spatial_dim * channels
    print("Input {0}x{0}x{1}: {2} B".format(n_in, channels, inp))
    print("Output {0}x{0}x{1}: {2} B".format(spatial_dim, channels, out))
    print("Weights: {} B".format(weights))
    print("Mem (input+weights+output): {} B".format(inp + weights + out))
    print("im2col buffer: {} B".format(im2col))
    print("MACs: {}".format(macs))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('channels', type=int, help='Number of input and output channels')
    parser.add_argument('spatial_dimensions', type=int, help='Output spatial dimension')
    parser.add_argument('--kernel-shape', '-ks', dest='kernel_shape', type=int, default=3, help='Kernel size. Default: 3')
    args = parser.parse_args()
    create_layer(channels=args.channels, spatial_dim=args.spatial_dimensions, kernel_shape=args.kernel_shape)
