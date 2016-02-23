#ifndef _EXTERNAL_ENCODERS_HPP_
#define _EXTERNAL_ENCODERS_HPP_
#include <cstdint>
#include <string>
#include <typeinfo>
#include "sqeazy_common.hpp"
#include "sqeazy_traits.hpp"

//TODO: what if lz4 is not available??
#ifndef LZ4_VERSION_MAJOR
#include "lz4.h"
#endif

namespace sqeazy {

 

  template < typename T , typename S = std::size_t>
struct lz4_scheme {

    typedef T raw_type;
    typedef char compressed_type;
    typedef S size_type;

    static S last_num_encoded_bytes;

    static const bool is_compressor = true;

    static const std::string name() {

        return std::string("lz4");
	
    }


    /**
     * @brief encode input raw_type buffer and write to output (not owned, not allocated)
     *
     * @param _input input raw_type buffer
     * @param _output output char buffer (not owned, not allocated)
     * @param _length mutable std::vector<size_type>, contains the length of _input at [0] and the number of written bytes at [1]
     * @return sqeazy::error_code
     */
    template <typename SizeType>
    static const error_code encode(const raw_type* _input,
                                   compressed_type* _output,
                                   std::vector<SizeType>& _dims,//size of _input
                                   size_type& _bytes_written = last_num_encoded_bytes
                                  ) {
      
        typedef typename sqeazy::twice_as_wide<SizeType>::type local_size_type;
        local_size_type total_length = std::accumulate(_dims.begin(), _dims.end(), 1, std::multiplies<SizeType>());
        local_size_type total_length_in_byte = total_length*sizeof(raw_type);
	local_size_type max_payload_length_in_byte = LZ4_compressBound(total_length_in_byte);

        const compressed_type* input = reinterpret_cast<const compressed_type*>(_input);

        size_type num_written_bytes = LZ4_compress_limitedOutput(input,
								 _output,
								 total_length_in_byte,
								 max_payload_length_in_byte);

        if(num_written_bytes > 0) {
            _bytes_written = num_written_bytes;
            last_num_encoded_bytes = _bytes_written;
            return SUCCESS;
        }
        else {
            _bytes_written = 0;
            last_num_encoded_bytes = 0;
            return FAILURE;
        }

    }


    /**
     * @brief encode _input using the lz4 library, the output contains the number of input elements encoded as 64bit signed integer (long) and the encoded data
     *
     * @param _input array/buffer that contains data of raw_type
     * @param _output the char buffer that contains the compressed data (not truncated, not owned)
     * @param _length size of _input (unmutable)
     * @param _bytes_written number of bytes/elements in _output that actually contain data
     * @return sqeazy::error_code SUCCESS if non-zero number of bytes have been encoded by lz4
     */
    static const error_code encode(const raw_type* _input,
                                   compressed_type* _output,
                                   const size_type& _length,//size of _input
                                   size_type& _bytes_written = last_num_encoded_bytes
                                  ) {

        std::vector<size_type> artificial_dims(1);
        artificial_dims[0] = _length;

        return encode(_input, _output, artificial_dims, _bytes_written);
	
    }

    /**
     * @brief decode the input data stream _input with lz4 (the number of output elements is extracted from the first 64
     * bit of the input stream)
     *
     * @param _input input buffer as char
     * @param _output output buffer (needs to be allocated outside this function)
     * @param _length number of elements in _input
     * @return sqeazy::error_code
     */
    static const error_code decode(const compressed_type* _input,
                                   raw_type* _output,
                                   const size_type& _len_in,
                                   const size_type& _len_out
                                  ) {
      
      compressed_type* output = reinterpret_cast<compressed_type*>(_output);
      size_type num_bytes_decoded = LZ4_decompress_safe(_input,output,_len_in, _len_out);

      return ( num_bytes_decoded > 0 && num_bytes_decoded == _len_out ) ? SUCCESS : FAILURE;

    }

    //////////////////////////////////////////////////////////////////////////
    //TODO: the following functions are actually not very compressor specific
    //      -> refactor to policy!

    template <typename U>
    static const unsigned long max_encoded_size(U _src_length_in_bytes) {

        unsigned long lz4_bound = LZ4_compressBound(_src_length_in_bytes);

        return lz4_bound;

    }
    


};

template < typename T , typename S>
S lz4_scheme<T,S>::last_num_encoded_bytes = 0;


};//sqy namespace


//TODO: what if ffmpeg is not available??
#ifndef AV_CODEC_ID_HEVC
extern "C" {

#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
  
}

#endif


#include <type_traits>

#include "hevc_scheme_impl.hpp"

namespace sqeazy {

  
  template < typename T , typename S = std::size_t>
  struct hevc_scheme {

    typedef T raw_type;
    typedef uint8_t compressed_type;
    typedef S size_type;
  
    static S last_num_encoded_bytes;
  
