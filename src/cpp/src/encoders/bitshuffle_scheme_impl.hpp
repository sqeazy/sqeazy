#ifndef _BITSHUFFLE_SCHEME_IMPL_H_
#define _BITSHUFFLE_SCHEME_IMPL_H_

#include "compass.hpp"
#include "neighborhood_utils.hpp"
#include "sqeazy_common.hpp"
#include "traits.hpp"

#include "dynamic_stage.hpp"
#include "string_parsers.hpp"

extern "C" {

  #include "bitshuffle_core.h"

}

// #ifdef _OPENMP
// #include "omp.h"
// #endif


// #if defined(__AVX__) || (defined(COMPASS_CT_HAS_SSE4) && COMPASS_CT_HAS_SSE4 > 0)
// #include "bitplane_reorder_sse.hpp"
// #endif

// #include "bitplane_reorder_scalar.hpp"

namespace sqeazy {

  //TODO: remove static_num_bits_per_plane as soon as deprecated API is gone
  template <typename in_type>
  struct bitshuffle_scheme : public filter<in_type> {

    typedef filter<in_type> base_type;
    typedef in_type raw_type;
    typedef in_type compressed_type;

    std::size_t block_size;

    static_assert(std::is_arithmetic<raw_type>::value==true,"[bitshuffle_scheme] input type is non-arithmetic");
    static const std::string description() { return std::string("rewrite bitplanes of item as chunks of buffer, use <block_size = 0> Bytes"); };

    //TODO: check syntax of lz4 configuration at runtime
    bitshuffle_scheme(const std::string& _payload=""):
      block_size(0)
      {

        pipeline_parser p;
        auto config_map = p.minors(_payload.begin(), _payload.end());

        if(config_map.size()){
          auto f_itr = config_map.find("block_size");
          std::size_t found = 0;
          if(f_itr!=config_map.end())
            found = std::stoi(f_itr->second);
        }


      }


    std::string name() const override final {

      std::ostringstream msg;
      msg << "bitshuffle";
      return msg.str();

    }


    /**
       \brief serialize the parameters of this filter

       \return 
       \retval string .. that encodes the configuration paramters

    */
    std::string config() const override {

      std::ostringstream msg;
      msg << "block_size=" << std::to_string(block_size);
      return msg.str();

    }

    std::intmax_t max_encoded_size(std::intmax_t _size_bytes) const override final {

      return _size_bytes;
    }

    compressed_type* encode( const raw_type* _input, compressed_type* _output, std::size_t _length) override final {

      std::size_t encoded = bshuf_bitshuffle_nthreads(_input, _output, _length,
                                                      sizeof(in_type),block_size, this->n_threads());
      if(encoded)
        return _output+(encoded);
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
      // size_type max_size = _length - (_length % num_planes);
      // if(max_size < _length)
      //   std::copy(_input+max_size,_input+_length,_output+max_size);
      std::size_t decoded = bshuf_bitunshuffle(_input, _output, _length, sizeof(in_type), block_size);

      if(decoded)
        return 0;
      else
        return 1;
    }

    int decode( const compressed_type* _input,
                raw_type* _output,
                std::size_t _length,
                std::size_t _olength = 0) const override final
      {

        if(!_olength)
          _olength = _length;

        // typedef typename sqeazy::twice_as_wide<std::size_t>::type size_type;
        // size_type max_size = _length - (_length % num_planes);
        // if(max_size < _length)
        //   std::copy(_input+max_size,_input+_length,_output+max_size);

        std::size_t decoded = bshuf_bitunshuffle(_input, _output, _length, sizeof(in_type), block_size);

        if(decoded)
          return 0;
        else
          return 1;
      }


    ~bitshuffle_scheme(){};

    std::string output_type() const final override {

      return sqeazy::header_utils::represent<compressed_type>::as_string();

    }

    bool is_compressor() const final override {

      return base_type::is_compressor;

    }


  };

}

#endif /* _BITSHUFFLE_SCHEME_IMPL_H_ */
