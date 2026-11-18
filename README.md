# APAI-LAB07: Tiling

## Material

Assignment: [here](_docs/assignment.docx)
Slides: [here](_docs/slides.pdf)


## How to deliver the assignment:

Use Virtuale, upload only the assignment file named as follows: LAB07_APAI_<name_surname>.pdf


**Assignment DEADLINE: Sunday 22/11/2026 (end of day, 23:59)**

___

## Quickstart

You are encouraged to work on your own laptop using the provided devcontainer (Option A). If that is not possible, you can use the virtual machine available on the lab machines (Option B).

### Option A: Your own laptop (devcontainer, recommended)

Requirements: [Docker](https://docs.docker.com/get-docker/), [VSCode](https://code.visualstudio.com/) and the VSCode [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) extension.

1. Open a terminal and clone this repository:
```
git clone https://github.com/EEESlab/APAI26-LAB07-Tiling
cd APAI26-LAB07-Tiling/
```
2. Open VSCode with `code .`
3. On VSCode, click on 'Reopen in container' (the first time, the container image is downloaded, which may take a while).
4. Open a new terminal in VSCode: the PULP SDK is configured automatically.
5. Now you're ready to start!

On Windows, install [WSL 2](https://learn.microsoft.com/en-us/windows/wsl/install) first, and choose the Windows version of Docker Desktop (even if you work with WSL). If VSCode reports that Docker cannot be found, enable the WSL integration in Docker Desktop (Settings -> Resources -> WSL integration), then Apply & Restart.

### Option B: Lab machines (virtual machine)

The Ubuntu-based virtual machine already contains the PULP SDK and VSCode (used only to edit the code and open terminals), so no Docker container is needed.

1. From a browser **outside** the virtual machine, download the lab zip file from Virtuale (the only website reachable from the lab machines) and extract it (right-click -> Extract).
2. Go to `C:/VM_APAI` and double-click `PULP-box.ova`. When VirtualBox opens, click on "Finish"/"Fine".
3. Once the virtual machine is imported, start it with "Start"/"Avvia".
4. Log in with username `pulp-box` and password `pulp`.
5. From the VirtualBox top bar, open Machine -> File Manager (left panel: files of the PC, right panel: files of the VM). Enter the login credentials at the bottom of the right panel.
6. Drag and drop the extracted lab folder into a subdirectory of the VM home directory (you need write rights there).
7. Inside the VM, open a terminal and go to the lab folder with `cd`.
8. **[DO NOT FORGET]** Every time you open a new terminal run:
```
source setup_pulp_sdk.sh
```
9. Now you're ready to start!

## How to run the code

1. Select the exercise in [inc/main.h](inc/main.h) (`#define EXERCISE1`, `EXERCISE2` or `EXERCISE3`, exactly one).
2. Generate the layer data for C channels and an N x N **output** (the input is (N + K - 1) x (N + K - 1)):
```
python3 parameters_generate.py --channels=<C> --output-spatial-dimensions=<N>
```
3. Build and run:
```
make clean all run
```

Repeat step 2 every time you change the layer: the generated files ([inc/dims.h](inc/dims.h), [inc/input.h](inc/input.h), [inc/weights.h](inc/weights.h), [inc/output.h](inc/output.h)) hold the dimensions, the data and the expected output of the layer.

To check your by-hand memory and MAC computations:
```
python3 mem_MACs_computation.py <C> <N>
```

Useful notes:
- The code sections to complete are marked with `EXERCISE N` comments and `/* YOUR CODE HERE */`. An exercise can have several sections in different files, so search the whole project (`Ctrl+Shift+F` in VSCode).
- The program prints the performance counters of cluster core 0 around the whole layer (DMA transfers + convolution), then checks the output against the expected one.
- If the layer (or the tile) does not fit in L1, you get `ERROR: L1 allocation failed, the layer (or tile) does not fit in L1 (about 56kB usable). Use a larger tiling factor.`
- If `TILING_PARAMETER` does not divide N, you get `ERROR: TILING_PARAMETER must divide the output spatial dimension N = ...`.
- If the output is wrong (forgotten data regeneration, wrong tile overlap, wrong buffer sizes...), you get `ERROR: first mismatch at index ...` followed by `Received ... errors` (the total number of wrong output values).

## Lab overview

All exercises run a 3x3 convolution (stride 1, no padding, C input = C output channels) on the 8-core PULP cluster, with 64kB of L1 memory (about 56kB usable for the layer buffers, the rest holds the cluster stacks).

| Exercise | Features (set automatically in [inc/main.h](inc/main.h)) | Code to write |
|---|---|---|
| 1 - Profile L1 vs L2 | `USE_L1_MEM` (comment it out for the L2 runs) | L1 buffer sizes ([src/main.c](src/main.c)), performance counters ([src/layer_execution.c](src/layer_execution.c)) |
| 2 - Tiling | `USE_L1_MEM`, `ENABLE_TILING` | `TILING_PARAMETER` ([src/main.c](src/main.c)), single-buffered tile loop ([src/layer_execution.c](src/layer_execution.c)), tile overlap in `tile_load()` ([src/kernel.c](src/kernel.c)) |
| 3 - Double buffering | `USE_L1_MEM`, `ENABLE_TILING`, `DOUBLE_BUFFERING` | double-buffered L1 buffer sizes ([src/main.c](src/main.c)), double-buffered tile loop ([src/layer_execution.c](src/layer_execution.c)) |

The functions to load a tile, run the kernel, store a tile and wait for the DMA are in [src/kernel.c](src/kernel.c) (see [inc/kernel.h](inc/kernel.h)); the convolution kernel itself comes from PULP-NN ([src/pulp_nn_conv.c](src/pulp_nn_conv.c)).
