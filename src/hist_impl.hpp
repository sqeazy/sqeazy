#ifndef _HIST_IMPL_H_
#define _HIST_IMPL_H_
#include <climits>
#include <vector>
#include <set>
#include "sqeazy_traits.hpp" 

namespace sqeazy {

  template <typename T, typename CounterT = unsigned long>
  struct histogram {
    static const unsigned long long num_bins = 1 << (sizeof(T) * CHAR_BIT);
    std::vector<CounterT> bins;
    CounterT  num_entries;
    
    histogram():
      bins(num_bins,0),
      num_entries(0)
    {
      
    }

    histogram(T* _image, const CounterT& _size):
      bins(num_bins,0),
      num_entries(_size)
    {
      
      fill_from_image(_image, _image + _size);
      
    }

    template <typename ItrT>
    histogram(ItrT _begin, ItrT _end):
      bins(num_bins,0),
      num_entries(_end - _begin)
    {
      
      fill_from_image(_begin, _end);
      
    }


    //TODO: use SFINAE to fill bins for signed histo as well
    template <typename ItrT>
    void fill_from_image(ItrT _image_begin, ItrT _image_end){
      num_entries = _image_end - _image_begin;

      for(;_image_begin!=_image_end;++_image_begin){
	bins[*_image_begin]++;
      }
    }

    typename sqeazy::twice_as_wide<CounterT>::type integral() const {
      
      typedef typename sqeazy::twice_as_wide<CounterT>::type rtype;

      rtype value = std::accumulate(bins.begin(),bins.end(),0);

      return value;
    }

    CounterT entries() const {
      return num_entries;
    }

    T mode() const {
      T value = std::max_element(bins.begin(),bins.end()) - bins.begin();
      return value;
    }

    T mean() const {
      typedef typename sqeazy::twice_as_wide<T>::type double_wide_type;
      
      double_wide_type value = 0;
      
      typename std::vector<CounterT>::const_iterator begin = bins.begin();
      typename std::vector<CounterT>::const_iterator end = bins.end();
      
      for(;begin!=end;++begin){
	double_wide_type distance = begin - bins.begin();
	value += distance*(*begin);
      }

      value/=integral();

      return value;
    }
    
    T largest_populated_bin() const {
      
      T value = 0;

      for(T i = num_bins-1;i>=0;i-=1){
	if(bins[i]){
	  value = i;
	  break;
	}
      }
      
      return value;
    }

    T smallest_populated_bin() const {
      
      T value = 0;

      for(T i = 0;i<num_bins;++i){
	if(bins[i]){
	  value = i;
	  break;
	}
      }
      
      return value;
    }


    ~histogram(){
     
    }
  };

}//sqeazy

#endif /* _HIST_IMPL_H_ */
