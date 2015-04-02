# Sqeazy #

**Sqeazy** - Fast and Flexible Volume Compression Library

A fast and flexible compression library for 2D/3D/ND volume data.
The core library is written in C++ with bindings for C, Java (to be implemented), and Python Java (to be implemented). Binaries are provided for Win (not implemented yet), OSX (works, but not all tests pass), and Linux (works).

# Dependencies

To build sqeazy, the cmake build system (any version higher than 2.8) is required as well as a gcc or clang compiler and affiliated linkers.

## libsqy

* lz4

## bench/sqy (application similar to zip/tar/...)

* lz4
* libtiff
* boost (> 1.42)

## bench/hist (simple app to produce histograms of input data)

* lz4
* libtiff
* boost (> 1.42)
* root

# General Remarks

Sqeazy is currently undergoing constant development. If you want to test-drive it, use the `sqy` application in `bench/`. Beware of using it for production data, we guarantee nothing in this case! We intend sqeazy to be build out-of-source.

## Build instructions

In order to build sqeazy, use the following (tested on Linux and OSX):

```bash
$ cd sqeazy
$ mkdir build
$ cd build
$ cmake ..
$ make 
```

The are some cmake build flags that are supported:
* (required if not in environment) `LZ4_LIB_PATH` the location of the lz4 static and dynamic libraries
* (required if not found in `LZ4_LIB_PATH`) `LZ4_INC_PATH` the location of lz4 header files (`lz4.h` etc)
* (required if not in environment) `BOOST_ROOT` the location of the boost libraries
* (required if not in environment) `TIFF_LIBRARY` the location of tiff libraries
* (required if not found in `TIFF_LIBRARY`) `TIFF_INCLUDE_PATH` the location of tiff header files (`tiff.h` etc)

### Building on Windows 7

This library must be built with MinGW-w64 on Windows in order to obtain obtimal performance and remain flexible. Please install MinGW from <http://nuwen.net/mingw.html>!

Here is a list of items to check before building on Win 7 :

* build lz4 with mingw-w64, using this lz4 Makefile
https://github.com/psteinb/lz4/blob/master/lib/Makefile.mingw64

* build libtiff as usual on windows

* use the boost installation coming with mingw-distro

* mingw-w64 has been observed to perform poorly on detecting the correct CPU architecture, hence please add
```
CMAKE_CXX_FLAGS = -msse4
```
to your cmake install.

Sqeazy will build as much as possible using static libraries on Windows in order to reduce administrative overhead. If you encounter any problems or have questions, please use the bug tracker.

# Experimental Features

In order to try SIMD support of sqeazy encoders and decoders, build sqeazy with:

```
cmake -DSQY_EXPERIMENTAL=ON <other flags> ..
```

# License

There is not license yet, as there is not production ready code.
