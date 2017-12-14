# Command-Line Examples

## Location of the binary

If you use the gradle build inside `/path/to/sqy/repo`, the `sqy` binary will be located at `/path/to/sqy/repo/build/cpp/src/sqy` after a successfull build. All of the following recommendations and documentation assumes that the path of `sqy` is setup in your current shell sesion, so that

```
$ sqy
```
will produce the sqy help successfully.


## Encoding a tiff stack

To encode a tiff stack (currently only 16-bit and 8-bit encoded stacks are supported), simply do:

```
$ sqy enc data.tif
```

This assumes that data.tif contains one 3D stack where each frame is saved as it's own `TIFFDirectory`. This command uses the default sqeazy pipeline "bitswap1->lz4". Upon success, this will produce a file called `data.sqy` in the same directory where data.tif resides. Note, that this is a custom data format of sqeazy. If you'd like to produce the same into an hdf5 file, just use:

```
$ sqy enc -e ".h5" data.tif
```

which will produce data.h5. This produces `data.h5` which will contain a dataset called `sqy_stack`. To change the dataset name to something meaningful like `helloworld`, do

```
$ sqy enc -e ".h5" -d "helloworld" data.tif
```

Inspect the produced hdf5 file with (assuming that hdf5 tools is in your PATH):

```
$ h5ls data.h5 
helloworld               Dataset {65, 42, 47}
```

## Decoding a compressed sqeazy file

Decoding of both the native format as well as the hdf5 format is supported. To decode a stack, just do

```
$ sqy dec data.sqy
```

or

```
$ sqy dec data.h5
```

this will produce/overwrite data.tif. If you'd like to unpack to anything else than `data.tif`, use the `-o` flag:

```
$ sqy dec -o helloworld.tif data.h5
```

In case a hdf5 file contains multiple datasets, you can select the data set to decode manually:

```
$ sqy dec -d "helloworld" -o helloworld.tif data.h5
```

