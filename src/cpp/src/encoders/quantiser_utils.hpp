#ifndef __QUANTISER_UTILS_HPP__
#define __QUANTISER_UTILS_HPP__

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



template<int base =  std::numeric_limits<uint16_t>::max()>
struct pow4Weighter{

  template <typename T>
  float operator()(const T& _value) const {
    return std::pow(_value/double(base),4);
  }
  
};

template<int exponent = 2>
struct power_law{

  static std::string name (){
    std::ostringstream msg;
    msg << "power_law" << exponent;
    return msg.str();
  }
  
  template <typename T, typename return_t = float>
  constexpr return_t operator()(const T& _value) const {
    return std::pow(_value,exponent);
  }
  
};


struct noWeighter{

  static std::string name (){
    std::ostringstream msg;
    msg << "no_weight";
    return msg.str();
  }
  
  template <typename T>
  constexpr float operator()(const T& _value) const {
    return 1.f;
  }
  
};


namespace sqeazy {

  template<typename raw_type, 
	   typename compressed_type>
  struct applyLUT{

    static const int max_size_from = std::numeric_limits<raw_type>::max() + 1;
  
    std::array<compressed_type, max_size_from> lut_;

    applyLUT(const std::array<compressed_type, max_size_from>& _lut):
      lut_(_lut){
    };

    compressed_type operator()(const raw_type& _value){
      return lut_[_value];
    }
  
  };

  //all is public for now
  template<typename raw_type,typename compressed_type, typename weight_functor_t = noWeighter>
  struct quantiser{


    static_assert(std::is_integral<raw_type>::value,"[quantiser] raw_type is not integral");
    static_assert(std::is_integral<compressed_type>::value,"[quantiser] compressed_type is not integral");
    static_assert(sizeof(raw_type) > sizeof(compressed_type),"[quantiser] raw_type is not wider than compressed_type");
    static_assert(sizeof(size_t) > sizeof(raw_type),"[quantiser] raw_type is not smaller than size_t");
    
    static const size_t max_raw_ =  std::numeric_limits<raw_type>::max() + 1 ; 
    static const size_t max_compressed_ = std::numeric_limits<compressed_type>::max() + 1; 

    static const raw_type max_value_raw_ =  std::numeric_limits<raw_type>::max() ; 
    static const compressed_type max_value_compressed_ = std::numeric_limits<compressed_type>::max(); 

    long sum_;
    std::array<uint32_t, max_raw_> histo_;
    std::array<float, max_raw_> weights_;
    std::array<float, max_raw_> importance_;
    std::array<compressed_type, max_raw_> lut_encode_;
    std::array<raw_type, max_compressed_> lut_decode_;

    
    quantiser(const raw_type* _begin = 0, const raw_type* _end = 0) :
      sum_(0),
      histo_(),
      weights_(),
      importance_(),
      lut_encode_(),
      lut_decode_()
    {
      
      histo_.fill(0.f);
      weights_.fill(1.f);
      importance_.fill(0.f);
      lut_encode_.fill(0);
      lut_decode_.fill(0);
      
      //this loop is fixed at compile time even!
      //could be potential candidate for unrolling
      weight_functor_t functor;
      for(uint32_t idx = 0;idx<quantiser::max_raw_;++idx){
	weights_[idx] = functor(idx);
      }

      setup(_begin, _end);
      
    };


    inline raw_type max_value() const {
      auto max_filled_itr = std::find_if(histo_.rbegin(),
					 histo_.rend(),
					 std::bind(std::not_equal_to<uint32_t>(),std::placeholders::_1,0)
					 );
      raw_type max_value = max_filled_itr.base() - 1 - histo_.begin();
      return max_value;
    }

    inline raw_type min_value() const {
      auto min_filled_itr = std::find_if(histo_.begin(),
					 histo_.end(),
					 std::bind(std::not_equal_to<uint32_t>(),std::placeholders::_1,0)
					 );
      raw_type min_value = min_filled_itr - histo_.begin();
      return min_value;
    }

    
    
    
    void computeHistogram(const raw_type* _data, const size_t& _nelems){
      computeHistogram(_data,_data + _nelems);
    }
    
