
# Frequently Asked Questions

## Oh my, All these dependencies ... how do I obtain them for my OS?

### Linux

The default package managers of the OS are typically enough. Except for ffmpeg, all of the dependencies' version come with the package manager. If that is not the case, please consider reporting this. For ffmpeg, please see the instructions on [http://ffmpeg.org/](ffmpeg.org) how to obtain the most recent stable version.

### OSX

We recommend using homebrew. It currently provides all dependencies of sqeazy.

### Windows

We currently have a test system with Windows 7 Professional 64bit and MS Visual Studio 14 installed. All advice is based on this system. If you have something else, feel free to report your experiences.

We recommend the following installations paths for sqeazy's dependencies on Windows:

* lz4 (download the source from [gitgub](https://github.com/Cyan4973/lz4) and build with cmake through the cmake_unofficial directory)
* libtiff (http://gnuwin32.sourceforge.net/packages/tiff.htm)
* boost (>= 1.55, either the official binaries from [boost binary website](http://sourceforge.net/projects/boost/files/boost-binaries/) or build it from source)
* hdf5 (>1.8, use the project [binaries for Windows](https://www.hdfgroup.org/HDF5/release/obtain5.html))
* ffmpeg (>= 3.0) with libx256 (>= 1.7) (take the relevant ffmpeg installation of binary and libraries from [ffmpeg.zeranoe.com](https://ffmpeg.zeranoe.com/blog/))
* cmake (>=3.1 from https://cmake.org/download/)



