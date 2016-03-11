#ifndef __QUANTISER_SCHEME_IMPL_HPP__
#define __QUANTISER_SCHEME_IMPL_HPP__

#include <map>
#include <cmath>
#include <algorithm>
#include <functional>
#include <numeric>
#include <limits>
#include <array>
#include <cstdint>
#include <typeinfo>
#include <type_traits>
#include <fstream>
#include <iomanip>

#include "traits.hpp"
#include "string_parsers.hpp"
#include "dynamic_stage.hpp"
#include "quantiser_utils.hpp"


namespace sqeazy {


  //all is public for now
  template<typename in_type,
	   typename out_type,
	   typename weight_functor_t = noWeighter>
  struct quantiser_scheme : public sink<in_type,out_type> {

    typedef sink<in_type,out_type> base_type;
    typedef in_type raw_type;
    typedef typename base_type::out_type compressed_type;

    std::string quantiser_config;
    quantiser<raw_type, compressed_type, weight_functor_t> shrinker;

    quantiser_scheme(const std::string& _payload = ""):
      quantiser_config(),
      shrinker(){
    }

    ~quantiser_scheme() override final {}

    std::string name() const override final {

      return std::string("quantiser");
    }

    std::string config() const override final {
      return quantiser_config;
    }

    /**
       \brief expected size of encoded buffer of size _size_bytes (the LUT etc are not included)
       
       \param[in] _size_bytes incoming buffer size in Bytes 
       
       \return 
       \retval 
       
    */
    std::intmax_t max_encoded_size(std::intmax_t _size_bytes) const override final {
    
      return _size_bytes*sizeof(raw_type)/sizeof(compressed_type);
      
    }

    std::string output_type() const final override {

      return typeid(compressed_type).name();
    
    }

    bool is_compressor() const final override {
    
      return base_type::is_compressor;
    
    }

    /**
     * @brief encode input raw_type buffer and write to output (not owned, not allocated)
     *
     * @param _input input raw_type buffer
     * @param _output output char buffer (not owned, not allocated)
     * @param _shape std::vector<size_type>, contains the shape of _input at [0] and the number of written bytes at [1]
     * @return sqeazy::error_code
     */
    compressed_type* encode( const raw_type* _in, compressed_type* _out, std::vector<std::size_t> _shape) override final {

      std::intmax_t length = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());

      return encode(_in,_out,length);
    }

    /**
     * @brief encode input raw_type buffer and write to output (not owned, not allocated)
     *
     * @param _input input raw_type buffer
     * @param _output output char buffer (not owned, not allocated)
     * @param _length mutable std::vector<size_type>, contains the shape of _input at [0] and the number of written bytes at [1]
     * @return sqeazy::error_code
     */
    compressed_type* encode( const raw_type* _in, compressed_type* _out, std::size_t _length) override final {

      //safe-guard
      if(!_in || !_length)
	return _out;

      const raw_type* in_begin = _in;
      const raw_type* in_end = _in + _length;
      shrinker.setup_com(in_begin, in_end);
      
      applyLUT<raw_type, compressed_type> lutApplyer(shrinker.lut_encode_);
      auto out_end = std::transform(_in,_in+_length,_out,lutApplyer);
            
      return out_end;
    }


    int decode( const compressed_type* _in, raw_type* _out, std::vector<std::size_t> _shape) const override final {

      std::size_t length = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());

      return decode(_in,_out,length);
      
    }

    int decode( const compressed_type* _in, raw_type* _out, std::size_t _length) const override final {

      applyLUT<compressed_type, raw_type> lutApplyer(shrinker.lut_decode_);
      auto end_ptr = std::transform(_in,_in+_length,_out,lutApplyer);
      
      return (end_ptr - _out) - _length;
    }

    
    void dump(const std::string& _fname){
      
      shrinker.dump(_fname);
      
    }
  };

};
#endif
