#ifndef _LZ4_HPP_
#define _LZ4_HPP_
#include <cstdint>
#include <string>
#include <typeinfo>

#include "sqeazy_common.hpp"
#include "traits.hpp"

#include "dynamic_stage.hpp"

//TODO: what if lz4 is not available??
#ifndef LZ4_VERSION_MAJOR
#include "lz4.h"
#endif

namespace sqeazy {

 

  template < typename T , typename S = std::size_t>
  struct lz4_scheme :  public sink<T> {

    typedef sink<T,char> sink_type;
    typedef T raw_type;
    typedef typename sink_type::out_type compressed_type;

    static_assert(std::is_arithmetic<raw_type>::value==true,"[lz4_scheme] input type is non-arithmetic");

    std::string lz4_config;

    //TODO: check syntax of lz4 configuration at runtime
    lz4_scheme(const std::string& _payload=""):
      lz4_config(_payload){

    }




    std::string name() const override final {

      return std::string("lz4");
	
    }


    /**
       \brief serialize the parameters of this filter
     
       \return 
       \retval string .. that encodes the configuration paramters
     
    */
    std::string config() const {

      return lz4_config;
    
    }

    std::intmax_t max_encoded_size(std::intmax_t _size_bytes) const override final {
    
      std::intmax_t lz4_bound = LZ4_compressBound(_size_bytes);

      return lz4_bound;
    }

    /**
     * @brief encode input raw_type buffer and write to output (not owned, not allocated)
     *
     * @param _input input raw_type buffer
     * @param _output output char buffer (not owned, not allocated)
     * @param _length number of char items in _in
     * @return sqeazy::error_code
     */
    compressed_type* encode( const raw_type* _in, compressed_type* _out, std::size_t _length) override final {
      std::vector<std::size_t> shape = {_length};
      return encode(_in,_out,shape);
    }
    
    /**
     * @brief encode input raw_type buffer and write to output (not owned, not allocated)
     *
     * @param _input input raw_type buffer
     * @param _output output char buffer (not owned, not allocated)
     * @param _shape mutable std::vector<size_type>, contains the length of _input at [0] and the number of written bytes at [1]
     * @return pointer to end of payload
     */
    compressed_type* encode( const raw_type* _in, compressed_type* _out, std::vector<std::size_t> _shape) override final {

      typedef typename sqeazy::twice_as_wide<size_t>::type local_size_type;
      
      local_size_type total_length = std::accumulate(_shape.begin(), _shape.end(), 1, std::multiplies<size_t>());
      local_size_type total_length_in_byte = total_length*sizeof(raw_type);
      
      local_size_type max_payload_length_in_byte = LZ4_compressBound(total_length_in_byte);

      const compressed_type* input = reinterpret_cast<const compressed_type*>(_in);

      size_type num_written_bytes = LZ4_compress_limitedOutput(input,
							       _out,
							       total_length_in_byte,
							       max_payload_length_in_byte);

      return _out+num_written_bytes;
    }



    int decode( const compressed_type* _in, raw_type* _out,
		std::vector<std::size_t> _inshape,
		std::vector<std::size_t> _outshape = std::vector<std::size_t>()) const override final {

      if(_outshape.empty())
	_outshape = _inshape;
      
      size_type _len_in = std::accumulate(_inshape.begin(), _inshape.end(),1,std::multiplies<size_type>());
      size_type _len_out = std::accumulate(_outshape.begin(), _outshape.end(),1,std::multiplies<size_type>());

      return decode(_in,_out,_len_in,_len_out);
    }

    int decode( const compressed_type* _in, raw_type* _out,
		std::size_t _inlen,
		std::size_t _outlen=0)  const override final {

      if(!_outlen)
	_outlen = _inlen;
      
      compressed_type* output = reinterpret_cast<compressed_type*>(_out);

      const int expected_bytes_decoded = _outlen*sizeof(raw_type);
      int num_bytes_decoded = LZ4_decompress_safe(_in,
						  output,
						  _inlen*sizeof(compressed_type),
						  expected_bytes_decoded);

      if(num_bytes_decoded == expected_bytes_decoded)
	return 0;
      else
	return 1;
      
    }
    
    

    ~lz4_scheme(){};

    std::string output_type() const final override {

      return typeid(compressed_type).name();
    
    }

    bool is_compressor() const final override {
    
      return sink<T>::is_compressor;
    
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // DEPRECATED API
    typedef S size_type;

    static S last_num_encoded_bytes;

    static const bool is_sink = true;

    static const std::string static_name() {

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
    static const error_code static_encode(const raw_type* _input,
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
    static const error_code static_encode(const raw_type* _input,
                                   compressed_type* _output,
                                   const size_type& _length,//size of _input
                                   size_type& _bytes_written = last_num_encoded_bytes
                                  ) {

        std::vector<size_type> artificial_dims(1);
        artificial_dims[0] = _length;

        return static_encode(_input, _output, artificial_dims, _bytes_written);
	
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
    static const error_code static_decode(const compressed_type* _input,
                                   raw_type* _output,
                                   const size_type& _len_in,
                                   const size_type& _len_out
                                  ) {
      
      char* output = reinterpret_cast<char*>(_output);
      const char* input = reinterpret_cast<const char*>(_input);
      
      size_type num_bytes_decoded = LZ4_decompress_safe(input,output,_len_in, _len_out);

      return ( num_bytes_decoded > 0 && num_bytes_decoded == _len_out ) ? SUCCESS : FAILURE;

    }

    //////////////////////////////////////////////////////////////////////////
    //TODO: the following functions are actually not very compressor specific
    //      -> refactor to policy!

    template <typename U>
    static const unsigned long static_max_encoded_size(U _src_length_in_bytes) {

        unsigned long lz4_bound = LZ4_compressBound(_src_length_in_bytes);

        return lz4_bound;

    }
    
    
  };

template < typename T , typename S>
S lz4_scheme<T,S>::last_num_encoded_bytes = 0;


};//sqy namespace


#endif /* _LZ4_H_ */
