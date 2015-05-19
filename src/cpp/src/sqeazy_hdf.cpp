#define _SQEAZY_HDF_CPP_

#include "hdf5.h"
#include "sqeazy.h"
#include "sqeazy_impl.hpp"
#include "sqeazy_header.hpp"
#include "sqeazy_predef_pipelines.hpp"

const H5Z_class2_t H5Z_SQY[1] = {{
    H5Z_CLASS_T_VERS,       /* H5Z_class_t version */
    (H5Z_filter_t)H5Z_FILTER_SQY,         /* Filter id number             */
    1,              /* encoder_present flag (set to true) */
    1,              /* decoder_present flag (set to true) */
    "sqy",                  /* Filter name for debugging    */
    NULL,                       /* The "can apply" callback     */
    NULL,                       /* The "set local" callback     */
    (H5Z_func_t)H5Z_filter_sqy,         /* The actual filter function   */
}};


size_t H5Z_filter_sqy(unsigned int _flags, //is it encode or decode
			size_t _cd_nelmts,
			const unsigned int _cd_values[], 
			size_t _nbytes,
			size_t *_buf_size, 
			void **_buf
			)
{
  char *outbuf = NULL;
  size_t outbuflen;
  int ret;

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
    outbuflen = sqeazy::bswap1_lz4_pipe::max_bytes_encoded(_nbytes,
							   sqeazy::bswap1_lz4_pipe::header_size(input_nelem)
							   ); 

    outbuf = new char[outbuflen];
    unsigned short* input = reinterpret_cast<unsigned short*>(*_buf);

    /* Compress data. */
    ret = sqeazy::bswap1_lz4_pipe::compress(input, outbuf, input_nelem);

  }

  /* Always replace the input buffer with the output buffer. */
  if(!ret){
    delete [] _buf;
    *_buf = outbuf;
    *_buf_size = outbuflen;


  }
  else{
    delete [] outbuf;
  }
  return ret;
  
}



H5PL_type_t   H5PLget_plugin_type() {return H5PL_TYPE_FILTER;}
const void*   H5PLget_plugin_info() {return H5Z_SQY;}         