    static const bool is_compressor = true;
  
    static const std::string name() {
    
      return std::string("hevc");
	
    }

    //static_assert(std::is_same<raw_type,uint8_t>::value, "[hevc_scheme] other types than uint8_t are not supported (yet)");

    
    /**
     * @brief encode input raw_type buffer and write to output (not owned, not allocated)
     *
     * @param _input input raw_type buffer
     * @param _output output char buffer (not owned, not allocated)
     * @param _length mutable std::vector<size_type>, contains the length of _input at [0] and the number of written bytes at [1]
     * @return sqeazy::error_code
     */
    template <typename SizeType>
    static const error_code encode(const raw_type* _input,
                                   compressed_type* _output,
                                   std::vector<SizeType>& _dims,//size of _input x-d,y-d,z-d
                                   size_type& _bytes_written = last_num_encoded_bytes
                                  ) {
      
        typedef typename sqeazy::twice_as_wide<SizeType>::type local_size_type;
        local_size_type total_length = std::accumulate(_dims.begin(), _dims.end(), 1, std::multiplies<SizeType>());
        local_size_type total_length_in_byte = total_length*sizeof(raw_type);
	local_size_type max_payload_length_in_byte = max_encoded_size(total_length_in_byte);

	
	int drange = sizeof(raw_type)*CHAR_BIT;
	std::vector<compressed_type> temp_out;	
	std::vector<raw_type> temp_in(_input,_input + total_length);
	
	size_type num_written_bytes = 0;

	//normalize data if lowest_set_bit is greater than 0
	//NB. this induces lossy compression
	if(sizeof(raw_type)!=1){
	  // const compressed_type* input = reinterpret_cast<const compressed_type*>(_input);
	  const int max_bit_set = sqeazy::highest_set_bit(_input,_input + total_length);
	  const int min_bit_set = sqeazy::lowest_set_bit(_input,_input + total_length);
	  drange = max_bit_set - min_bit_set;

	  if(drange<9)
	    std::transform(temp_in.begin(), temp_in.end(),
			   temp_in.begin(),
			   [&](raw_type& _item){
			     if(_item)
			       return _item >> min_bit_set;
			   });
	} 

	if(drange<9)
	  num_written_bytes = hevc_encode_stack((uint8_t*)&temp_in[0],
						_dims,
						temp_out);
	else {
	  //TODO: apply quantisation
	  num_written_bytes = 0;
	}

        if(num_written_bytes > 0) {
            _bytes_written = num_written_bytes;
            last_num_encoded_bytes = _bytes_written;
	    std::copy(temp_out.begin(), temp_out.end(),_output);
            return SUCCESS;
        }
        else {
            _bytes_written = 0;
            last_num_encoded_bytes = 0;
            return FAILURE;
        }

    }


    /**
     * @brief encode _input using the hevc library, the output contains the number of input elements encoded as 64bit signed integer (long) and the encoded data
     *
     * @param _input array/buffer that contains data of raw_type
     * @param _output the char buffer that contains the compressed data (not truncated, not owned)
     * @param _length size of _input (unmutable)
     * @param _bytes_written number of bytes/elements in _output that actually contain data
     * @return sqeazy::error_code SUCCESS if non-zero number of bytes have been encoded by hevc
     */
    static const error_code encode(const raw_type* _input,
                                   compressed_type* _output,
                                   const size_type& _length,//size of _input
                                   size_type& _bytes_written = last_num_encoded_bytes
                                  ) {


        return FAILURE// encode(_input, _output, artificial_dims, _bytes_written)
	  ;
	
    }

    /**
     * @brief decode the input data stream _input with hevc (the number of output elements is extracted from the first 64
     * bit of the input stream)
     *
     * @param _input input buffer as char
     * @param _output output buffer (needs to be allocated outside this function)
     * @param _length number of elements in _input
     * @return sqeazy::error_code
     */
    static const error_code decode(const compressed_type* _input,
                                   raw_type* _output,
                                   const size_type& _len_in,
                                   const size_type& _len_out
                                  ) {
      
      const uint8_t* input = reinterpret_cast<const uint8_t*>(_input);
      
      size_type num_bytes_decoded = hevc_decode_stack(input, _len_in, _output, _len_out);

      return ( num_bytes_decoded > 0 && num_bytes_decoded == _len_out ) ? SUCCESS : FAILURE;

    }

    //////////////////////////////////////////////////////////////////////////
    //TODO: the following functions are actually not very compressor specific
    //      -> refactor to policy!

    template <typename U>
    static const unsigned long max_encoded_size(U _src_length_in_bytes) {

      return _src_length_in_bytes;

    }

    
  };

  template < typename T , typename S>
  S hevc_scheme<T,S>::last_num_encoded_bytes = 0;

  
};//sqy namespace

#endif /* _EXTERNAL_ENCODERS_H_ */
