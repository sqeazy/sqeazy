#ifndef _TILE_SHUFFLE_SCHEME_IMPL_H_
#define _TILE_SHUFFLE_SCHEME_IMPL_H_

#include <sstream>
#include <string>

#include "sqeazy_common.hpp"
#include "traits.hpp"
#include "dynamic_stage.hpp"
#include "string_parsers.hpp"

#include "tile_shuffle_utils.hpp"

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
  struct tile_shuffle_scheme : public filter<in_type> {
  
    typedef filter<in_type> base_type;
    typedef in_type raw_type;
    typedef in_type compressed_type;

    static const std::string description() { return std::string("reorder the tiles in the incoming stack based on some defined metric"); };
    static const std::size_t default_tile_size = 32;

    std::size_t tile_size;
    std::string serialized_reorder_map;
        
    tile_shuffle_scheme(const std::string& _payload=""):
      tile_size(default_tile_size)
    {
      
      auto config_map = parse_string_by(_payload);

      if(config_map.size()){
	
      	auto f_itr = config_map.find("tile_size");
      	if(f_itr!=config_map.end())
      	  tile_size = std::stoi(f_itr->second);

	f_itr = config_map.find("reorder_map");
      	if(f_itr!=config_map.end())
      	  serialized_reorder_map = std::stoi(f_itr->second);
	
      }
    }

    std::string name() const override final {

      std::ostringstream msg;
        msg << "tile_shuffle";

        return msg.str();
	
    }

    std::string config() const override final {

      std::ostringstream msg;
      msg << "tile_size=" << std::to_string(tile_size);
      msg << ",";
      msg << "reorder_map=" << serialized_reorder_map;
      return msg.str();
    
    }

    std::intmax_t max_encoded_size(std::intmax_t _size_bytes) const override final {
    
      return _size_bytes;
    }

    template <typename T>
    static void serialize_range(T _begin, T _end, std::string& _msg){

      const std::size_t len = _end - _begin;
      const std::size_t n_bytes = len*sizeof(*_begin);
      //      const std::size_t n_bytes_per_item = sizeof(*_begin);
      _msg.resize(n_bytes);

      auto dst = _msg.begin();
      const char* recasted = reinterpret_cast<const char*>(&*_begin);
      std::copy(recasted,recasted+n_bytes,dst);
      
      
    }

    template <typename T>
    static void serialize_container(const T& _container, std::string& _msg){

      const std::size_t n_bytes = _container.size()*sizeof(*_container.data());
      _msg.resize(n_bytes);

      auto dst = _msg.begin();
      const char* recasted = reinterpret_cast<const char*>(&*_container.data());
      std::copy(recasted,recasted+n_bytes,dst);
      
      
    }
    
    template <typename T>
    static void deserialize_range(const std::string& _msg, T _begin, T _end){

      const std::size_t len = _end - _begin;
      const std::size_t n_bytes = len*sizeof(*_begin);

      if(n_bytes!=_msg.size())
	return;

      char* recasted = reinterpret_cast<char*>(&*_begin);
      std::copy(_msg.cbegin(), _msg.cend(),recasted);
      
      
    }

    template <typename T>
    static void deserialize_container(const std::string& _msg, T& _container){

      const std::size_t n_bytes = _container.size()*sizeof(*_container.data());

      if(n_bytes!=_msg.size())
	_container.resize(_msg.size()/sizeof(*_container.data()));

      char* recasted = reinterpret_cast<char*>(&*_container.data());
      std::copy(_msg.cbegin(), _msg.cend(),recasted);
      
      
    }

    
    compressed_type* encode( const raw_type* _input,
			     compressed_type* _output,
			     const std::vector<std::size_t>& _shape) override final {

      typedef std::size_t size_type;
      unsigned long length = std::accumulate(_shape.begin(), _shape.end(), 1, std::multiplies<size_type>());

      detail::tile_shuffle tiles_of(tile_size);
      
      auto value = tiles_of.encode(_input, _input+length,
      				   _output,
      				   _shape);

      
      serialize_range(tiles_of.decode_map.begin(), tiles_of.decode_map.end(),serialized_reorder_map);
      
      return value;
    }

    int decode( const compressed_type* _input,
		raw_type* _output,
		const std::vector<std::size_t>& _ishape,
		std::vector<std::size_t> _oshape = std::vector<std::size_t>()) const override final {

      std::size_t length = std::accumulate(_ishape.begin(), _ishape.end(), 1, std::multiplies<std::size_t>());
      std::vector<std::size_t> decode_map;
      deserialize_container(serialized_reorder_map,decode_map);
      
      detail::tile_shuffle tiles_of(tile_size,decode_map);
      
      auto value = tiles_of.decode(_input, _input+length,
      				   _output,
      				   _ishape);

      if(value==(_output+length))
      	return SUCCESS;
      else
      return FAILURE;

    }

    
    ~tile_shuffle_scheme(){};

    std::string output_type() const final override {

      return typeid(compressed_type).name();
    
    }

    bool is_compressor() const final override {
    
      return base_type::is_compressor;
    
    }




  };

}

#endif /* _TILE_SHUFFLE_SCHEME_IMPL_H_ */
