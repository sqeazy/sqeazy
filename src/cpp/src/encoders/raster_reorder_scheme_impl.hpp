#ifndef _RASTER_REORDER_SCHEME_IMPL_H_
#define _RASTER_REORDER_SCHEME_IMPL_H_

#include <sstream>
#include <string>

#include "sqeazy_common.hpp"
#include "traits.hpp"
#include "dynamic_stage.hpp"
#include "string_parsers.hpp"

#include "raster_reorder_utils.hpp"

namespace sqeazy {

  template <typename in_type>
  struct raster_reorder_scheme : public filter<in_type> {

    typedef filter<in_type> base_type;
    typedef in_type raw_type;
    typedef in_type compressed_type;


    static const std::size_t default_tile_size = 16/sizeof(in_type);
    static const std::string description() {
      std::ostringstream msg;
      msg << "reorder the memory layout of the incoming buffer by linearizing virtual tiles, ";
      msg << "control the tile size by tile_size=<integer|default: " << default_tile_size << ">";
      return msg.str();
    };

    std::size_t tile_size;


    raster_reorder_scheme(const std::string& _payload=""):
      tile_size(default_tile_size)
      {

        pipeline_parser p;
        auto config_map = p.minors(_payload.begin(),_payload.end());

        if(config_map.size()){

          auto f_itr = config_map.find("tile_size");
          if(f_itr!=config_map.end())
            tile_size = std::stoi(f_itr->second);

        }
      }

    std::string name() const override final {

      std::ostringstream msg;
      msg << "raster_reorder";

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

    /**
     *  \brief raster reordering of 3D memory segment
     *
     *  given a 3D array in memory, say of length n=64 in all dimensions
     *  raster reordering partitions this 3D index space into tiles of size <tile_size> per dimension;
     *  then the 3D original array is looped through, each tile is extracted and appended in linear fashion to the output
     *
     *  example in 2D: 8x8 input with tile_size =4
     input
     {
     0 , 1 ,2 ,3 ,4 ,5 ,6 ,7 ,
     8 ,9 ,10 ,11 ,12 ,13 ,14 ,15 ,
     16 ,17 ,18 ,19 ,20 ,21 ,22 ,23 ,
     24 ,25 ,26 ,27 ,28 ,29 ,30 ,31 ,
     32 ,33 ,34 ,35 ,36 ,37 ,38 ,39 ,
     40 ,41 ,42 ,43 ,44 ,45 ,46 ,47 ,
     48 ,49 ,50 ,51 ,52 ,53 ,54 ,55 ,
     56 ,57 ,58 ,59 ,60 ,61 ,62 ,63
     }

     output
     {
     0,1,2,3,8,9,10,11,         #tile 0
     16,17,18,19,24,25,26,27,   #tile 0
     4,5,6,7,12,13,14,15,       #tile 1
     20,21,22,23,28,29,30,31,   #tile 1
     ...
     }

     *  \param param
     *  \return return type
     */
    compressed_type* encode( const raw_type* _input,
                             compressed_type* _output,
                             const std::vector<std::size_t>& _shape) override final {

      typedef std::size_t size_type;
      unsigned long length = std::accumulate(_shape.begin(), _shape.end(), 1, std::multiplies<size_type>());

      detail::reorder tiles_of(tile_size);

      auto value = tiles_of.encode(_input, _input+length,
                                   _output,
                                   _shape,
                                   this->n_threads()
        );

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
                                   _ishape,
                                   this->n_threads()
        );

      if(value==(_output+length))
        return SUCCESS;
      else
        return FAILURE;

    }


    ~raster_reorder_scheme(){};

    std::string output_type() const final override {

      return typeid(compressed_type).name();

    }

    bool is_compressor() const final override {

      return base_type::is_compressor;

    }




  };

}

#endif /* _RASTER_REORDER_SCHEME_IMPL_H_ */
