#ifndef _SQEAZY_ALGORITHMS_H_
#define _SQEAZY_ALGORITHMS_H_

#include <iterator>

namespace sqeazy {
  
  template <typename element_t>
  int lowest_set_bit(element_t _data, element_t _data_end){

    int value = sizeof(element_t)*CHAR_BIT - 1;
    for(;_data!=_data_end;++_data){

      int lowest = 0;
      for(;lowest<value;++lowest)
	{
	  if((*_data >> lowest) & 1)
	    value = lowest;
	}

      if(lowest<value)
	value = lowest;
   
      if(value == 0)
	break;
    }  
  
    return value;
  }

  template <typename element_t>
  int highest_set_bit(element_t _data, element_t _data_end){
    
    auto max = *std::max_element(_data, _data_end);
    float value = 0;

    if(max)
      value = std::log(max)/std::log(2);
    
    return std::ceil(value);
  }

  template <typename it_type, typename it2_type>
  static inline double l2norm(it_type _lbegin, it_type _lend,
			      it2_type _rbegin){

    double dsum = 0;
    size_t size = _lend - _lbegin;

    for(;_lbegin!=_lend;++_lbegin,++_rbegin){
      double diff = *_lbegin - *_rbegin;
      dsum += diff*diff;
    }

    double value = std::sqrt(dsum)/size;

    return value;
  }
  
  template <typename it_type, typename it2_type>
  static inline double mse(it_type _lbegin, it_type _lend,
			   it2_type _rbegin){

      typedef decltype(*_lbegin) item_t;
      typedef decltype(*_rbegin) item2_t;

      static_assert(sizeof(item_t)==sizeof(item2_t),"[sqeazy::mse] types of different width received");
    
      size_t left_size = _lend - _lbegin;
      
      std::vector<float> diff(_lend - _lbegin,0);
      std::transform(_lbegin,_lend,
		     _rbegin,
		     diff.begin(),
		     std::minus<float>());

      double mse = std::inner_product(diff.begin(), diff.end(),
						diff.begin(),
						0.);
      mse /= left_size;

      return std::sqrt(mse);
    }

  //NRMSE = sqrt{1/N sum (I-I')^2}/[max(I) - min(I)]
  template <typename it_type, typename it2_type>
  static inline double nrmse(it_type _lbegin, it_type _lend,
			     it2_type _rbegin){

      typedef decltype(*_lbegin) item_t;
      typedef decltype(*_rbegin) item2_t;

      static_assert(sizeof(item_t)==sizeof(item2_t),"[sqeazy::nrmse] types of different width received");
    
      size_t left_size = _lend - _lbegin;
      
      std::vector<float> diff(_lend - _lbegin,0);
      std::transform(_lbegin,_lend,
		     _rbegin,
		     diff.begin(),
		     std::minus<float>());

      double rnmse = std::sqrt(std::inner_product(diff.begin(), diff.end(),
						diff.begin(),
						0.));
      rnmse /= (left_size);

      item_t max_value = *std::max_element(_lbegin, _lend);
      item_t min_value = *std::min_element(_lbegin, _lend);
      
      rnmse = std::sqrt(rnmse)/std::abs(max_value - min_value);
      
      return rnmse;
    }
  
  template <typename it_type, typename it2_type>
  static inline double psnr(it_type _lbegin, it_type _lend,
	      it2_type _rbegin){

    
    typedef decltype(*_lbegin) item_t;
    typedef decltype(*_rbegin) item2_t;

    static_assert(sizeof(item_t)==sizeof(item2_t),"[sqeazy::psnr] types of different width received");
    
    double mse = sqeazy::mse(_lbegin, _lend, _rbegin);

    const size_t max = ~0;
    const double offset = 20*std::log((item_t)max);
    double psnr = offset - 10*std::log(mse);

    return psnr;
  }

  template <typename iter_t>
  size_t dyn_range(iter_t _begin, iter_t _end){

    typedef typename std::iterator_traits<iter_t>::value_type value_t;
    static_assert(std::is_integral<value_t>::value,"[sqeazy::dyn_range] only works with integer types");
    
    
    auto max = *std::max_element(_begin, _end);
    auto min = *std::min_element(_begin, _end);

    size_t value = (max-min) +1;
    
    return value;
  }
  
};

#endif /* _SQEAZY_ALGORITHMS_H_ */
