#ifndef _HEVC_HPP_
#define _HEVC_HPP_
#include <cstdint>
#include <string>
#include <typeinfo>

#include "sqeazy_common.hpp"
#include "traits.hpp"

#include "dynamic_stage.hpp"



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
  struct hevc_scheme :  public sink<T,std::uint8_t> {

    typedef sink<T,std::uint8_t> sink_type;
    typedef T raw_type;
    typedef typename sink_type::out_type compressed_type;

    static_assert(std::is_arithmetic<raw_type>::value==true,"[hevc_scheme] input type is non-arithmetic");

    std::string hevc_config;

    //TODO: check syntax of hevc configuration at runtime
    hevc_scheme(const std::string& _payload=""):
      hevc_config(_payload){

    }

    std::string name() const override final {

      return std::string("hevc");
	
    }


    /**
       \brief serialize the parameters of this filter
     
       \return 
       \retval string .. that encodes the configuration paramters
     
    */
    std::string config() const {

      return hevc_config;
    
    }

    //TODO: not implemented yet!!
    std::intmax_t max_encoded_size(std::intmax_t _size_bytes) const override final {
    
      return _size_bytes;
      
    }
    
    /**
     * @brief encode input raw_type buffer and write to output (not owned, not allocated)
     *
     * @param _input input raw_type buffer
     * @param _output output char buffer (not owned, not allocated)
     * @param _length mutable std::vector<size_type>, contains the length of _input at [0] and the number of written bytes at [1]
     * @return sqeazy::error_code
     */
    compressed_type* encode( const raw_type* _in, compressed_type* _out, std::vector<std::size_t> _shape) override final {

      typedef typename sqeazy::twice_as_wide<size_t>::type local_size_type;
      
      local_size_type total_length = std::accumulate(_shape.begin(), _shape.end(), 1, std::multiplies<size_t>());
      local_size_type total_length_in_byte = total_length*sizeof(raw_type);
      local_size_type max_payload_length_in_byte = static_max_encoded_size(total_length_in_byte);

	
      int drange = sizeof(raw_type)*CHAR_BIT;
      std::vector<compressed_type> temp_out;	
      std::vector<raw_type> temp_in(_in,_in + total_length);
	
      size_t num_written_bytes = 0;

      //normalize data if lowest_set_bit is greater than 0
      //NB. this is lossy compression
      if(sizeof(raw_type)!=1){
	// const compressed_type* input = reinterpret_cast<const compressed_type*>(_in);
	const int max_bit_set = sqeazy::highest_set_bit(_in,_in + total_length);
	const int min_bit_set = sqeazy::lowest_set_bit(_in,_in + total_length);
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
					      _shape,
					      temp_out);
      else {
	//TODO: apply quantisation
	num_written_bytes = 0;
      }

      if(num_written_bytes > 0) 
	std::copy(temp_out.begin(), temp_out.end(),_out);
      
      std::intmax_t compressed_elements = num_written_bytes*sizeof(compressed_type);
      return _out+compressed_elements;
      
    }

    //FIXME: the output buffer will most likely be larger than the input buffer
    int decode( const compressed_type* _in, raw_type* _out, std::vector<std::size_t> _shape) const override final {

      size_t _len_in = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<size_t>());
      size_t _len_out = max_encoded_size(_len_in);
	
      const uint8_t* input = reinterpret_cast<const uint8_t*>(_in);
      
      std::size_t num_bytes_decoded = hevc_decode_stack(input, _len_in, _out, _len_out);
      
      return ( num_bytes_decoded > 0 && num_bytes_decoded == _len_out ) ? SUCCESS : FAILURE;
      
            
    }


    ~hevc_scheme(){};

    std::string output_type() const final override {

      return typeid(compressed_type).name();
    
    }

    bool is_compressor() const final override {
    
      return sink<T>::is_compressor;
    
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // DEPRECATED API

    // typedef T raw_type;
    // typedef uint8_t compressed_type;
    typedef S size_type;
  
    static S last_num_encoded_bytes;
  
    static const bool is_sink = true;
  
    static const std::string static_name() {
    
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
    static const error_code static_encode(const raw_type* _input,
                                   compressed_type* _output,
                                   std::vector<SizeType>& _dims,//size of _input x-d,y-d,z-d
                                   size_type& _bytes_written = last_num_encoded_bytes
                                  ) {
      
        typedef typename sqeazy::twice_as_wide<SizeType>::type local_size_type;
        local_size_type total_length = std::accumulate(_dims.begin(), _dims.end(), 1, std::multiplies<SizeType>());
        local_size_type total_length_in_byte = total_length*sizeof(raw_type);
	local_size_type max_payload_length_in_byte = static_max_encoded_size(total_length_in_byte);

	
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
	  num_written_bytes = hevc_encode_stack(&temp_in[0],
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
    static const error_code static_encode(const raw_type* _input,
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
    static const error_code static_decode(const compressed_type* _input,
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
    static const unsigned long static_max_encoded_size(U _src_length_in_bytes) {

      return _src_length_in_bytes;

    }

    
  };

  template < typename T , typename S>
  S hevc_scheme<T,S>::last_num_encoded_bytes = 0;

  
};//sqy namespace

#endif /* _HEVC_H_ */
