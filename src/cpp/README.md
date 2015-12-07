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

## bench/sqy (application similar to zip/tar/...)

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

This library must be built with MinGW-w64 on Windows in order to obtain obtimal performance and remain flexible. Please install MinGW from [mingw-distro](http://nuwen.net/mingw.html) or using the [msys2](http://sourceforge.net/projects/msys2/) distribution (the latter is recommended)!

Here is a list of items to check before building on Win 7 :

1 open a mingw-w64 enabled shell (for msys2: use `mingw64_shell.bat`)
2 build lz4 with mingw-w64, using this lz4 Makefile
<https://github.com/psteinb/lz4/blob/master/lib/Makefile.mingw64>
3 install [github.com/Alexpux/MINGW-packages](https://github.com/Alexpux/MINGW-packages) and build libtiff using the `mingw-w64-libtiff` subdirectory, then install using `pacman`
4 use the boost installation coming with mingw-distro or build boost with in [github.com/Alexpux/MINGW-packages](https://github.com/Alexpux/MINGW-packages)

In order to configure cmake correctly, do the following *after* the above installation succeeded:

* go to the out-of-source build directory, e.g. `cd /path/to/sqeazy/; mkdir build`
* call the cmake GUI `cmake-gui` (command-line flags will follow shortly)
* `Configure` using Unix Makefiles and `gcc.exe/g++.exe/gfortran.exe` from `C:\path\to\mingw-w64` (don't worry this will fail, due to `make.exe` not working, fix that by pointing `CMAKE_MAKE_PROGRAM` to the correct file location of `make.exe`)
* set `LZ4_LIB_PATH` and `LZ4_INC_PATH` to your successfull build of `lz4`
* set either `BOOST_ROOT` to the path that contains `lib` with all boost libraries and `include` with all boost headers, or point `BOOST_LIBRARY_DIR` and `BOOST_INCLUDE_DIR` to the correct locations individually
* set `TIFF_INCLUDE_DIR` to the path containing `tiff.h` and `TIFF_LIBRARY` to the absolute path of `libtiff.a` or `libtiff.so`

Sqeazy will build as much as possible using static libraries on Windows in order to reduce administrative overhead. If you encounter any problems or have questions, please use the bug tracker.

# Experimental Features

In order to try SIMD support of sqeazy encoders and decoders, build sqeazy with:

```
cmake -DSQY_EXPERIMENTAL=ON <other flags> ..
```

# License

There is no license yet, as there is no production ready code.
