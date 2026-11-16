# APAI26-LAB07-Tiling

**Assignment DEADLINE: TBD**

## Material

Assignment: [here](_docs/assignment_lab07_old.docx)
Slides: [part 1](_docs/slides_part1_old.pdf), [part 2](_docs/slides_part2_old.pdf)


## How to deliver the assignment:

Use Virtuale, upload only the assignment file named as follows:

`LAB#_APAI_name.pdf`

___

## Quickstart

### How to set the environment (Your PC)

1. Open a terminal
2. Go into 'APAI-Docker' folder.
3. Open VSCode with 'code .'
4. On VSCode, click on 'Reopen in container'.
5. Open you new terminal in VSCode and launch the following commands to clone this repository:
```
git clone https://github.com/EEESlab/APAI26-LAB07-Tiling
cd APAI26-LAB07-Tiling/
```
6. Now you're ready to start!

### How to set the environment (LAB1)

1. Open the VirtualBox virtual machine
2. Open a terminal (CTRL+T or open terminal)
3. Go into 'APAI-Docker' folder.
4. Open VSCode with 'code .'
5. On VSCode, click on 'Reopen in container'.
6. Open you new terminal in VSCode and launch the following commands to clone this repository:
```
git clone https://github.com/EEESlab/APAI26-LAB07-Tiling
cd APAI26-LAB07-Tiling/
```
7. Now you're ready to start!

#### How to run the code
**[DO NOT FORGET]** Every time you open a new terminal run:

`source setup-pulp-sdk.sh`

1. Select the exercise in `inc/main.h` (`#define EXERCISE1`, `EXERCISE2` or `EXERCISE3`).
2. Generate the layer data. `N` is the **output** spatial dimension, the input is `N + K - 1`:
```
python3 parameters_generate.py --kernel-shape=3 --channels=<C> --output-spatial-dimensions=<N>
```
3. Build and run:
```
make clean all run
```

To check your by-hand memory and MAC computations: `python3 mem_MACs_computation.py <C> <N>`.

If the layer (or tile) does not fit in L1 you get:
`ERROR: L1 allocation failed, the layer (or tile) does not fit in L1 (64kB).`
If you forget to regenerate the data after changing the layer, you get output mismatches (`Received ... errors`).

___

## Exercises (overview)

All exercises use a 3x3 convolution, stride 1, no padding, C input = C output channels, L1 = 64kB.

| Exercise | Features (set automatically in `inc/main.h`) | Code to write |
|---|---|---|
| 1. Profile L1 vs L2 | `USE_L1_MEM` (comment it out for the L2 part) | L1 buffer sizes (`src/main.c`), performance counters (`src/layer_execution.c`) |
| 2. Tiling | `USE_L1_MEM`, `ENABLE_TILING` | `TILING_PARAMETER` (`src/main.c`), number of tiles and single-buffered tile loop (`src/layer_execution.c`), tile overlap in `tile_load()` (`src/kernel.c`) |
| 3. Double buffering | `USE_L1_MEM`, `ENABLE_TILING`, `DOUBLE_BUFFERING` | double-buffered L1 buffer sizes (`src/main.c`), double-buffered tile loop (`src/layer_execution.c`) |

Look for `/* YOUR CODE HERE */`.
