# Sqeazy #

**Sqeazy** - Fast and Flexible Volume Compression Library

A fast and flexible compression library for 2D/3D/ND volume data.
The core library is written in C++ with bindings for C, Java (to be implemented), and Python Java (to be implemented). Binaries are provided for Win (not implemented yet), OSX (works, but not all tests pass), and Linux (works).

# Dependencies

To build sqeazy, the cmake build system (any version higher than 2.8) is required as well as a standard compiler and linker (gcc, llvm/clang were tested).

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

Sqeazy is currently undergoing constant development. If you want to test-drive it, use the `sqy` application in `bench/`. Beware of using it for production data, we guarantee nothing in this case!

## Build instructions

In order to build sqeazy, use the following:

```bash
$ cd sqeazy
$ mkdir build
$ cd build
$ cmake ..
$ make 
```

The are some cmake build flags that are supported:
to be filled!

# License

There is not license yet, as there is not production ready code.
