#ifndef _SQEAZY_H5_FILTER_HPP_
#define _SQEAZY_H5_FILTER_HPP_

#include "hdf5.h"

#include "sqeazy_definitions.hpp"
#include "sqeazy_pipelines.hpp"


namespace sqy = sqeazy;
  
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


  const char* c_input = reinterpret_cast<char*>(*_buf);


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
    
    /* extract the header from the payload */
    std::size_t c_input_size = *_buf_size;
    sqeazy::image_header hdr(c_input,  c_input + c_input_size);
    size_t found_num_bits = hdr.sizeof_header_type()*CHAR_BIT;
    std::vector<size_t> out_shape(hdr.shape()->begin(), hdr.shape()->end());
    
    if(hdr.empty()){
      ret = 100;
    }
    else{
      c_input_size = hdr.size() + hdr.compressed_size_byte();
      std::vector<size_t> in_shape = {c_input_size};
      
      /* setup output data */
      outbuflen = hdr.raw_size_byte();
      outbuf = new char[outbuflen];

      /* Start decompression. */
      if(found_num_bits == 16){
	if(!sqy::dypeline<std::uint16_t>::can_be_built_from(hdr.pipeline())){
	  std::cerr << "unable to build pipeline from " << hdr.pipeline() << "\n";
	}
	else{
	  auto pipe = sqy::dypeline<std::uint16_t>::from_string(hdr.pipeline());
      
	  ret = pipe.decode(c_input,
			    reinterpret_cast<std::uint16_t*>(outbuf),
			    in_shape,
			    out_shape
			    );
	}
      }

      if(found_num_bits == 8){
	if(!sqy::dypeline_from_uint8::can_be_built_from(hdr.pipeline())){
	  std::cerr << "unable to build pipeline from " << hdr.pipeline() << "\n";
	}
	else{
	auto pipe = sqy::dypeline_from_uint8::from_string(hdr.pipeline());
      
	ret = pipe.decode(c_input,
			  reinterpret_cast<std::uint8_t*>(outbuf),
			  in_shape,
			  out_shape);
	}
      }
    }

  } else {

    /** Compress data.
     **
     ** This is quite simple, since the size of compressed data in the worst
     ** case is known and it is not much bigger than the size of uncompressed
     ** data.  This allows us to use the simplified one-shot interface to
     ** compression.
     **/
    
    /* extract header from cd_values */
    const char* cd_values_bytes = reinterpret_cast<const char*>(_cd_values);
    unsigned long cd_values_bytes_size = _cd_nelmts*sizeof(unsigned);
    const char* cd_values_bytes_end = cd_values_bytes+cd_values_bytes_size;
    sqeazy::image_header cd_val_hdr(cd_values_bytes, cd_values_bytes_end);

    unsigned long c_input_shift = (2*cd_values_bytes_size > _nbytes) ? _nbytes : 2*cd_values_bytes_size;
    
    if(sqeazy::image_header::contained(c_input,  c_input + c_input_shift)){

      //data is already compressed
      sqeazy::image_header hdr(c_input,  c_input + c_input_shift);
      
      //headers mismatch
      if(hdr!=cd_val_hdr){
	ret = 1;
      }

      outbuflen = hdr.size() + hdr.compressed_size_byte();
      outbuf = new char[outbuflen];
      std::copy(c_input,c_input + outbuflen, outbuf);
      ret = 0;
    }
    else{

      //data must be compressed
      bool pipe_possible = sqy::dypeline<std::uint16_t>::can_be_built_from(cd_val_hdr.pipeline());
      size_t found_num_bits = cd_val_hdr.sizeof_header_type()*CHAR_BIT;
      
      if(!pipe_possible){
	ret = 1 ;
      }
      else{

	std::vector<size_t> shape(cd_val_hdr.shape()->begin(), cd_val_hdr.shape()->end());
	// unsigned header_size_byte = hdr.size();
	std::vector<char> payload;
	char* encoded_end = nullptr;

	if(found_num_bits == 16){
	  auto pipe = sqy::dypeline<std::uint16_t>::from_string(cd_val_hdr.pipeline());
	  outbuflen = pipe.max_encoded_size(_nbytes);
	  payload.resize(outbuflen);
	  
	  encoded_end = pipe.encode(reinterpret_cast<const std::uint16_t*>(c_input),
				    payload.data(),
				    shape);
	}

	
	if(found_num_bits == 8){
	  auto pipe = sqy::dypeline_from_uint8::from_string(cd_val_hdr.pipeline());
	  outbuflen = pipe.max_encoded_size(_nbytes);
	  payload.resize(outbuflen);
	  
	  encoded_end = pipe.encode(reinterpret_cast<const std::uint8_t*>(c_input),
				    payload.data(),
				    shape);
	}
	
	if(encoded_end!=nullptr){
	  outbuflen = encoded_end - payload.data();
	  outbuf = new char[outbuflen];
	  std::copy(payload.data(), payload.data()+outbuflen, outbuf);
	  ret = 0;//success
	} else {
	  ret = 1; // failure
	}
	  
	
      }
    }
  }

  /* Always replace the input buffer with the output buffer. */
  if(!ret)//success!
    {
    
      delete [] *(char**)_buf;//do we know the size of _buf?
    *_buf = outbuf;
    *_buf_size = outbuflen;
    value = outbuflen;

  }
  else{
    //failed 
    //by hdf5 convention, return 0
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
  "HDF5 sqy filter; see https://github.org/sqeazy/sqeazy",  /* Filter info */
  NULL, /* The "can apply" callback (TODO: what is that?) */
  NULL, /* The "set local" callback (TODO: what is that?) */
  (H5Z_func_t) H5Z_filter_sqy,  /* The filter function */
}};

// declare hdf5 plugin info functions
SQY_FUNCTION_PREFIX H5PL_type_t   H5PLget_plugin_type(){return H5PL_TYPE_FILTER;};
SQY_FUNCTION_PREFIX const void*   H5PLget_plugin_info(){return H5Z_SQY;};



#endif /* _SQEAZY_H5_FILTER_H_ */
