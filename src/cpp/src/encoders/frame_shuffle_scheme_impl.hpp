#ifndef _TILE_SHUFFLE_SCHEME_IMPL_H_
#define _TILE_SHUFFLE_SCHEME_IMPL_H_

#include <sstream>
#include <string>

#include "sqeazy_common.hpp"
#include "traits.hpp"
#include "dynamic_stage.hpp"
#include "string_parsers.hpp"

#include "frame_shuffle_utils.hpp"

namespace sqeazy {


  template <typename in_type>
  struct frame_shuffle_scheme : public filter<in_type> {

    typedef filter<in_type> base_type;
    typedef in_type raw_type;
    typedef in_type compressed_type;

    static const std::string description() { return std::string("reorder the frames in the incoming stack based on some defined metric"); };
    static const std::size_t default_frame_chunk_size = 1;

    std::size_t frame_chunk_size;
    std::string serialized_reorder_map;

    frame_shuffle_scheme(const std::string& _payload=""):
      frame_chunk_size(default_frame_chunk_size)
    {

      pipeline_parser p;auto config_map = p.minors(_payload.begin(),_payload.end());

      if(config_map.size()){

        auto f_itr = config_map.find("frame_chunk_size");
      	if(f_itr!=config_map.end())
          frame_chunk_size = std::stoi(f_itr->second);

	f_itr = config_map.find("reorder_map");
      	if(f_itr!=config_map.end())
      	  serialized_reorder_map = f_itr->second;

      }
    }

    std::string name() const override final {

      std::ostringstream msg;
        msg << "frame_shuffle";

        return msg.str();

    }

    std::string config() const override final {

      std::ostringstream msg;
      msg << "frame_chunk_size=" << std::to_string(frame_chunk_size);
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

      detail::frame_shuffle frames_of(frame_chunk_size);

      auto value = frames_of.encode(_input, _input+length,
      				   _output,
      				   _shape);


      serialized_reorder_map = parsing::range_to_verbatim(frames_of.decode_map.begin(), frames_of.decode_map.end());


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

      detail::frame_shuffle frames_of(frame_chunk_size,decode_map);

      auto value = frames_of.decode(_input, _input+length,
                                   _output,
                                   _ishape);

      if(value==(_output+length))
      	return SUCCESS;
      else
        return FAILURE;

    }


    ~frame_shuffle_scheme(){};

    std::string output_type() const final override {

      return typeid(compressed_type).name();

    }

    bool is_compressor() const final override {

      return base_type::is_compressor;

    }




  };

}

#endif /* _TILE_SHUFFLE_SCHEME_IMPL_H_ */
