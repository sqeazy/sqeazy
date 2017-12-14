# Available Encoders or Sinks

A pipeline step we call a __sink__, generally refers to a processing step that transforms the stack in a way which changes the data type of the stack may also change the memory footprint of it so that the input stack cannot be represented by a euclidean cube anymore.

```
  pass_through	pass/copy content to next stage
     quantiser	scalar histogram-based quantisation for conversion uint16->uint8; <decode_lut_path>
              	: the lut will be taken from there (for decoding) or written there (for
              	encoding); <decode_lut_string> : decode LUT will be taken from the argument
              	value (for decoding) or written there (for encoding); <weighting_function>=(none,
              	power_of_<enumerator>_<denominator> : apply power-law pow(x,<enumerator>/<denominator>)
              	to histogram weights, offset_power_of_<enumerator>_<denominator> : apply
              	power-law pow(x,<enumerator>/<denominator>) to histogram weights starting
              	at first non-zero bin index) 
          hevc	hevc encode gray8 buffer with hevc, args can be anything that libavcodec
              	can understand, see ffmpeg -h encoder=hevc
          h264	h264 encode gray8 buffer with h264, args can be anything that libavcodec
              	can understand, e.g. h264(preset=ultrafast,qp=0) would match the ffmpeg
              	flags --preset=ultrafast --qp=0; see ffmpeg -h encoder=h264 for details.;if
              	the input data is of 16-bit type, it is quantized by a min_of_4 difference
              	scheme
           lz4	compress input with lz4, <accel|default = 1> improves compression speed
              	at the price of compression ratio
```


# Available Filters

__Filters__ do alter the content of a stack, but leave the data type and the geometric interpretation of the chunk of memory unchanged.

## Before A Sink

```
             diff3x3x1	store difference to mean of neighboring items
              bitswap1	rewrite bitplanes of item as chunks of buffer, use <num_bits_per_plane|default
                      	= 1> to control how many bits each plane has
     remove_background	set all items to 0 that are below <threshold>, reduce any other item by
                      	threshold
  rmbkrd_neighbor5x5x5	shot noise removal scheme, if <faction|default = 50%> of pixels in neighborhood
                      	(default: 5x5x5) fall below <threshold|default = 1>, set pixel to 0
             rmestbkrd	estimate noise from darkest planes, remove median of that plane from all
                      	pixels/voxels
        raster_reorder	reorder the memory layout of the incoming buffer by linearizing virtual
                      	tiles, control the tile size by tile_size=<integer|default: 8>
          tile_shuffle	reorder the tiles in the incoming stack based on some defined metric; use
                      	tile_size=<integer|default: 32> to configure the shape of the tile to 
                      	xtract
         frame_shuffle	reorder the frames in the incoming stack based on some defined metric
        zcurve_reorder	reorder the memory layout of the incoming buffer using space filling z
                      	curves
```

## After Sink

```
       diff3x3x1	store difference to mean of neighboring items
        bitswap1	rewrite bitplanes of item as chunks of buffer, use <num_bits_per_plane|default
                	= 1> to control how many bits each plane has
            h264	h264 encode gray8 buffer with h264, args can be anything that libavcodec
                	can understand, e.g. h264(preset=ultrafast,qp=0) would match the ffmpeg
                	flags --preset=ultrafast --qp=0; see ffmpeg -h encoder=h264 for details.;if
                	the input data is of 16-bit type, it is quantized by a min_of_4 difference
                	scheme
            hevc	hevc encode gray8 buffer with hevc, args can be anything that libavcodec
                	can understand, see ffmpeg -h encoder=hevc
             lz4	compress input with lz4, <accel|default = 1> improves compression speed
                	at the price of compression ratio
  raster_reorder	reorder the memory layout of the incoming buffer by linearizing virtual
                	tiles, control the tile size by tile_size=<integer|default: 16>
    tile_shuffle	reorder the tiles in the incoming stack based on some defined metric; use
                	tile_size=<integer|default: 32> to configure the shape of the tile to 
                	xtract
   frame_shuffle	reorder the frames in the incoming stack based on some defined metric
  zcurve_reorder	reorder the memory layout of the incoming buffer using space filling z
                	curves
```

## Disclaimer

This page mostly consists of dump of the command line app. This page is subject to change in the future!
