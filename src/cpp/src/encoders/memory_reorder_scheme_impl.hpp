#ifndef _MEMORY_REORDER_SCHEME_IMPL_H_
#define _MEMORY_REORDER_SCHEME_IMPL_H_

#include <sstream>
#include <string>

#include "sqeazy_common.hpp"
#include "traits.hpp"
#include "dynamic_stage.hpp"
#include "string_parsers.hpp"

#include "memory_reorder_utils.hpp"

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
  struct memory_reorder_scheme : public filter<in_type> {
  
    typedef filter<in_type> base_type;
    typedef in_type raw_type;
    typedef in_type compressed_type;

    static const std::string description() { return std::string("reorder the memory layout of the incoming buffer"); };
    static const std::size_t default_tile_size = 16/sizeof(in_type);

    std::size_t tile_size;
    
    // memory_reorder_scheme():
    //   tile_size(default_tile_size){
    // }

    
    memory_reorder_scheme(const std::string& _payload=""):
      tile_size(default_tile_size)
    {
      
      auto config_map = parse_string_by(_payload);

      if(config_map.size()){
	
      	auto f_itr = config_map.find("tile_size");
      	if(f_itr!=config_map.end())
      	  tile_size = std::stoi(f_itr->second);
	
      }
    }

    std::string name() const override final {

      std::ostringstream msg;
        msg << "memory_reorder";

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

      typedef std::size_t size_type;
      unsigned long length = std::accumulate(_shape.begin(), _shape.end(), 1, std::multiplies<size_type>());

      detail::reorder tiles_of(tile_size);
      
      auto value = tiles_of.encode(_input, _input+length,
				   _output,
				   _shape);

      return value;
    }

    int decode( const compressed_type* _input,
		raw_type* _output,
		const std::vector<std::size_t>& _ishape,
		std::vector<std::size_t> _oshape = std::vector<std::size_t>()) const override final {

      std::size_t length = std::accumulate(_ishape.begin(), _ishape.end(), 1, std::multiplies<std::size_t>());

      detail::reorder tiles_of(tile_size);
      
      auto value = tiles_of.decode(_input, _input+length,
				   _output,
				   _ishape);

      if(value==(_output+length))
	return SUCCESS;
      else
	return FAILURE;

    }

    
    ~memory_reorder_scheme(){};

    std::string output_type() const final override {

      return typeid(compressed_type).name();
    
    }

    bool is_compressor() const final override {
    
      return base_type::is_compressor;
    
    }




  };

}

#endif /* _MEMORY_REORDER_SCHEME_IMPL_H_ */
