# Building Sqeazy

Sqeazy comes in a whole bag of varieties. The native application and library is build using cmake. The grand project layout, compilation for java and packaging is performed with gradle.

To build everything from the root directory that contains this README.md file, just call
```gradle build```
or use the shipped script ```let_me_build_that_for_you.sh``` (click ```let_me_build_that_for_you``` on Windows).

If any of the dependencies listed below are not available through your environment, please call gradle like so on the command line:

```
gradle build -Phdf5_path=/usr/local/ -Plz4_path=/usr/local/ -Pboost_path=/usr/local/ -Pffmpeg_path=/usr/local/ -Ptiff_path=/usr/local
```

where the above assumes all required libraries are installed under /usr/local. Adapt this path according to your system.

## libsqeazy (native library and app)

```libsqeazy``` dependencies are listed below:

* lz4
* boost (> 1.55)
* hdf5 (>1.8)
* ffmpeg (>= 3.0) with libx256 (>= 1.7)

```sqy``` - the binary application to use similar to zip/tar/etc. depends on:

* lz4
* libtiff
* boost (>= 1.55)
* hdf5 (>1.8)
* ffmpeg (>= 3.0) with libx256 (>= 1.7)

For further information on building native sqeazy on the supported platforms, see the [FAQ](FAQ.md) or the [cmake documentation](src/cpp/README.md).


