# Sqeazy #

**Sqeazy** - Fast and Flexible Volume Compression Library

A fast and flexible compression library for 2D/3D/ND volume data (currently only 3D is supported).
The core library is written in C++ with bindings for C, Java (to be implemented), and Python (to be implemented). Binaries are provided for Win (working for the native library), OSX (working for the native library, but not all tests pass), and Linux (works, native library and java bindings).

# Why yet another file format?

Looking at the state of the art of 3D image compression in the life sciences, there are a number of packages that come to mind. 

- [Keller Lab Block Filetype (klb)](https://bitbucket.org/fernandoamat/keller-lab-block-filetype)
- [B3DB](https://git.embl.de/balazs/B3D)
- [openjxr](https://github.com/glencoesoftware/jxrlib)

Several points have to be observed:

- [klb](https://bitbucket.org/fernandoamat/keller-lab-block-filetype) last saw a commit June 2017
- [B3DB](https://git.embl.de/balazs/B3D) does only build on windows at the moment

## A little speed comparison

The main goal of sqeazy is to push the encoding speed high while retaining a maximum compression ratio. We want to compress the data on the scope as fast as possible.

| file                           | algorithm    | original size [MByte] | compressed size [MByte] | time_to_compress [s] | compression_bandwidth [MB/s] | comment   |
| just-a-bunch-of-zeroes (16bit) | klb (master) |                    64 |                   0.004 |                 12.9 |                          4.9 | 4 threads |
| xwing drosophila               | klb (master) |                   149 |                      31 |                 15.3 |                          9.7 | 4 threads |
| just-a-bunch-of-zeroes (16bit) | sqy (master) |                    64 |                         |                      |                              | 4 threads |
| xwing drosophila               | sqy (master) |                   149 |                         |                      |                              | 4 threads |


# What this Wiki can do for you

1. [Installation](Installation)
1. [For Developers](ForDevelopers)
1. [Command Line Examples](CLIExamples)
