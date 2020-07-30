# Hello vpp

This sample shows how to use the oneAPI Video Processing Library (oneVPL) to
perform simple video processing.

| Optimized for   | Description
|---------------- | ----------------------------------------
| OS              | Ubuntu* 18.04; Windows* 10
| Hardware        | Intel® Processor Graphics GEN9 or newer
| Software        | Intel® oneAPI Video Processing Library(oneVPL)

## What You Will Learn

- How to use oneVPL to resize an I420 raw video file.


## Time to Complete

  5 minutes


## Sample Details

This sample is a command line application that takes a file containing a raw
I420 format video elementary stream as an argument, processes it with oneVPL
writes the resized output to `out.i420` in I420 raw video format.


| Configuration     | Default setting
| ----------------- | ----------------------------------
| Target device     | CPU
| Input format      | I420
| Output format     | I420
| Output resolution | 640 x 480


## Build and Run the Sample

To build and run the sample you need to install prerequisite software and set up
your environment.

### Install Prerequisite Software

 - Intel® oneAPI Base Toolkit for Windows* or Linux*
 - [CMake](https://cmake.org)
 - A C/C++ compiler


### Set Up Your Environment

#### Linux

Run `setvars.sh` every time you open a new terminal window:

The `setvars.sh` script can be found in the root folder of your oneAPI
installation, which is typically `/opt/intel/inteloneapi/` when installed as
root or sudo, and `~/intel/inteloneapi/` when installed as a normal user.  If
you customized the installation folder, the `setvars.sh` is in your custom
location.

To use the tools, whether from the command line or using Eclipse, initialize
your environment. To do it in one step for all tools, use the included
environment variable setup utility: `source <install_dir>/setvars.sh`)

```
source <install_dir>/setvars.sh
```


#### Windows

Run `setvars.bat` every time you open a new command prompt:

The `setvars.bat` script can be found in the root folder of your oneAPI
installation, which is typically `C:\Program Files (x86)\inteloneapi\` when
installed using default options. If you customized the installation folder, the
`setvars.bat` is in your custom location.

To use the tools, whether from the command line or using Visual Studio,
initialize your environment. To do it in one step for all tools, use the
included environment variable setup utility: `<install_dir>\setvars.bat`)

```
<install_dir>\setvars.bat
```


### Build the Sample

From the directory containing this README:

```
mkdir build
cd build
cmake ..
```


### Run the Sample

```
cmake --build . --target run
```

The run target runs the sample executable with the argument
`<sample_dir>/content/cars_128x96.i420 128 96`.

You can find the output file `out.i420` in the build directory and its size is `640x480`.

### Display the Output

You can display the output with a video player that supports raw streams such as
FFplay. You can use the following command to display the output with FFplay:

```
ffplay -video_size 640x480 -pixel_format yuv420p -f rawvideo out.i420
```