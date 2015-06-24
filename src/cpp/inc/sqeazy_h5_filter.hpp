#ifndef _SQEAZY_H5_FILTER_H_
#define _SQEAZY_H5_FILTER_H_

#include "hdf5.h"
#include "H5PLextern.h"
#include "sqeazy_definitions.hpp"
#include "pipeline_select.hpp"



/* declare a hdf5 filter function */
SQY_FUNCTION_PREFIX size_t H5Z_filter_sqy(unsigned _flags,
					  size_t _cd_nelmts,
					  const unsigned _cd_values[],
					  size_t _nbytes,
					  size_t *_buf_size,
					  void** _buf){
  char *outbuf = NULL;
  size_t outbuflen = 0; //in byte
  int ret = 1;
  size_t value = 0;

  if (_flags & H5Z_FLAG_REVERSE) {

    /** Decompress data.
     **
     ** This process is troublesome since the size of uncompressed data
     ** is unknown, so the low-level interface must be used.
     ** Data is decompressed to the output buffer (which is sized
     ** for the average case); if it gets full, its size is doubled
     ** and decompression continues.  This avoids repeatedly trying to
     ** decompress the whole block, which could be really inefficient.
     **/


    /* Prepare the output buffer. */

    char* c_input = reinterpret_cast<char*>(*_buf);
    unsigned long long c_input_size = *_buf_size;

    /* extract the header */
    sqeazy::image_header hdr(c_input,  c_input + c_input_size);
    if(hdr.empty()){
      ret = 100;
    }
    else{
    /* setup pipeline */
    sqeazy::pipeline_select<> pipe_found(hdr.pipeline());

    /* setup output data */
    outbuflen = pipe_found.decoded_size_byte(c_input, _nbytes);
    outbuf = new char[outbuflen];

    /* Start decompression. */
    ret = pipe_found.decompress((const char*)c_input, outbuf, c_input_size);
    }

  } else {

    /** Compress data.
     **
     ** This is quite simple, since the size of compressed data in the worst
     ** case is known and it is not much bigger than the size of uncompressed
     ** data.  This allows us to use the simplified one-shot interface to
     ** compression.
     **/
    const char* cd_values_bytes = reinterpret_cast<const char*>(_cd_values);
    unsigned long cd_values_bytes_size = _cd_nelmts*sizeof(unsigned);
    const char* cd_values_bytes_end = cd_values_bytes+cd_values_bytes_size;
    const char* input = reinterpret_cast<char*>(*_buf);


    sqeazy::image_header hdr(input,  input + (2*cd_values_bytes_size));
    
    if(!hdr.empty()){

      outbuflen = *_buf_size;
      outbuf = new char[outbuflen];
      
      std::copy(input,input + outbuflen, outbuf);
      ret = 0;
    }
    else{
      
      sqeazy::image_header<void> header(cd_values_bytes, cd_values_bytes_end);//???
      std::string hdr = header.str();
      sqeazy::pipeline_select<> pipe_found(hdr);

      if(pipe_found.empty()){
	ret = 1 ;
      }
      else{

	unsigned header_size_byte = hdr.size();
	outbuflen = pipe_found.max_compressed_size(_nbytes,
						   header_size_byte
						   );
	std::vector<char> payload(outbuflen);
	unsigned long bytes_written = 0;
	

	std::vector<unsigned int> shape;
	header.shape(shape);
	ret = pipe_found.compress(input, &payload[0], shape ,bytes_written);

	if(!ret){
	  outbuflen = bytes_written;
	  outbuf = new char[outbuflen];
	  std::copy(payload.begin(), payload.begin() + bytes_written, outbuf);
	}
	  
      
      }
    }
  }

  /* Always replace the input buffer with the output buffer. */
  if(!ret)//success!
    {
    
    delete [] *_buf;
    *_buf = outbuf;
    *_buf_size = outbuflen;
    value = outbuflen;

  }
  else{//failed
    delete [] outbuf;
    value = 0;
  }
  
  return value;
  
}

/* use an integer greater than 256 to be id of the registered filter. */
//zip code of MPI CBG
static const H5Z_filter_t H5Z_FILTER_SQY = 01307;

static const H5Z_class2_t H5Z_SQY[1] = {{
  H5Z_CLASS_T_VERS, /* H5Z_class_t version */
  H5Z_FILTER_SQY, /* Filter id number */
  1, /* encoder_present flag (set to true) */
  1, /* decoder_present flag (set to true) */
  "HDF5 sqy filter; see https://bitbucket.org/sqeazy/sqeazy",  /* Filter info */
  NULL, /* The "can apply" callback (TODO: what is that?) */
  NULL, /* The "set local" callback (TODO: what is that?) */
  (H5Z_func_t) H5Z_filter_sqy,  /* The filter function */
}};

// declare hdf5 plugin info functions
SQY_FUNCTION_PREFIX H5PL_type_t   H5PLget_plugin_type(){return H5PL_TYPE_FILTER;};
SQY_FUNCTION_PREFIX const void*   H5PLget_plugin_info(){return H5Z_SQY;};



#endif /* _SQEAZY_H5_FILTER_H_ */