    void computeHistogram(const raw_type* _begin, const raw_type* _end){
      std::fill(histo_.begin(),histo_.end(),0.);
      sum_ = 0;
    
      for(;_begin!=_end;++_begin){
	histo_[*_begin]++;
	sum_+=*_begin;
      }
    }

    void computeImportance(){
      std::fill(importance_.begin(),importance_.end(),0.);

      //loop fixed at compile time (unrolled?)
      for(uint32_t idx = 0;idx<quantiser::max_raw_;idx++){
	importance_[idx] = histo_[idx]*weights_[idx];
      }
    }

    void adaptive_lloyd_max(const float& _imp_sum){

      
      float importanceSum = 0;

      if(_imp_sum)
	importanceSum = _imp_sum;
      else
	importanceSum = std::accumulate(importance_.begin(),importance_.end(),0.);

      size_t levels_available = max_compressed_;//-1 one because we are assuming to use 0 already
      float bucketSize = importanceSum/levels_available;
      
      float importanceIntegral = importance_[0];
      float quantile_sum = importance_[0];
      uint32_t comp_idx = 0;
      float max_importance_in_bucket = importance_[0];
      float index_max_importance = 0;

      for(uint32_t raw_idx = 1;raw_idx<quantiser::max_raw_;++raw_idx){
	
	//bucket overflow 
	if(quantile_sum >= bucketSize && comp_idx<(max_compressed_-1)){

	  //FIXME: using std::array::at is expected to harm performance
	  lut_decode_.at(comp_idx) = static_cast<raw_type>(index_max_importance);
	  comp_idx++;
	  levels_available--;
	  	  
	  quantile_sum = importance_[raw_idx];//or 0?

	  if(importanceIntegral<importanceSum && levels_available>1)
	    bucketSize = (importanceSum-importanceIntegral)/levels_available;
	    
	  max_importance_in_bucket = importance_[raw_idx];
	  index_max_importance = raw_idx;

	} else {
	  quantile_sum += importance_[raw_idx];
	  
	  if(importance_[raw_idx]>max_importance_in_bucket){
	    max_importance_in_bucket = importance_[raw_idx];
	    index_max_importance = raw_idx;
	  }
	
	}

	lut_encode_[raw_idx] = static_cast<compressed_type>(comp_idx);
	importanceIntegral+=importance_[raw_idx];
	
      }

      //FIXME: using std::array::at is expected to harm performance
      lut_decode_.at(comp_idx) = static_cast<raw_type>(index_max_importance);
      

    }

    
    void adaptive_lloyd_com(const float& _imp_sum){

      
      float importanceSum = 0;

      if(_imp_sum)
	importanceSum = _imp_sum;
      else
	importanceSum = std::accumulate(importance_.begin(),importance_.end(),0.);

      size_t levels_available = max_compressed_;//-1 one because we are assuming to use 0 already
      float bucketSize = importanceSum/levels_available;
      
      float importanceIntegral = importance_[0];
      float quantile_sum = importance_[0];
      uint32_t comp_idx = 0;
      float weighted_mean_importance_in_bucket = 0*importance_[0];
      float index_weighted_mean_importance = 0;

      for(uint32_t raw_idx = 1;raw_idx<quantiser::max_raw_;++raw_idx){
	
	//bucket overflow 
	if(quantile_sum >= bucketSize && (comp_idx<lut_decode_.size()-1)){

	  //FIXME: using std::array::at is expected to harm performance
	  lut_decode_.at(comp_idx) = static_cast<raw_type>(index_weighted_mean_importance);
	  comp_idx++;
	  levels_available--;
	  
	  quantile_sum = importance_[raw_idx];//or 0?
	  weighted_mean_importance_in_bucket = raw_idx*importance_[raw_idx];
	  
	  if(importanceIntegral<importanceSum)
	    bucketSize = (importanceSum-importanceIntegral)/levels_available;
	  
	  if(quantile_sum!=0.)
	    index_weighted_mean_importance = weighted_mean_importance_in_bucket/quantile_sum;

	} else {
	  quantile_sum += importance_[raw_idx];
	  weighted_mean_importance_in_bucket += raw_idx*importance_[raw_idx];

	  if(quantile_sum!=0.)
	    index_weighted_mean_importance = weighted_mean_importance_in_bucket/quantile_sum;
	 	 
	
	}

	lut_encode_[raw_idx] = static_cast<compressed_type>(comp_idx);
	importanceIntegral+=importance_[raw_idx];
	
      }

      //FIXME: using std::array::at is expected to harm performance
      lut_decode_.at(comp_idx) = static_cast<raw_type>(index_weighted_mean_importance);
      

    }

