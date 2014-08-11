#ifndef _HIST_IMPL_H_
#define _HIST_IMPL_H_
#include <climits>
#include <limits>
#include <vector>
#include <cmath>
#include <numeric>
#include "sqeazy_traits.hpp"

namespace sqeazy {

  template <typename IteratorT>
  IteratorT median_index(IteratorT begin, IteratorT end){
    
    //using double is a guess here
    double total_integral = std::accumulate(begin, end,0);

    double running_integral = 0;
    IteratorT median = end;

    for(;begin!=end;++begin) {
        running_integral += *begin;
        if((running_integral/total_integral) > .5) {
            median = begin - 1;
            break;
        }
    }

    return median;

  }
  
  
  
template <typename T, typename CounterT = unsigned long>
struct histogram {

    typedef typename sqeazy::twice_as_wide<CounterT>::type integral_type;

    static const unsigned long long num_bins = 1 << (sizeof(T) * CHAR_BIT);
    std::vector<CounterT> bins;
    CounterT  num_entries;

    //distribution info
    integral_type integral_value;
    T mean_value;
    T mean_variation_value;
    T median_value;
    T median_variation_value;
    T mode_value;
    T small_pop_bin_value;
    T large_pop_bin_value;


    histogram():
        bins(num_bins,0),
        num_entries(0),
        integral_value(0),
        mean_value(0),
        mean_variation_value(0),
        median_value(0),
        median_variation_value(0),
        mode_value(0),
        small_pop_bin_value(0),
        large_pop_bin_value(std::numeric_limits<T>::max())
    {

    }

    histogram(T* _image, const CounterT& _size):
        bins(num_bins,0),
        num_entries(_size),
        integral_value(0),
        mean_value(0),
        mean_variation_value(0),
        median_value(0),
        median_variation_value(0),
        mode_value(0),
        small_pop_bin_value(0),
        large_pop_bin_value(std::numeric_limits<T>::max())
    {

        fill_from_image(_image, _image + _size);

    }

    template <typename ItrT>
    histogram(ItrT _begin, ItrT _end):
        bins(num_bins,0),
        num_entries(_end - _begin),
        integral_value(0),
        mean_value(0),
        mean_variation_value(0),
        median_value(0),
        median_variation_value(0),
        mode_value(0),
        small_pop_bin_value(0),
        large_pop_bin_value(std::numeric_limits<T>::max())
    {

        fill_from_image(_begin, _end);

    }




    //TODO: use SFINAE to fill bins for signed histo as well
    template <typename ItrT>
    void fill_from_image(ItrT _image_begin, ItrT _image_end) {
        num_entries = _image_end - _image_begin;

        for(; _image_begin!=_image_end; ++_image_begin) {
            bins[*_image_begin]++;
        }

        small_pop_bin_value = calc_smallest_populated_bin();
        large_pop_bin_value = calc_largest_populated_bin();

        integral_value = calc_integral();
        mean_value = calc_mean();
	mean_variation_value = calc_mean_variation();
        median_value = calc_median();
        median_variation_value = calc_median_variation();
        mode_value = calc_mode();





    }

    integral_type calc_integral() const {

        integral_type value = std::accumulate(bins.begin(),bins.end(),0);

        return value;
    }

    integral_type integral() const {

        return integral_value;

    }

    CounterT entries() const {
        return num_entries;
    }

    T calc_mode() const {
        T value = std::max_element(bins.begin(),bins.end()) - bins.begin();
        return value;
    }

    T mode() const {
        return mode_value;
    }

    T calc_mean() const {
        integral_type value = 0;

        for(T i = small_pop_bin_value; i < (large_pop_bin_value+1); ++i) {
            value += i*(bins[i]);
        }

        value/=float(integral());

        return value;
    }

    T mean() const {
        return mean_value;
    }

    T calc_mean_variation() const {

        float mean_variation = 0;

        for(T i = smallest_populated_bin(); i<(largest_populated_bin()+1); ++i) {
            float temp = bins[i] - float(mean());
            mean_variation += (temp)*(temp);
        }

        mean_variation/=float(num_bins - 1);

        return std::sqrt(mean_variation);

    }
    
    T mean_variation() const { return mean_variation_value; }
    
    T calc_largest_populated_bin() const {

        T value = 0;

        for(T i = num_bins-1; i>=0; i-=1) {
            if(bins[i]) {
                value = i;
                break;
            }
        }

        return value;
    }

    T largest_populated_bin() const {

        return large_pop_bin_value;
    }


    T calc_smallest_populated_bin() const {

        T value = 0;

        for(T i = 0; i<num_bins; ++i) {
            if(bins[i]) {
                value = i;
                break;
            }
        }

        return value;
    }

    T smallest_populated_bin() const {

        return small_pop_bin_value;
    }


    T calc_median() const {

	
        return median_index(bins.begin()+smallest_populated_bin(),
	  bins.begin()+largest_populated_bin() +1
	) - bins.begin();

    }

    T calc_median_variation() const {

        float median_variation = 0;

        for(T i = smallest_populated_bin(); i<(largest_populated_bin()+1); ++i) {
            float temp = bins[i] - float(median());
            median_variation += (temp)*(temp);
        }

        median_variation/=float(num_bins - 1);

        return std::sqrt(median_variation);

    }

    
    T median() const {
        return median_value;
    }

    T median_variation() const { return median_variation_value; }
    
    ~histogram() {

    }
};

  template <typename T>
  typename T::value_type mpicbg_median_variation(T begin, T end){
    
    typedef typename T::value_type value_type;
    typedef std::vector<value_type> local_vector;
    
    histogram<value_type> h_original(begin, end);
    unsigned size = end - begin;
    const value_type median = h_original.median();
    
    typename local_vector::const_iterator intensity = begin;
    typename local_vector::const_iterator image_end = end;
    
    local_vector reduced(size);
    typename local_vector::iterator rbegin = reduced.begin();
    for(;intensity!=image_end;++intensity, ++rbegin){
      if(*intensity < median)
	*rbegin = 0;
      else
	*rbegin = *intensity - median;
      
    }
    
    histogram<value_type> h_reduced(reduced.begin(), reduced.end());
    
    return h_reduced.median();

  }

}//sqeazy

#endif /* _HIST_IMPL_H_ */
