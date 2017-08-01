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


  template <typename in_type>
  struct tile_shuffle_scheme : public filter<in_type> {

    typedef filter<in_type> base_type;
    typedef in_type raw_type;
    typedef in_type compressed_type;

    static const std::string description() { return std::string("reorder the tiles in the incoming stack based on some defined metric; use tile_size=<integer|default: 32> to configure the shape of the tile to extract"); };
    static const std::size_t default_tile_size = 32;

    std::size_t tile_size;
    std::string serialized_reorder_map;

    tile_shuffle_scheme(const std::string& _payload=""):
      tile_size(default_tile_size)
    {

      pipeline_parser p;auto config_map = p.minors(_payload.begin(),_payload.end());

      if(config_map.size()){

      	auto f_itr = config_map.find("tile_size");
      	if(f_itr!=config_map.end())
      	  tile_size = std::stoi(f_itr->second);

	f_itr = config_map.find("reorder_map");
      	if(f_itr!=config_map.end())
      	  serialized_reorder_map = f_itr->second;

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


    compressed_type* encode( const raw_type* _input,
                             compressed_type* _output,
                             const std::vector<std::size_t>& _shape) override final {

      typedef std::size_t size_type;
      unsigned long length = std::accumulate(_shape.begin(), _shape.end(), 1, std::multiplies<size_type>());

      detail::tile_shuffle tiles_of(tile_size);

      auto value = tiles_of.encode(_input, _input+length,
                                   _output,
                                   _shape,
                                   this->n_threads());


      serialized_reorder_map = parsing::range_to_verbatim(tiles_of.decode_map.begin(), tiles_of.decode_map.end());


      return value;
    }

    int decode( const compressed_type* _input,
		raw_type* _output,
		const std::vector<std::size_t>& _ishape,
                std::vector<std::size_t> _oshape = std::vector<std::size_t>()) const override final {

      std::size_t length = std::accumulate(_ishape.begin(), _ishape.end(), 1, std::multiplies<std::size_t>());

      std::vector<std::size_t> decode_map(parsing::verbatim_yields_n_items_of<std::size_t>(serialized_reorder_map),0);

      // auto res =
      parsing::verbatim_to_range(serialized_reorder_map,
                                 decode_map.begin(), decode_map.end());

      detail::tile_shuffle tiles_of(tile_size,decode_map);

      auto value = tiles_of.decode(_input, _input+length,
                                   _output,
                                   _ishape,
                                   this->n_threads());

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