    void linear_mapping_quantisation(){
      uint32_t comp_idx = 0;

      for(uint32_t raw_idx = 0;raw_idx < max_raw_ && comp_idx < lut_decode_.size();++raw_idx){

	lut_encode_[raw_idx] = static_cast<compressed_type>(comp_idx);
	lut_decode_[comp_idx] = static_cast<raw_type>(raw_idx);
	
	if(importance_[raw_idx])
	  comp_idx++;

      }

      
    }

    
    void computeLUT(){

      float importanceSum = std::accumulate(importance_.begin(),importance_.end(),0.);

      //nothing to do here
      if(!(importanceSum!=0))
	return;

      uint32_t n_levels = std::count_if(importance_.begin(), importance_.end(),
					std::bind(std::not_equal_to<uint32_t>(),std::placeholders::_1,0));
	
      if(n_levels <= max_compressed_)
      	linear_mapping_quantisation();
      else
      	// adaptive_lloyd_max(importanceSum);
	adaptive_lloyd_max(importanceSum);
      
    }

    void setup(const raw_type* _begin, const raw_type* _end){

      //safe-guard
      if(!_begin || !(_end - _begin))
	return;
    
      computeHistogram(_begin, _end);
      computeImportance();
      computeLUT();


    }

    void setup_com(const raw_type* _begin, const raw_type* _end){

      //safe-guard
      if(!_begin || !(_end - _begin))
	return;
    
      computeHistogram(_begin, _end);
      computeImportance();

      float importanceSum = std::accumulate(importance_.begin(),importance_.end(),0.);

      //nothing to do here
      if(!(importanceSum!=0))
	return;

      uint32_t n_levels = std::count_if(importance_.begin(), importance_.end(),
					std::bind(std::not_equal_to<uint32_t>(),std::placeholders::_1,0));
	
      if(n_levels <= max_compressed_)
      	linear_mapping_quantisation();
      else
      	// adaptive_lloyd_max(importanceSum);
	adaptive_lloyd_com(importanceSum);
      

    }

    
    void setup(const raw_type* _input, const size_t& _in_nelems){

      //safe-guard
      if(!_input || !_in_nelems)
	return;
    
      computeHistogram(_input, _input+ _in_nelems);
      computeImportance();
      computeLUT();


    }

    
    
    void encode(const raw_type* _input, const size_t& _in_nelems, 
		compressed_type* _output){

      //safe-guard
      if(!_input || !_in_nelems)
	return;
    
      setup(_input,_in_nelems);
      
      applyLUT<raw_type, compressed_type> lutApplyer(lut_encode_);
      std::transform(_input,_input+_in_nelems,_output,lutApplyer);
    
    }
  
    void decode(const compressed_type* _input, const size_t& _in_nelems, raw_type* _output){
    
      applyLUT<compressed_type, raw_type> lutApplyer(lut_decode_);
      std::transform(_input,_input+_in_nelems,_output,lutApplyer);
    
    }

    //could this be replaced by a stream operator overload?
    template <typename T>
    void lut_to_file(const std::string& _path, const T& _lut){

      std::fstream lutf(_path,std::ios::out|std::ios::trunc);

      for(auto& el : _lut)
	lutf << el << "\n";
      
      return;
    }

    //could this be replaced by a stream operator overload?
    void lut_from_file(const std::string& _path, std::array<raw_type, max_compressed_>& _lut){

      std::ifstream lutf(_path,std::ios::in);

      //TODO: safeguard received ill-defined luts!
      
      raw_type val;
      uint32_t counter = 0;
      while (lutf >> val)
	{
	  _lut[counter++] = val;
	}

    }
    
