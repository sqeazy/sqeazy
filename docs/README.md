# Sqeazy #

**Sqeazy** - Fast and Flexible Volume Compression Library

A fast and flexible compression library for 2D/3D/ND volume data (currently only 3D is supported).
The core library is written in C++ with bindings for C, Java, and Python (to be implemented). Binaries are provided for Win (working for the native library), OSX (working for the native library, but not all tests pass), and Linux (works, native library and java bindings).

# Why yet another file format/codec?

Looking at the state of the art of 3D image compression in the life sciences, there are a number of packages that come to mind. 

- [Keller Lab Block Filetype (klb)](https://bitbucket.org/fernandoamat/keller-lab-block-filetype)
- [B3DB](https://git.embl.de/balazs/B3D)

Several points have to be observed:

- [klb](https://bitbucket.org/fernandoamat/keller-lab-block-filetype) 

    + yields very good compression ratio
    + last saw a commit June 2017
    + yields very slow compression in terms of speed
    + build system is buggy
    
- [B3DB](https://git.embl.de/balazs/B3D) 

    + does only build on windows at the moment ([linux port](https://github.com/openmicroscopy/B3D/pull/1) is work in progress)
    + no standard build system 
    + requires GPU (at least CUDA installed) according to the docs

## A little speed comparison

The main goal of sqeazy is to push the encoding speed high while retaining a maximum compression ratio. We want to compress the data on the scope as fast as possible.

| file                           | algorithm | original size [MByte] | compressed size [MByte] | time_to_compress [s] | compression_bandwidth [MB/s] | comment     |
| :---                           | :---      |                 :---: |                   :---: |                :---: |                        :---: | :---:       |
| just-a-bunch-of-zeroes (16bit) | klb       |                    64 |                   0.004 |                 12.9 |                          4.9 | 4 threads   |
| spim drosophila                | klb       |                   149 |                      31 |                 15.3 |                          9.7 | 4 threads   |
| just-a-bunch-of-zeroes (16bit) | sqy       |                    64 |                   0.256 |                  0.2 |                          320 | 4+1 threads |
| spim drosophila                | sqy       |                   149 |                      41 |                  0.7 |                          200 | 4+1 threads |
| just-a-bunch-of-zeroes (16bit) | lz4       |                    64 |                   0.256 |                 0.04 |                         1600 | 1 thread    |
| spim drosophila                | lz4       |                   149 |                      69 |                  0.4 |                          372 | 1 thread    |

Note, the above numbers correspond to one single measurement. We plan to update these numbers with more robust statistical results in the future. For the above table, the following versions were used:

- for klb: [master branch at 805a09d6f84](https://bitbucket.org/fernandoamat/keller-lab-block-filetype/commits/805a09d6f84a8e0c55c3f68dbf8e4aae8522e975)
- for sqy: [master branch at 4c45a9b5519](https://github.com/sqeazy/sqeazy/commit/4c45a9b55192dd34896c76cf75edbe6cc700d776)
- for lz4: 1.8.0

# What this Wiki can do for you

1. [Installation](Installation.md)
2. Sqeazy Features
    - [Overview](overview.md)
    - [available filters/encoders](available_steps.md)
    - how to use the [sqeazy API](ForDevelopers.md)
    - [Command Line Examples](CLIExamples.md)
4. [How to build a redistributable binary](distribution.md)
5. [FAQ](FAQ.md)
