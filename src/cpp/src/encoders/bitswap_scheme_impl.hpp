#ifndef _BITSWAP_SCHEME_IMPL_H_
#define _BITSWAP_SCHEME_IMPL_H_

#include "neighborhood_utils.hpp"
#include "sqeazy_common.hpp"
#include "traits.hpp"
#include "bitswap_scheme_utils.hpp"
#include "dynamic_stage.hpp"
#include "string_parsers.hpp"

namespace sqeazy {

  //TODO: remove static_num_bits_per_plane as soon as deprecated API is gone
  template <typename in_type,const unsigned static_num_bits_per_plane = 1>
  struct bitswap_scheme : public filter<in_type> {
  
    typedef filter<in_type> base_type;
    typedef in_type raw_type;
    typedef in_type compressed_type;

    std::uint32_t num_bits_per_plane;
    std::uint32_t num_planes;
    
    static_assert(std::is_arithmetic<raw_type>::value==true,"[bitswap_scheme] input type is non-arithmetic");
    static const std::string description() { return std::string("rewrite bitplanes of item as chunks of buffer, use <num_bits_per_plane|default = 1> to control how many bits each plane has"); };
    
    //TODO: check syntax of lz4 configuration at runtime
    bitswap_scheme(const std::string& _payload=""):
      num_bits_per_plane(static_num_bits_per_plane),
      num_planes(0)
    {
      
      auto config_map = parse_string_by(_payload);

      if(config_map.size()){
	auto f_itr = config_map.find("num_bits_per_plane");
	std::uint32_t found_num_bits_per_plane = 0;
	if(f_itr!=config_map.end())
	  found_num_bits_per_plane = std::stoi(f_itr->second);

	if(found_num_bits_per_plane!=num_bits_per_plane)
	  std::cerr << "[bitswap_scheme] found num_bits_per_plane = " << found_num_bits_per_plane
		    << " but was configured with " << static_num_bits_per_plane << ", proceed with caution!\n";
      }

      const std::uint32_t raw_type_num_bits = sizeof(raw_type)*CHAR_BIT;
      num_planes = raw_type_num_bits/num_bits_per_plane;
    }


    std::string name() const override final {

      std::ostringstream msg;
      msg << "bitswap"
	  << num_bits_per_plane;
      return msg.str();
	
    }


    /**
       \brief serialize the parameters of this filter
     
       \return 
       \retval string .. that encodes the configuration paramters
     
    */
    std::string config() const {

      std::ostringstream msg;
      msg << "num_bits_per_plane=" << num_bits_per_plane;
      return msg.str();
    
    }

    std::intmax_t max_encoded_size(std::intmax_t _size_bytes) const override final {
    
      return _size_bytes;
    }

    compressed_type* encode( const raw_type* _input, compressed_type* _output, std::size_t _length) override final {

      
      std::size_t max_size = _length - (_length % num_planes);
      if(max_size < _length)
	std::copy(_input+max_size,_input+_length,_output+max_size);

      int err = 0;
      if(sqeazy::platform::use_vectorisation::value && num_bits_per_plane==1){
#ifdef _SQY_VERBOSE_
	std::cout << "[bitswap_scheme::encode]\tusing see method\n";
#endif
	err = sqeazy::detail::sse_bitplane_reorder_encode<static_num_bits_per_plane>(_input, 
										     _output, 
										     max_size);
	
      }
      else{
#ifdef _SQY_VERBOSE_
	std::cout << "[bitswap_scheme::encode]\tusing scalar method\n";
#endif
	err = sqeazy::detail::scalar_bitplane_reorder_encode<static_num_bits_per_plane>(_input, 
											_output, 
											max_size);
      }

      if(!err)
	return _output+(_length);
      else
	return nullptr;

    }
   
    compressed_type* encode( const raw_type* _input, compressed_type* _output, const std::vector<std::size_t>& _shape) override final {

      size_t _length = std::accumulate(_shape.begin(),
				       _shape.end(),
				       1,
				       std::multiplies<std::size_t>());

      return encode(_input,_output,_length);
      
      
    }



    int decode( const compressed_type* _input, raw_type* _output,
		const std::vector<std::size_t>& _ishape,
		std::vector<std::size_t> _oshape = std::vector<std::size_t>()) const override final {

      if(_oshape.empty())
	_oshape = _ishape;
      
      typedef typename sqeazy::twice_as_wide<std::size_t>::type size_type;
      size_type _length = std::accumulate(_ishape.begin(),
					  _ishape.end(),
					  1,
					  std::multiplies<std::size_t>());
      size_type max_size = _length - (_length % num_planes);
      if(max_size < _length)
	std::copy(_input+max_size,_input+_length,_output+max_size);
      return sqeazy::detail::scalar_bitplane_reorder_decode<static_num_bits_per_plane>(_input, _output, max_size);
    }
    
