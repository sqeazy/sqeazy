
# For Developers

Sqeazy is an app and a library at the same time. This Wiki for developers is intended for those that would like to interface their application to libsqeazy either natively (C/C++) or through Java. After successfull compilation, sqeazy exposes a binary `sqy` that allows `tar` like compression or decompression of 3D images (currently only .tiff files are supported). Also, a dynamically linked library and the according header file is provided.

**Disclaimer: Until further notice, the sqeazy API is under development!**

## For C Developers

In order to use sqeazy, use `sqeazy.h` and (depending on your OS) `libsqeazy.{so,dll,dyld}`. As the API will vary until we reach 1.0, please consult the aforementioned header file for more details.


## For C++ Developers

So far, there is no public API for sqeazy available. But as sqeazy internally is written using C++11, a header-only exposure for C++ is planned.


## For Java Developers

There is a BridJ wrapper around `libsqeazy.{so,dll,dyld}` available. It's use is documented in `./src/java/src/sqeazy/bindings/test/SqeazyLibraryTests.java` and is based solely on the C interface.


## For Python Developers

If you'd like to see python bindings for sqeazy, please submit an issue to show support for this idea. We are planning to add a python interface for (at least) python3 using [pybind11](http://pybind11.readthedocs.io/en/stable/) in the future.
