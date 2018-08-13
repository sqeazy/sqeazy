# Binary Distribution

This document tries to summarize, what steps were taken to build and bundle the sqeazy shared library that is distributed on bintray.

## Build preparations

### macOS

Before you do anything, export the following in your terminal to achieve a cross-OSX/macOS-platform binary:

```
$ export MACOSX_DEPLOYMENT_TARGET=10.9
```

At the time of writing, Xcode didn't provide a usable OpenMP implementation. Thus, we had to build llvm manually (the version shipped with homebrew didn't contain position independent code).

Follow the steps indicated [here](https://llvm.org/docs/GettingStarted.html#getting-started-quickly-a-summary), make sure you include the openmp library from llvm, unpack all source code in the relevant subdirectories of the llvm source tree. Let the llvm source tree be located in `src-llvm`, create a build folder next to it and `cd` into it. Then:

```
$ cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -DLLVM_ENABLE_PIC=ON -DCMAKE_BUILD_TYPE=Release -DLIBOMP_ENABLE_SHARED=OFF ../src-llvm/
$ make 
$ make install
```


### X264 

Obtain the source from [here](http://www.videolan.org/developers/x264.html) or directly clone from the official repo:

```
$ git clone http://git.videolan.org/git/x264.git
$ cd x264
$ ./configure --disable-opencl --enable-pic --enable-static --disable-cli --prefix=$PREFIX
```

* Windows :
    - clone static library in <prefix>/lib to libx265.lib if it does not exist already (required for ffmpeg pickup)
	- my cmake port can also be used for building [psteinb/x264](https://github.com/psteinb/x264)

### X265

Obtain the source from [here](https://bitbucket.org/multicoreware/x265/downloads/) or directly clone from the official repo:

```
hg clone https://bitbucket.org/multicoreware/x265
cd x265
cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_SHARED=OFF -DENABLE_PIC=ON -DENABLE_CLI=OFF -DSTATIC_LINK_CRT=ON -DCMAKE_INSTALL_PREFIX=$PREFIX ..
```

* Windows :
    - clone static library in <prefix>/lib to libx265.lib and x2654.lib if either does not exist already (required for ffmpeg pickup)
	- don't forget  the right generator for 32-bit versus 64-bit
	- use `-DSTATIC_LINK_CRT=ON` as well as the above
	
### FFMPEG

Obtain the source from [here](http://ffmpeg.org/download.html) or directly clone from the official repo:

```
git clone git://source.ffmpeg.org/ffmpeg.git
cd ffmpeg
./configure --pkg-config-flags=--static --extra-ldflags='$X264_PREFIX/lib/libx264.a $X265_PREFIX/lib/libx265.a' --disable-doc --enable-small --disable-shared --disable-encoders --disable-decoders --enable-encoder=libx264,libx264rgb,libx265 --enable-decoder=h264,h264_vda,hevc --enable-pic --enable-static --enable-libx264 --enable-libx265 --enable-gpl  --disable-sdl --disable-libxcb --disable-zlib --disable-bzlib --disable-xlib --disable-lzma --disable-indevs --disable-outdevs  --disable-protocols --disable-muxers --disable-demuxers --enable-muxer=h264,hevc --enable-demuxer=h264,hevc --disable-filters --disable-parsers --enable-parser=x264,hevc --prefix=$PREFIX
```

* the above assumes that all external dependencies (libx264 and libx265 are available as static libraries), so we add `--pkg-config-flags="--static"`

* Windows (64bit, Visual Studio 14 2015):
	- open visual studio x64 command prompt
	- load environment variables
	```
	vcvarsall amd64
	```
	- export path to libx254.lib and libx265.lib to LIB
	```
	set LIB=C:\Users\me\software\x265\2.0\lib;%LIB%
	set LIB=C:\Users\me\software\x264\master\lib;%LIB%
	```
	- export path to x254.h and x264.h to INCLUDE
	```
	set INCLUDE=C:\Users\me\software\x265\2.0\include;%INCLUDE%
	set INCLUDE=C:\Users\me\software\x264\master\include;%INCLUDE%
	```
	- start msys2 prompt
	```
	C:\msys64\msys2_shell.bat
	```
	- go to ffmpeg source directory
	- call configure
	```
	 ./configure --toolchain=msvc --disable-shared --enable-libx264 --enable-libx265 --enable-pic --enable-gpl --prefix=/c/Users/me/software/ffmpeg/3.0.2-static --disable-sdl --disable-libxcb --disable-zlib --disable-bzlib --disable-xlib --disable-lzma --disable-indevs --disable-outdevs
	 make
	 make install
	```
	
### HDF5

- obtain the source from [here](https://support.hdfgroup.org/downloads/)

- if using the `configure` based build
```
./configure --prefix=$PREFIX --enable-cxx --enable-hl --with-pic --disable-shared 
make
make install
```

- if using the cmake driven build
    - command
```
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DPOSITION_INDEPDENT_CODE=ON -DHDF5_BUILD_CPP_LIB=ON -DHDF5_BUILD_HL_LIB=ON -DCMAKE_INSTALL_PREFIX=<prefix> ..
```
    - version 1.10.0 ships a FindHDF5.cmake that has a bug, this needs to be fixed by hand 
	```
	FIND_PATH (HDF5_ROOT_DIR "hdf5-config.cmake"
    HINTS ${_HDF5_HINTS}
    PATHS ${_HDF5_PATHS}
    PATH_SUFFIXES
        cmake/hdf5
        lib/cmake/hdf5
        share/cmake/hdf5
		)
	```
	to something like
	```
	FIND_PATH (HDF5_ROOT_DIR "hdf5-config.cmake"
    HINTS ${_HDF5_HINTS}
    PATHS ${_HDF5_PATHS}
    PATH_SUFFIXES
        cmake/hdf5
        lib/cmake/hdf5
        share/cmake/hdf5
		share/cmake
	)
	```
	- on Windows, the top level CMakeLists.txt needs the following lines at around L390:
    ```
	if(NOT ${BUILD_SHARED_LIBS})
		if(DEFINED MSVC)
			set(CompilerFlags CMAKE_CXX_FLAGS_RELEASE CMAKE_C_FLAGS_RELEASE)
			foreach(CompilerFlag ${CompilerFlags})
				string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
			endforeach()
		endif()
	endif()
    ```

### LZ4

Obtain the source from [here](https://github.com/lz4/lz4). Then go to the `contrib/cmake_unofficial/` subfolder and issue:

```
mkdir build
cd build 
cmake -DBUILD_SHARED_LIBS=OFF -DLZ4_POSITION_INDEPENDENT_LIB=ON ..
```

## Build 

The following assumes that you installed all dependencies as described above

### using gradle

-  does the setup, build and upload of bundled sqeazy to bintray for the current operating system

```
gradle upload_os_bundle -Pboost_path=<boost_static_path>  -Ptiff_path=<tiff_static_path> -Pffmpeg_path=<ffmpeg_static_path> -Plz4_path=<lz4_static_path> -Phdf5_path=<hdf5_static_path> -Pcmake_app_path=<path_to_cmake_if_nonstandard>
```

-  after you did the above on all supported operating systems, the following collects/downloads the binaries of all supported platforms of libsqeazy and builds a jar from them

```
gradle merge_local_bundles -Pboost_path=<boost_static_path>  -Ptiff_path=<tiff_static_path> -Pffmpeg_path=<ffmpeg_static_path> -Plz4_path=<lz4_static_path> -Phdf5_path=<hdf5_static_path> -Pcmake_app_path=<path_to_cmake_if_nonstandard>
```
- uploads fat jar to bintray

```
gradle bintrayUpload
```

## Tested on

### Linux 

* Fedora 27
* CentOS 7.1
* Ubuntu 14.04, Ubuntu 16.04

### Windows

* Windows 7 Pro 64bit with Visual Studio 15 2017 

### macOS

* 10.12.5, 10.11.*


# Building and Uploading

The documentation below assumes that you have the rights to access the sqeazy bintray account and that `bintray_user` and `bintray_key` are defined by some global mechanism (e.g. `/Users/me/.gradle/gradle.properties`).

## Windows 7

* make sure that the following packages are available through PATH, LIB and INCLUDE:

- boost\1_59_0_static
- tiff\4.0.6
- hdf5\1.8.17-static
- ffmpeg\3.0.2-static 
- x265\2.0-static-crt
- x264\master-static
- gradle
- curl

* invoke the batch script that contains the locations of all the dependencies on Windows on my build machine (adapt at will).

  ```
  C:\path\to\sqeazy > call share/windows/7/setup-deployment.bat
  ```
  
  After this script is through, close the shell and reopen it again (no joke).

* call gradle to build the bundle and upload it

  ```
  C:\path\to\sqeazy > gradle upload_bundle -Pboost_path=C:\Users\steinbac\software\boost\1_59_0_static -Phdf5_path=C:\Users\steinbac\software\hdf5\1.8.17-static
  ```

## macOS

I assume, that you have all the dependencies installed. In my case, I found ffmpeg to be critical on macOS. Homebrew allowed me to install boost and hdf5 in a static fashion, so there is no need to point gradle/cmake to it. Only ffmpeg required special attention. Call:

```
$ gradle -Pcxx_compiler=`which clang++` -Pffmpeg_path=/Users/steinbac/software/ffmpeg/3.0.2-x264-hevc-static-minimal/ upload_bundle
```

## Linux

I advice you to use the Dockerfiles in [share/docker/trusty](share/docker/trusty) and build the image by doing:

```
$ cd /path/to/sqeazy
$ cd share/docker/trusty
$ make build-static
$ docker run -e ORG_GRADLE_PROJECT_bintray_user=${MY_BINTRAY_USER} -e ORG_GRADLE_PROJECT_bintray_key=${MY_BINTRAY_KEY} -it --rm -v /path/to/sqeazy/:/sqeazy sqy/trusty/static:latest
$ cd /sqeazy #now inside the container
$ gradle upload_bundle
```
