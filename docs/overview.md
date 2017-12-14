# Sqeazy Overview

## Rationale

Sqeazy is meant as a compression library that offers flexibility, encoding speed and portability in a cross platform way. The main motivation of sqeazy is to bring lossless or lossy compression as close as possible to the data source. To make things more precise we aim at a compression ingest bandwidth of around 1 GB/s in a multi-thread context of around 8 threads.

## Pipelines

Sqeazy offers to specify pipelines of filter steps before the actual encoding is performed. These steps are performed in order and are potentially multi-threaded. The global limit on the number of threads is honored by every filter step. 

A pipeline is specified by a string that must follow fixed semantics. This string can then be handed to the command line app or to the sqeazy API. It must comply to the form: `filter1->sink`. Sqeazy differentiates between filters and sinks. A __sink__ refers to a processing step that transforms the stack in a way which changes the data type of the stack and may also change the memory footprint of it so that the input stack cannot be represented by a euclidean cube anymore. A __filter__ alters the content of a stack, but leaves the data type and the geometric interpretation of the chunk of memory unchanged.

Here are some examples:

- `bitswap1->lz` : perform a bitplane reordering and then hand the output to a lz4 compressor
- `bitswap1->lz(accel=8)` : perform a bitplane reordering and then hand the output to a lz4 compressor cranking up the lz4 compression to level 8 (see the [lz4](www.lz4.org) docs for details). Note, this feature is curently broken [#59](https://github.com/sqeazy/sqeazy/issues/59)).
- `quantiser(decode_lut_path=/tmp/test.lut)->x264` : apply our quantisation scheme to 16-bit data, dump the decoding look-up-table (lut) to `/tmp/test.lut` and encode the resulting stack with `x264`


# File Format(s)

The raw sqeazy file format consists of 2 parts:
- a json header like this one
```
$ head -n19 /dev/shm/test_256x1024x128_16bit.sqy  
{
    "pipename": "bitswap1(num_bits_per_plane=1)->lz4",
    "raw": {
        "type": "t",
        "rank": "3",
        "shape": {
            "dim": "128",
            "dim": "1024",
            "dim": "256"
        }
    },
    "encoded": {
        "bytes": "263182"
    },
    "sqy": {
        "version": "0.5.2",
        "headref": "fb193e3"
    }
}
```
- a magic token
- the compressed payload of the stack

## HDF5 support

The `sqy` command line app supports encoding the input stacks (e.g. from a tiff file) into hdf5 files. To do so, issue:

```
$ ./sqy encode -e .h5 /path/to/some.tif
$ ls /path/to/some.h5
/path/to/some.h5
```

We also have a HDF5 filter available that is installed with the library. When using that filter you have to tweak the environment of any HDF5 tooling first. Not doing so, will cause an error:

```
$ h5ls -d /scratch/steinbac/sqeasy_data/xwing/some.h5
sqy_stack                Dataset {108, 1352, 532}
    Data:
        (0,0,0)         Unable to print data.
H5tools-DIAG: Error detected in HDF5:tools (1.8.18) thread 0:
  #000: ../../../tools/lib/h5tools_dump.c line 1621 in h5tools_dump_simple_dset(): H5Dread failed
    major: Failure in tools library
    minor: error in function
```

But if you point `HDF5_PLUGIN_PATH` to the location of `libsqeazy.so`, it works:

```
$ export HDF5_PLUGIN_PATH=/path/to/libsqeazy.so/
$ h5ls -d /scratch/steinbac/sqeasy_data/xwing/some.h5 | head -n30
sqy_stack                Dataset {108, 1352, 532}
    Data:
        (0,0,0) 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        (0,0,89) 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        (0,0,177) 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        (0,0,265) 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        (0,0,353) 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        (0,0,441) 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        (0,0,529) 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        (0,1,85) 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        (0,1,173)
```
