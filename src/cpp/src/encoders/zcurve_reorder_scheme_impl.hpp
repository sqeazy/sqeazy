#ifndef _ZCURVE_REORDER_SCHEME_IMPL_H_
#define _ZCURVE_REORDER_SCHEME_IMPL_H_

#include <sstream>
#include <string>
#include <functional>

#include "sqeazy_common.hpp"
#include "traits.hpp"
#include "dynamic_stage.hpp"
#include "string_parsers.hpp"

#include "morton.hpp"

namespace sqeazy {


  /**
* @brief this implements a shot noise type removal scheme in static member function encode, i.e. inside the neighborhood of a pixel
* in _input, the number of pixels are counted that fall under a certain threshold, if this count exceeds
* the limit "percentage_below" (or the one given at runtime), the central pixel
* (around which the neighborhood is located) is set to 0 as well.
*
* this scheme cannot be run inplace.
* this scheme is not reversable.
*
*/
  template <typename in_type>
  struct zcurve_reorder_scheme : public filter<in_type> {
  
    typedef filter<in_type> base_type;
    typedef in_type raw_type;
    typedef in_type compressed_type;
    typedef decltype(detail::morton_at_ct<>::from)	from_func_t;
    typedef decltype(detail::morton_at_ct<>::to)	to_func_t;

    static const std::string description() { return std::string("reorder the memory layout of the incoming buffer using space filling z curves"); };
    static const std::size_t default_tile_size = 16/sizeof(in_type);

    std::size_t tile_size;
    std::function<std::uint64_t(std::uint32_t,std::uint32_t,std::uint32_t)> zcurve_encode;
    std::function<void(std::uint64_t,std::uint32_t&,std::uint32_t&,std::uint32_t&)>  zcurve_decode;
    std::size_t max_index_per_dim;
    
    zcurve_reorder_scheme(const std::string& _payload=""):
      tile_size(default_tile_size),
      zcurve_encode(detail::morton_at_ct<>::from),
      zcurve_decode(detail::morton_at_ct<>::to),
      max_index_per_dim(0)
    {
      
      auto config_map = parse_string_by(_payload);

      if(config_map.size()){
	
      	auto f_itr = config_map.find("tile_size");
      	if(f_itr!=config_map.end()){
      	  tile_size = std::stoi(f_itr->second);

	  switch(tile_size){
	  case 2:
	    zcurve_encode = detail::morton_at_ct<>::from;
	    zcurve_decode = detail::morton_at_ct<>::to;
	    max_index_per_dim = 1 << detail::morton_at_ct<>::bits_per_dim;
	    break;
	  case 4:
	    zcurve_encode = detail::morton_at_ct<2>::from;
	    zcurve_decode = detail::morton_at_ct<2>::to;
	    max_index_per_dim = 1 << detail::morton_at_ct<2>::bits_per_dim;
	    break;
	  case 8:
	    zcurve_encode = detail::morton_at_ct<3>::from;
	    zcurve_decode = detail::morton_at_ct<3>::to;
	    max_index_per_dim = (1 << detail::morton_at_ct<3>::bits_per_dim);
	    break;
	  case 16:
	    zcurve_encode = detail::morton_at_ct<4>::from;
	    zcurve_decode = detail::morton_at_ct<4>::to;
	    max_index_per_dim = (1 << detail::morton_at_ct<4>::bits_per_dim);
	    break;
	  case 32:
	    zcurve_encode = detail::morton_at_ct<5>::from;
	    zcurve_decode = detail::morton_at_ct<5>::to;
	    max_index_per_dim = (1 << detail::morton_at_ct<5>::bits_per_dim);
	    break;
	  case 64:
	    zcurve_encode = detail::morton_at_ct<6>::from;
	    zcurve_decode = detail::morton_at_ct<6>::to;
	    max_index_per_dim = (1 << detail::morton_at_ct<6>::bits_per_dim);
	    break;
	  case 128:
	    zcurve_encode = detail::morton_at_ct<7>::from;
	    zcurve_decode = detail::morton_at_ct<7>::to;
	    max_index_per_dim = (1 << detail::morton_at_ct<7>::bits_per_dim);
	    break;
	  default:
	    zcurve_encode = detail::morton_at_ct<>::from;
	    zcurve_decode = detail::morton_at_ct<>::to;
	    max_index_per_dim = (1 << detail::morton_at_ct<1>::bits_per_dim);
	  };
	  
	}
	
      }
    }

    std::string name() const override final {

      std::ostringstream msg;
        msg << "zcurve_reorder";

        return msg.str();
	
    }

    std::string config() const override final {

      std::ostringstream msg;
      msg << "tile_size=" << std::to_string(tile_size);
      return msg.str();
    
    }

    std::intmax_t max_encoded_size(std::intmax_t _size_bytes) const override final {
    
      return _size_bytes;
    }

    compressed_type* encode( const raw_type* _input,
			     compressed_type* _output,
			     const std::vector<std::size_t>& _shape) override final {

      std::uint64_t dst = 0;
      // std::vector<std::size_t> safe_shape = _shape;
      // for(std::uint32_t i = 0;i<_shape.size();++i){
	
      // }
      
      const raw_type* itr = _input;
      
      for(std::uint32_t z = 0;z<_shape[row_major::z];++z)
	for(std::uint32_t y = 0;y<_shape[row_major::y];++y)
	  for(std::uint32_t x = 0;x<_shape[row_major::x];++x){

	    dst = zcurve_encode(z,y,x);
	    *(_output+dst) = *(itr++);
	    
	  }

      compressed_type* value = _output + (itr - _input);
      return value;
    }

    int decode( const compressed_type* _input,
		raw_type* _output,
		const std::vector<std::size_t>& _ishape,
		std::vector<std::size_t> _oshape = std::vector<std::size_t>()) const override final {

      std::size_t length = std::accumulate(_ishape.begin(), _ishape.end(), 1, std::multiplies<std::size_t>());

      std::uint64_t src = 0;
      raw_type* out = _output;
      
      for(std::uint32_t z = 0;z<_ishape[row_major::z];++z)
	for(std::uint32_t y = 0;y<_ishape[row_major::y];++y)
	  for(std::uint32_t x = 0;x<_ishape[row_major::x];++x){

	    src = zcurve_encode(z,y,x);
	    *out = *(_input + src);
	    ++out;
	  }
      
      if(out==(_output+length))
      	return SUCCESS;
      else
      	return FAILURE;
      
    }

    
    ~zcurve_reorder_scheme(){};

    std::string output_type() const final override {

      return typeid(compressed_type).name();
    
    }

    bool is_compressor() const final override {
    
      return base_type::is_compressor;
    
    }




  };

}

#endif /* _ZCURVE_REORDER_SCHEME_IMPL_H_ */
