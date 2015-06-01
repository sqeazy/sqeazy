#ifndef _SQEAZY_HDF5_IMPL_H_
#define _SQEAZY_HDF5_IMPL_H_

#include "H5Cpp.h"

#include "sqeazy.h"
#include "sqeazy_h5_filter.hpp"
#include "sqeazy_impl.hpp"
#include "sqeazy_header.hpp"
#include "sqeazy_predef_pipelines.hpp"




size_t H5Z_filter_sqy(unsigned int _flags, //is it encode or decode
		      size_t _cd_nelmts,
		      const unsigned int _cd_values[], 
		      size_t _nbytes,
		      size_t *_buf_size, 
		      void **_buf
		      )
{
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

    outbuflen = sqeazy::bswap1_lz4_pipe::decoded_size_byte(c_input, _nbytes);

    outbuf = new char[outbuflen];

    /* Start decompression. */

    unsigned short* output = reinterpret_cast<unsigned short*>(outbuf);

    ret = sqeazy::bswap1_lz4_pipe::decompress(c_input, output, _nbytes);

    /* End decompression. */

  } else {

    /** Compress data.
     **
     ** This is quite simple, since the size of compressed data in the worst
     ** case is known and it is not much bigger than the size of uncompressed
     ** data.  This allows us to use the simplified one-shot interface to
     ** compression.
     **/
    unsigned long input_nelem = _nbytes/sizeof(unsigned short);
    unsigned header_size_byte = sqeazy::bswap1_lz4_pipe::header_size(input_nelem);
    outbuflen = sqeazy::bswap1_lz4_pipe::max_bytes_encoded(_nbytes,
							   header_size_byte
							   ); 
    
    outbuf = new char[outbuflen];
    unsigned short* input = reinterpret_cast<unsigned short*>(*_buf);

    /* Compress data. */
    ret = sqeazy::bswap1_lz4_pipe::compress(input, outbuf, input_nelem,outbuflen);

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



H5PL_type_t   H5PLget_plugin_type() {return H5PL_TYPE_FILTER;}
const void*   H5PLget_plugin_info() {return H5Z_SQY;}         

namespace sqeazy {

  struct loaded_hdf5_plugin {

    loaded_hdf5_plugin(){
      H5Zregister(H5Z_SQY);
      //std::cout << "registering H5Z_SQY\n";
    }

    ~loaded_hdf5_plugin(){
      H5Zunregister(H5Z_FILTER_SQY);
      //std::cout << "unregistering H5Z_FILTER_SQY\n";
    }
    
  };

}


#endif /* _SQEAZY_HDF5_IMPL_H_ */
