# README
This project includes two main executables:

1. first_experiment
2. platformsAndDeviced

## first_experiment
The first_experiment program is designed to execute operations on multiple devices (e.g., GPUs) and manage workloads efficiently. Specifically, it includes the following functionalities:
1. Run operations on a specific device: Specify which GPU (e.g., GPU1 or GPU2) will handle the workload.
2. Split workload evenly between GPUs: Divide the total images equally between two GPUs without considering their performance differences.
3. Balance workload based on performance: Allocate the workload between two GPUs proportionally, taking into account the observed performance of each device.SSH

```bash
./first_experiment
```

## platformsAndDeviced
The platformsAndDeviced program enumerates and displays the platforms and devices available on your system. This includes information about GPUs, CPUs, or any other OpenCL-compatible devices.

```bash
-f or --folder <path>
```
Specifies the folder containing the images to process. Default: ./images

```bash
-v or --verbose
```
Enables verbose output for detailed logging during execution.

```bash
-w or --watch
```
Activates monitoring of image processing performance.

```bash
-t or --threads
```
Enables multithreaded loading of images

```bash
-n or --num_images <number>
```
Specifies the total number of images to process. Default: 16384

```bash
-d or --device <device_option>
```
Specifies the device configuration for processing

```bash
./platformsAndDeviced
```

# Notes
Recompile the programs using:
```bash
make
```
Use the clean target in the Makefile to remove executables before recompiling:
```bash
make clean
```
