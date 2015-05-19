# Sqeazy #

**Sqeazy** - Fast and Flexible Volume Compression Library

A fast and flexible compression library for 2D/3D/ND volume data.
The core library is written in C++ with bindings for C, Java (to be implemented), and Python (to be implemented). Binaries are provided for Win (working for the native library), OSX (working for the native library, but not all tests pass), and Linux (works, native library and java bindings).

# Building it

As mentioned above, sqeazy comes in a whole bag of varieties. The native application and library is build using cmake. The grand project layout, compilation for java and packaging is performed with gradle.

To build everything from the root directory that contains this README.md file, just call
```gradle build```
or use the shipped script ```let_me_build_that_for_you.sh``` (click ```let_me_build_that_for_you``` on Windows).

## libsqeazy (native library and app)

```libsqeazy``` dependencies are listed below:

* lz4

```sqy``` - the binary application to use similar to zip/tar/etc. depends on:

* lz4
* libtiff
* boost (> 1.42)

For further information on building native sqeazy on the supported platforms, see [src/cpp/README.md].

## java bindings

The java bindings just require libsqeazy to be present in the directory ```/path/to/repo/src/cpp/build/src```.

# License

There is no license yet, as there is no production ready code.
