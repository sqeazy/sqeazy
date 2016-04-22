# Sqeazy #

**Sqeazy** - Fast and Flexible Volume Compression Library

A fast and flexible compression library for 2D/3D/ND volume data (currently only 3D is supported).
The core library is written in C++ with bindings for C, Java (to be implemented), and Python (to be implemented). Binaries are provided for Win (working for the native library), OSX (working for the native library, but not all tests pass), and Linux (works, native library and java bindings).

# Building it

As mentioned above, sqeazy comes in a whole bag of varieties. The native application and library is build using cmake. The grand project layout, compilation for java and packaging is performed with gradle.

To build everything from the root directory that contains this README.md file, just call
```gradle build```
or use the shipped script ```let_me_build_that_for_you.sh``` (click ```let_me_build_that_for_you``` on Windows).

If any of the dependencies listed below are not available through your environment, please call gradle like:

```
gradle build -Phdf5_path=/usr/local/ -Plz4_path=/usr/local/ -Pboost_path=/usr/local/ -Pffmpeg_path=/usr/local/ -Ptiff_path=/usr/local
```

where in the above all required libraries by sqeazy are installed under /opt/local. Adapt this path according to your system.

## libsqeazy (native library and app)

```libsqeazy``` dependencies are listed below:

* lz4
* boost (> 1.55)
* hdf5 (>1.8)
* ffmpeg (>= 2.5.8) with libx256 (>= 1.7)

```sqy``` - the binary application to use similar to zip/tar/etc. depends on:

* lz4
* libtiff
* boost (>= 1.55)
* hdf5 (>1.8)
* ffmpeg (>= 2.5.8) with libx256 (>= 1.7)

For further information on building native sqeazy on the supported platforms, see [src/cpp/README.md].

## java bindings

The java bindings just require libsqeazy to be present in the directory ```/path/to/repo/src/cpp/build/src```.

# License

There is no license yet, as there is no production ready code.
