[![Build Status](https://travis-ci.org/sqeazy/sqeazy.svg?branch=master)](https://travis-ci.org/sqeazy/sqeazy)

# Sqeazy #

**Sqeazy** - Fast and Flexible Volume Compression Library

A fast and flexible compression library for 3D volume data (ND potentially planned).
The core library is written in C++ with bindings for C, Java (through BridJ), and Python (to be implemented). Binaries are provided for Win (working for the native library), OSX (working for the native library, but not all tests pass), and Linux (works, native library and java bindings).

## Using Sqeazy Java interface from binary distribution (experimental)

Sqeazy is hosted on bintray. In order to use it, do:

### If gradle is your build system

Add the following to your `build.gradle`:

```
repositories 
{
	//...
	maven { url  "https://dl.bintray.com/sqeazy/maven"  }
}

dependencies {
	//...
    compile 'net.sqeazy:sqeazy:0.5.2'
}
```

### If maven is your build system

```
<repositories>
	...
	<repository>
		<id>sqeazy</id>
		<url>https://dl.bintray.com/sqeazy/maven</url>
	</repository>
</repositories>

<dependencies>
	...
	<dependency>
		<groupId>net.sqeazy</groupId>
		<artifactId>sqeazy</artifactId>
		<version>0.1</version>
		<type>pom</type>
	</dependency>
</dependencies>
```


### To use sqeazy in Java do

```
import sqeazy.bindings.SqeazyLibrary;
import org.bridj.Pointer;

public class Library {

    public boolean isSqeazyAvailable() {

	final Pointer<Integer> version = Pointer.allocateInts(3);
	SqeazyLibrary.SQY_Version_Triple(version);

	int sum = version.get(0) + version.get(1) + version.get(2);
	if(sum>0)
	    return true;
	else
	    return false;
    }

}
```

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
* boost (> 1.56)
* hdf5 (>= 1.8.12)
* ffmpeg (>= 3.0) with libx256 (>= 1.7)

```sqy``` - the binary application to use similar to zip/tar/etc. depends on:

* lz4
* libtiff
* boost (>= 1.56)
* hdf5 (>= 1.8.12)
* ffmpeg (>= 3.0) with libx256 (>= 1.7)

For further information on building native sqeazy on the supported platforms, see the [FAQ](FAQ.md) or the [cmake documentation](src/cpp/README.md).

## java bindings

The java bindings just require libsqeazy to be present in the directory ```/path/to/repo/src/cpp/build/src```.

# License

There is no license yet, as there is no production ready code.