    /**
       \brief load decode table from path and decode the payload given by _input to produce _output
       
       \param[in] 
       
       \return 
       \retval 
       
    */
    void decode(const std::string& _path,
    		const compressed_type* _input,
    		const size_t& _in_nelems,
    		raw_type* _output){


      std::array<raw_type, max_compressed_> loaded_lut_decode_;
      lut_from_file(_path,loaded_lut_decode_);
      
      if(!std::accumulate(loaded_lut_decode_.begin(), loaded_lut_decode_.end(),0)){
	std::cerr << "lut from " << _path << " cannot be loaded, decoding skipped\n";
	return;
      }
      
      applyLUT<compressed_type, raw_type> lutApplyer(loaded_lut_decode_);
      std::transform(_input,_input+_in_nelems,_output,lutApplyer);
    
    }


  
    const std::array<int, quantiser::max_raw_>* get_histogram() const {
      return &histo_;
    
    }

    const std::array<float, quantiser::max_raw_>* get_weights() const {
      return &weights_;
    
    }
  
    const std::array<float, quantiser::max_raw_>* get_importance() const {
      return &importance_;
    
    }

    const std::array<raw_type, max_compressed_>* get_decode_lut() const {
      return &lut_decode_;
    }

    const std::array<compressed_type, max_raw_>* get_encode_lut() const {
      return &lut_encode_;
    }
        
    void dump(const std::string& _name) const {

      std::ofstream quant_log(_name, std::ios::out  | std::ios::trunc );

      quant_log << "[sqeazy::quantiser] \t"
		<< sizeof(raw_type)*CHAR_BIT << "bit "
		<< typeid(raw_type).name() << " -> "
		<< sizeof(compressed_type)*CHAR_BIT << "bit "
		<< typeid(compressed_type).name();

      float importanceSum = std::accumulate(importance_.begin(),importance_.end(),0.);
      float bucketSize = importanceSum/max_compressed_;
      quant_log << " (initial bucket size: " << bucketSize << ", sum(wei*int): "<< importanceSum<< ")\n";
      
      quant_log << std::setw(13) << "[begin,end]"
		<< std::setw(10) << "step size"
		<< std::setw(10) << "integral"
		<< std::setw(12) << "acc-weight"
		<< std::setw(14) << "acc-(wei*int)"
		<< std::setw(8 ) << "enc-as"
		<< std::setw(8 ) << "reco-as"
		<< "\n";
      
      int prec_ = quant_log.precision();
      quant_log.precision(4);
      uint32_t begin = 0;
      uint32_t end = begin + 1;

      uint64_t histo_sum=0;
      float weights_sum=0;
      float importance_sum=0;
      uint32_t last_level = lut_encode_[0];
      
      for( uint32_t item = 0;item < (max_raw_);++item){

	//entering a new level
	if(lut_encode_[item]>last_level || item == max_value_raw_){

	  end = item-1;
	  
	  if(item == max_value_raw_){
	    histo_sum += histo_[item];
	    weights_sum += weights_[item];
	    importance_sum += importance_[item];
	    end = item;
	  }
	  
	  quant_log << std::setw(6 ) << begin << "-"
		    << std::setw(6 ) << end
		    << std::setw(10) << (end - begin + 1)
		    << std::setw(10) << histo_sum
		    << std::setw(12) << weights_sum
		    << std::setw(14) << importance_sum
		    << std::setw(8 ) << (int)lut_encode_[end]
		    << std::setw(8 ) << (int)lut_decode_[lut_encode_[end]]
		    << "\n";
	  begin = item;
	  histo_sum=0;
	  weights_sum=0;
	  importance_sum=0;
	  last_level = lut_encode_[item];
	}

	histo_sum += histo_[item];
	weights_sum += weights_[item];
	importance_sum += importance_[item];	

	
      }

    

      quant_log << "\nencode_lut:\n";
      for( uint32_t item = 0;item < (max_raw_);++item)
	quant_log << std::setw(10) << item << std::setw(10) << (int)lut_encode_[item]
		  << std::setw(15) << importance_[item] << "\n";

      quant_log << "\ndecode_lut:\n";
      for( uint32_t item = 0;item < (max_compressed_);++item)
	quant_log << std::setw(10) << item << std::setw(10) << (int)lut_decode_[item] << "\n";
      
      quant_log.precision(prec_);
      
    }



    
  };

};
#endif
