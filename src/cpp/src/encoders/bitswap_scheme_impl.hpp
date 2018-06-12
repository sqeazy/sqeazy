#ifndef _BITSWAP_SCHEME_IMPL_H_
#define _BITSWAP_SCHEME_IMPL_H_

#include "compass.hpp"
#include "neighborhood_utils.hpp"
#include "sqeazy_common.hpp"
#include "traits.hpp"

#include "dynamic_stage.hpp"
#include "string_parsers.hpp"

#ifdef _OPENMP
#include "omp.h"
#endif


#if defined(__AVX__) || (defined(COMPASS_CT_HAS_SSE4) && COMPASS_CT_HAS_SSE4 > 0)
#include "bitplane_reorder_sse.hpp"
#endif

#include "bitplane_reorder_scalar.hpp"

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

        pipeline_parser p;
        auto config_map = p.minors(_payload.begin(), _payload.end());

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
    std::string config() const override {

      std::ostringstream msg;
      msg << "num_bits_per_plane=" << std::to_string(num_bits_per_plane);
      return msg.str();

    }

    std::intmax_t max_encoded_size(std::intmax_t _size_bytes) const override final {

      return _size_bytes;
    }

    compressed_type* encode( const raw_type* _input, compressed_type* _output, std::size_t _length) override final {

      std::size_t max_size = _length - (_length % num_planes);
//      std::size_t n_bits_per_element = sizeof(raw_type)*CHAR_BIT;

      if(max_size < _length)
        std::copy(_input+max_size,_input+_length,_output+max_size);

      int err = 0;
      if(sqeazy::platform::use_vectorisation::value &&
         compass::runtime::has(compass::feature::sse4()) &&
         num_bits_per_plane==1 &&
         sizeof(raw_type)>1 &&
         sqeazy::detail::sse_valid_length<static_num_bits_per_plane,raw_type>(_length)
		  )
      {

#ifdef _SQY_VERBOSE_
        std::cout << "[bitswap_scheme::encode]\tusing see method\n";
#endif
        err = sqeazy::detail::sse_bitplane_reorder_encode<static_num_bits_per_plane>(_input,
                                                                                     _output,
                                                                                     max_size,
                                                                                     this->n_threads());

      }
      else{
#ifdef _SQY_VERBOSE_
        std::cout << "[bitswap_scheme::encode]\tusing scalar method, why ? "
                  << "sqeazy::platform::use_vectorisation::value = "<< sqeazy::platform::use_vectorisation::value << ", "
                  << "compass::runtime::has(compass::feature::sse4()) = " << compass::runtime::has(compass::feature::sse4())<< ","
                  << "num_bits_per_plane==1 " << num_bits_per_plane << " ==1 ,"
                  << "sizeof(raw_type)=" << sizeof(raw_type) << ">1, "
                  << "sqeazy::detail::sse_valid_length<static_num_bits_per_plane,raw_type>(_length) " <<
                  sqeazy::detail::sse_valid_length << static_num_bits_per_plane,raw_type>(_length)
                  << "\n";
#endif
        err = sqeazy::detail::scalar_bitplane_reorder_encode<static_num_bits_per_plane>(_input,
                                                                                        _output,
                                                                                        max_size,
                                                                                        this->n_threads());
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
      return sqeazy::detail::scalar_bitplane_reorder_decode<static_num_bits_per_plane>(_input,
                                                                                       _output,
                                                                                       max_size);
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
        return sqeazy::detail::scalar_bitplane_reorder_decode<static_num_bits_per_plane>(_input,
                                                                                         _output,
                                                                                         max_size);
      }


    ~bitswap_scheme(){};

    std::string output_type() const final override {

      return typeid(compressed_type).name();

    }

    bool is_compressor() const final override {

      return base_type::is_compressor;

    }


  };

}

#endif /* _BITSWAP_SCHEME_IMPL_H_ */
