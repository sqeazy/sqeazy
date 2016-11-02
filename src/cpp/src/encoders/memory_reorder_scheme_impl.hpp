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

    memory_reorder_scheme(){
    }

    
    memory_reorder_scheme(const std::string& _payload=""){

      // auto config_map = parse_string_by(_payload);

      // if(config_map.size()){
      // 	auto f_itr = config_map.find("fraction");
      // 	if(f_itr!=config_map.end())
      // 	  fraction = std::stof(f_itr->second);

      // 	f_itr = config_map.find("threshold");
      // 	if(f_itr!=config_map.end())
      // 	  threshold = std::stoi(f_itr->second);
      // }
    }

    std::string name() const override final {

      std::ostringstream msg;
        msg << "memory_reorder"
            // << Neighborhood::x_offset_end - Neighborhood::x_offset_begin << "x"
            // << Neighborhood::y_offset_end - Neighborhood::y_offset_begin << "x"
            // << Neighborhood::z_offset_end - Neighborhood::z_offset_begin 
	  ;

        return msg.str();
	
    }

    std::string config() const override final {

      // std::ostringstream msg;
      // msg << "threshold=" << std::to_string(threshold) << ",";
      // msg << "fraction=" << std::to_string(fraction);
      return "";
    
    }

    std::intmax_t max_encoded_size(std::intmax_t _size_bytes) const override final {
    
      return _size_bytes;
    }

    compressed_type* encode( const raw_type* _input,
			     compressed_type* _output,
			     const std::vector<std::size_t>& _shape) override final {

      typedef std::size_t size_type;
      unsigned long length = std::accumulate(_shape.begin(), _shape.end(), 1, std::multiplies<size_type>());

      
      return _output+length;
      
    }

    int decode( const compressed_type* _input, raw_type* _output,
		const std::vector<std::size_t>& _shape,
		std::vector<std::size_t>) const override final {

      std::size_t total_size = std::accumulate(_shape.begin(), _shape.end(), 1, std::multiplies<std::size_t>());
      return 0;

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
