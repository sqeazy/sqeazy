#ifndef _PASS_THROUGH_SCHEME_UTILS_H_
#define _PASS_THROUGH_SCHEME_UTILS_H_

#include <functional>
#include <algorithm>
#include <sstream>
#include <climits>
#include <numeric>
#include "sqeazy_common.hpp"
#include "traits.hpp"
#include "dynamic_stage.hpp"

namespace sqeazy {
  
  template <typename in_type, typename out_type = char >
  struct pass_through : public sink<in_type,out_type>  {

    typedef sink<in_type,out_type> base_type;
    typedef in_type raw_type;
    typedef typename base_type::out_type compressed_type;


    static_assert(std::is_arithmetic<raw_type>::value==true,"[pass_through] input type is non-arithmetic");

    static const std::string description() { return std::string("pass/copy content to next stage"); };
    
    //TODO: check syntax of lz4 configuration at runtime
    pass_through(const std::string& _payload="")
    {

    }




    std::string name() const override final {

      return std::string("pass_through");
	
    }


    /**
       \brief serialize the parameters of this filter
     
       \return 
       \retval string .. that encodes the configuration paramters
     
    */
    std::string config() const {

      return "";
    
    }

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

      
      std::size_t in_elements = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());
      raw_type* out_begin = reinterpret_cast<raw_type*>(_out);
      std::copy(_in,_in+in_elements,out_begin);

      std::size_t num_written_bytes = in_elements*sizeof(raw_type)/sizeof(compressed_type);
      
      return _out+num_written_bytes;
    }



    int decode( const compressed_type* _in, raw_type* _out,
		std::vector<std::size_t> _inshape,
		std::vector<std::size_t> _outshape = std::vector<std::size_t>()) const override final {

      if(_outshape.empty())
	_outshape = _inshape;
      
      std::size_t in_elements = std::accumulate(_inshape.begin(), _inshape.end(),1,std::multiplies<std::size_t>());
      compressed_type* out_begin = reinterpret_cast<compressed_type*>(_out);
      std::copy(_in,_in+in_elements,out_begin);
      
      return 0;
      
    }
    
    

    ~pass_through(){};

    std::string output_type() const final override {

      return typeid(compressed_type).name();
    
    }

    bool is_compressor() const final override {
    
      return sink<in_type>::is_compressor;
    
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // DEPRECATED API
    
    static const bool is_sink = false;

    /**
     * @brief producing the name of this scheme and return it as a string
     *
     * @return const std::string
     */
    static const std::string static_name() {

      //TODO: add name of Neighborhood
      std::ostringstream msg;
      msg << "" ;
      return msg.str();

    }


    template <typename size_type>
    /**
     * @brief encoding the diff scheme, i.e. the output value for input intensity I is equal to the sum 
     * of the neighborhood divided by the number of pixels traversed in the neighborhood
     * 
     * @param _width width of input stack
     * @param _height height of the input stack
     * @param _depth depth of the input stack
     * @param _input input stack of type raw_type
     * @param _output output stack of type compressed_type but same extent than the input
     * @return sqeazy::error_code
     */
    static const error_code static_encode(const size_type& _width,
					  const size_type& _height,
					  const size_type& _depth,
					  const raw_type* _input,
					  compressed_type* _output)
    {

      unsigned long length = _width*_height*_depth;
      std::copy(_input, _input + length, _output);//crossing fingers due to possible type mismatch
      return SUCCESS;
    }


    template <typename size_type>
    static const error_code static_encode(const raw_type* _input,
					  compressed_type* _output,
					  size_type& _dim
					  )
    {

      return static_encode(_dim, 1, 1, _input, _output);
    }

    template <typename size_type>
    static const error_code static_encode(const raw_type* _input,
					  compressed_type* _output,
					  std::vector<size_type>& _dims
					  )
    {
      return static_encode(_dims.at(0), _dims.at(1), _dims.at(2), _input, _output);
    }

    template <typename size_type>
    /**
     * @brief reconstructing data that was encoded by this diff scheme
     * 
     * @return sqeazy::error_code
     */
    static const error_code static_decode(const size_type& _width,
					  const size_type& _height,
					  const size_type& _depth,
					  const compressed_type* _input,
					  raw_type* _output)
    {
      unsigned long length = _width*_height*_depth;
      std::copy(_input,_input + length, _output);
      return SUCCESS;
    }


    template <typename size_type>
    static const error_code static_decode(const compressed_type* _input,
					  raw_type* _output,
					  std::vector<size_type>& _dims
					  ) {

      return static_decode(_dims.at(0), _dims.at(1), _dims.at(2), _input, _output);

    }

    template <typename size_type>
    static const error_code static_decode(const compressed_type* _input,
					  raw_type* _output,
					  size_type& _dim
					  ) {

      const size_type one = 1;
      return static_decode(_dim, one, one, _input, _output);

    }
  
    template <typename U>
    static const unsigned long max_encoded_size(U _src_length_in_bytes){
      return _src_length_in_bytes;
    }
  };

} //sqeazy
#endif /* _PASS_THROUGH_SCHEME_UTILS_H_ */
