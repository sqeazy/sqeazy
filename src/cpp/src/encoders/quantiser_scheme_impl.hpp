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
	   typename out_type = char,
	   typename weight_functor_t = noWeighter>
  struct quantiser_scheme : public sink<in_type,out_type> {

    typedef sink<in_type,out_type> base_type;
    typedef in_type raw_type;
    typedef typename base_type::out_type compressed_type;

    std::string quantiser_config;
    parsed_map_t  config_map;

    quantiser<raw_type, compressed_type, weight_functor_t> shrinker;

    quantiser_scheme(const std::string& _payload = ""):
      quantiser_config(_payload),
      config_map(),
      shrinker(){

      if(_payload.size())
	config_map = parse_string_by(_payload);

      auto fitr = config_map.find("decode_lut_path");
      if(fitr!=config_map.end())
	shrinker.lut_from_file(fitr->second,shrinker.lut_decode_);
      else{
	fitr = config_map.find("decode_lut_string");
	if(fitr!=config_map.end()){
	  shrinker.lut_from_string(fitr->second,shrinker.lut_decode_);
	}
      }
    }

    ~quantiser_scheme() override final {}

    std::string name() const override final {

      return std::string("quantiser");
    }

    std::string config() const override final {

      std::ostringstream msg;

      auto begin = config_map.begin();
      auto end = config_map.end();

      size_t count = 0;
      for (;begin!=end;++begin){
	msg << begin->first << "=" << begin->second;
	if((count++)<(config_map.size()-1))
	  msg << ",";
      }
      
      return msg.str();
      
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

      auto fitr = config_map.find("decode_lut_path");
      if(fitr!=config_map.end())
	shrinker.lut_to_file(fitr->second,shrinker.lut_decode_);
      else
	config_map["decode_lut_string"] = shrinker.lut_to_string(shrinker.lut_decode_);
      
      applyLUT<raw_type, compressed_type> lutApplyer(shrinker.lut_encode_);
      auto out_end = std::transform(_in,_in+_length,_out,lutApplyer);
            
      return out_end;
    }


    int decode( const compressed_type* _in,
		raw_type* _out,
		std::vector<std::size_t> _inshape,
		std::vector<std::size_t> _outshape = std::vector<std::size_t>()
		) const override final {

      if(_outshape.empty())
	_outshape = _inshape;
      
      std::size_t inlength = std::accumulate(_inshape.begin(), _inshape.end(),1,std::multiplies<std::size_t>());
      std::size_t outlength = std::accumulate(_outshape.begin(), _outshape.end(),1,std::multiplies<std::size_t>());

      return decode(_in,_out,inlength,outlength);
      
    }

    int decode( const compressed_type* _in, raw_type* _out,
		std::size_t _inlength,
		std::size_t _outlength = 0) const override final {

      
      if(!_outlength)
	_outlength = _inlength;
      
      applyLUT<compressed_type, raw_type> lutApplyer(shrinker.lut_decode_);
      auto end_ptr = std::transform(_in,_in+_inlength,_out,lutApplyer);
      
      return (end_ptr - _out) - _inlength;
    }

    
    void dump(const std::string& _fname){
      
      shrinker.dump(_fname);
      
    }
  };

};
#endif
