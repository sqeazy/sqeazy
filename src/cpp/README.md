# Sqeazy #

**Sqeazy** - Fast and Flexible Volume Compression Library

A fast and flexible compression library for 2D/3D/ND volume data.
The core library is written in C++ with bindings for C, Java (to be implemented), and Python Java (to be implemented). Binaries are provided for Win (not implemented yet), OSX (works, but not all tests pass), and Linux (works).

# Dependencies

To build sqeazy, the cmake build system (any version higher than 2.8) is required as well as a gcc or clang compiler and affiliated linkers.

## libsqy

* lz4
* boost (>= 1.55)
* hdf5 (>= 1.8)
* ffmpeg (>= 2.5.8) with libx256 (>= 1.7)

## sqy (application similar to zip/tar/...)

* lz4
* libtiff
* boost (>= 1.55)
* hdf5 (>1.8)
* ffmpeg (>= 2.5.8) with libx256 (>= 1.7)

# General Remarks

Sqeazy is currently undergoing constant development. If you want to test-drive it, use the `sqy` application in `bench/`. Beware of using it for production data, we guarantee nothing in this case! We intend sqeazy to be build out-of-source.

## Build instructions

In order to build sqeazy, use the following (tested on Linux and OSX):

```bash
$ cd sqeazy
$ mkdir build
$ cd build
$ cmake <flags> ..
$ make 
```

The are some cmake build flags that are supported/required:
* (required if not in environment) `LZ4_LIB_PATH` the location of the lz4 static and dynamic libraries
* (required if not found in `LZ4_LIB_PATH`) `LZ4_INC_PATH` the location of lz4 header files (`lz4.h` etc)
* (required if not in environment) `BOOST_ROOT` the location of the boost libraries
* (required if not in environment) `TIFF_LIBRARY` the location of tiff library file
* (required if not found in `TIFF_LIBRARY`) `TIFF_INCLUDE_PATH` the path that contains the tiff header files (`tiff.h` etc)

### Building on Windows 7

This library must be built with MS Visual Compiler 14 (aka visual studio 2015) on Windows.

To obtain the dependencies on your system, we recommend the following installations:

* lz4 (download the source from [gitgub](https://github.com/Cyan4973/lz4) and build with cmake through the cmake_unofficial directory)
* libtiff (http://gnuwin32.sourceforge.net/packages/tiff.htm)
* boost (>= 1.55, either the official binaries from [boost binary website](http://sourceforge.net/projects/boost/files/boost-binaries/) or build it from source)
* hdf5 (>1.8, use the project [binaries for Windows](https://www.hdfgroup.org/HDF5/release/obtain5.html))
* ffmpeg (>= 2.5.8) with libx256 (>= 1.7) (take the relevant ffmpeg installation of binary and libraries from [ffmpeg.zeranoe.com](https://ffmpeg.zeranoe.com/blog/))
* cmake (>=3.1 from https://cmake.org/download/)

Here is how to build sqeazy on Windows from the command line (GUI based cmake invocation needs to be adapted along this): 
```
> cd X:\path\to\sqy\repo
> mkdir build
> cd build
> cmake.exe -DCMAKE_INSTALL_PREFIX=C:\Users\steinbac\temp_sqy -DHDF5_ROOT=C:\Users\steinbac\software\hdf5\1.8.1
6 -DBOOST_ROOT=C:\Users\steinbac\software\boost\1_59_0_static -DLZ4_ROOT=C:\Users\steinbac\software\lz4\r131\vc14 -DTIFF_INCLUDE_DIR=C:\Users\steinbac\software\
tiff\3.8.2-1\include -DTIFF_LIBRARY=C:\Users\steinbac\software\tiff\3.8.2-1\lib\libtiff.dll -DFFMPEG_ROOT_DIR=C:\Users\steinbac\software\ffmpeg\2.8.1 -DCMAKE_BU
ILD_TYPE=Release ..
> cmake.exe --build . --target ALL_BUILD --config Release
> ctest.exe -C Release #(optional) the above builds in Release mode
> cmake.exe --build . --target install --config Release
```

Sqeazy will build as much as possible using static libraries on Windows in order to reduce administrative overhead. 
If you encounter any problems or have questions, please use the bug tracker.


# Experimental Features

In order to try SIMD support of sqeazy encoders and decoders, build sqeazy with:

```
cmake -DSQY_EXPERIMENTAL=ON <other flags> ..
```

# License

There is no license yet, as there is no production ready code.
