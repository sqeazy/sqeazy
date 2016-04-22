# Something is not right with HEVC/H265 support in libsqeazy?

Be sure to have ffmpeg and all it's libraries installed with OSX support. To check that, use the following command:
```
$ ffmpeg -h encoder=hevc
...
Encoder libx265 [libx265 H.265 / HEVC]:
    General capabilities: delay threads 
    Threading capabilities: auto
    Supported pixel formats: yuv420p yuv422p yuv444p
libx265 AVOptions:
    -crf               <float>      E..V.... set the x265 crf (from -1 to FLT_MAX) (default -1)
...
```

On OSX, you do something like:

```
$ brew install ffmpeg --with-x265 --with-x264 --with-vpx --with-tools
```