    int decode( const compressed_type* _input,
		raw_type* _output,
		std::size_t _length,
		std::size_t _olength = 0) const override final
    {

      if(!_olength)
	_olength = _length;
      
      typedef typename sqeazy::twice_as_wide<std::size_t>::type size_type;
      size_type max_size = _length - (_length % num_planes);
      if(max_size < _length)
	std::copy(_input+max_size,_input+_length,_output+max_size);
      return sqeazy::detail::scalar_bitplane_reorder_decode<static_num_bits_per_plane>(_input, _output, max_size);
    }
    

    ~bitswap_scheme(){};

    std::string output_type() const final override {

      return typeid(compressed_type).name();
    
    }

    bool is_compressor() const final override {
    
      return base_type::is_compressor;
    
    }









    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // DEPRECATED API
    static const bool is_sink = false;

    static const unsigned static_raw_type_num_bits = sizeof(raw_type)*CHAR_BIT;
    static const unsigned static_num_planes = static_raw_type_num_bits/static_num_bits_per_plane;

    /**
     * @brief producing the name of this scheme and return it as a string
     *
     * @return const std::string
     */
    static const std::string static_name() {

      std::ostringstream val("");
      val << "bswap" << static_num_bits_per_plane;
      return val.str();

    }

    template <typename S>
    /**
     * @brief performs bitplane encoding, every bit of any value from _input is written to the respective 
     * compartment in _output, the granularity of the bit decomposition is given by static_num_bits_per_plane
     * that means if static_num_bits_per_plane = 1 and the input type is 8-bit wide then the output buffer is 
     * divided in 8 compartments in which the values of the nth bit of every input value are written to
     * 
     * @param _input 3D input stack
     * @param _output 3D output stack of same type and extent as _input
     * @param _length vector of extents
     * @return sqeazy::error_code
     */
    static const error_code static_encode(const raw_type* _input,
					  raw_type* _output,
					  const std::vector<S>& _length)
    {
      typename sqeazy::twice_as_wide<S>::type total_length = std::accumulate(_length.begin(), _length.end(), 1, std::multiplies<S>());
      return static_encode(_input, _output, total_length);
    }

    
    /**
     * @brief performs bitplane encoding, every bit of any value from _input is written to the respective 
     * compartment in _output, the granularity of the bit decomposition is given by static_num_bits_per_plane
     * that means if static_num_bits_per_plane = 1 and the input type is 8-bit wide then the output buffer is 
     * divided in 8 compartments in which the values of the nth bit of every input value are written to
     * 
     * @param _input 3D input stack
     * @param _output 3D output stack of same type and extent as _input
     * @param _length as a scalar
     * @return sqeazy::error_code
     */
    template <typename size_type>
    static const error_code static_encode(const raw_type* _input,
					  raw_type* _output,
					  const size_type& _length)
    {

      size_type max_size = _length - (_length % static_num_planes);
      if(max_size < _length)
	std::copy(_input+max_size,_input+_length,_output+max_size);
      
      if(sqeazy::platform::use_vectorisation::value && static_num_bits_per_plane==1){
#ifdef _SQY_VERBOSE_
	std::cout << "[bitplane encode]\tusing see method\n";
#endif
	return sqeazy::detail::sse_bitplane_reorder_encode<static_num_bits_per_plane>(_input, 
									       _output, 
									       max_size);
      }
      else{
#ifdef _SQY_VERBOSE_
	std::cout << "[bitplane encode]\tusing scalar method\n";
#endif	
	return sqeazy::detail::scalar_bitplane_reorder_encode<static_num_bits_per_plane>(_input, 
										  _output, 
										  max_size);
      }
    }

    template <typename S>
    static const error_code static_decode(const raw_type* _input,
					  raw_type* _output,
					  const std::vector<S>& _length)
    {
      typename sqeazy::twice_as_wide<S>::type total_length = std::accumulate(_length.begin(), _length.end(), 1, std::multiplies<S>());
      return static_decode(_input, _output, total_length);
    }

    template <typename size_type>
    static const error_code static_decode(const raw_type* _input,
					  raw_type* _output,
					  const size_type& _length)
    {
      size_type max_size = _length - (_length % static_num_planes);
      if(max_size < _length)
	std::copy(_input+max_size,_input+_length,_output+max_size);
      return sqeazy::detail::scalar_bitplane_reorder_decode<static_num_bits_per_plane>(_input, _output, max_size);
    }


  };

}

#endif /* _BITSWAP_SCHEME_IMPL_H_ */
