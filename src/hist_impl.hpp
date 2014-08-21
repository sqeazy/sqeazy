#ifndef _HIST_IMPL_H_
#define _HIST_IMPL_H_
#include <climits>
#include <limits>
#include <vector>
#include <cmath>
#include <numeric>
#include "sqeazy_traits.hpp"

namespace sqeazy {

template <typename T, typename FT>
T round(const FT& _num) {

    return std::floor(_num + .5);
}

template <typename IteratorT>
IteratorT median_index(IteratorT begin, IteratorT end) {

    //using double is a guess here
    double total_integral = std::accumulate(begin, end,0);

    double running_integral = 0;
    IteratorT median = end;
    IteratorT in_begin = begin;
    for(; begin!=end; ++begin) {
        running_integral += *begin;
        if((running_integral/total_integral) > .5) {
            median = begin - 1;
            break;
        }
    }

    if(median<in_begin) {
        median=in_begin;
    }

    return median;

}



template <typename T, typename CounterT = unsigned int >
struct histogram {

    typedef typename sqeazy::twice_as_wide<CounterT>::type integral_type;
    typedef T value_type;
    typedef CounterT bins_type;
    typedef typename sqeazy::twice_as_wide<T>::type twice_value_type;

    static const unsigned long long num_bins = 1 << (sizeof(T) * CHAR_BIT);
    std::vector<bins_type> bins;
    integral_type  num_entries;

    //distribution info
    integral_type integral_value;
    float mean_value;
    float mean_variation_value;
    float median_value;
    float median_variation_value;
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


    histogram(T* _image, const integral_type& _size):
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
        if(_size>0)
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
        if(num_entries)
            fill_from_image(_begin, _end);

    }




    //TODO: use SFINAE to fill bins for signed histo as well
    template <typename ItrT>
    void fill_from_image(ItrT _image_begin, ItrT _image_end) {
        num_entries = _image_end - _image_begin;
        /*
                if(!num_entries)
                    return;*/

        for(ItrT Itr = _image_begin; Itr!=_image_end; ++Itr) {
            bins[*Itr]++;
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

    float calc_mean() const {
        float value = 0;
        const twice_value_type end = large_pop_bin_value +1;
        float temp = 0;
        const float inv_integral = 1.f/integral();

        const CounterT* data = &bins[0];
        for(twice_value_type i = small_pop_bin_value; i < (end); ++i) {
            temp = data[i];
            value += i*temp;
        }

        return value*inv_integral;
    }

    float mean() const {
        return mean_value;
    }

    float calc_mean_variation() const {

        float mean_variation = 0;
        const twice_value_type end = large_pop_bin_value +1;
 
 
	unsigned mean_index = round<unsigned>(mean());
        float mean_value = 0;
        if(mean_index<num_bins) {
            mean_value = bins[mean_index];
        }
        
        for(twice_value_type i = smallest_populated_bin(); i<(end); ++i) {
            float temp = bins[i] - mean_value;
            mean_variation += (temp)*(temp);
        }

        mean_variation/=float(num_bins - 1);

        return std::sqrt(mean_variation);

    }

    float mean_variation() const {
        return mean_variation_value;
    }

    T calc_largest_populated_bin() const {

        T value = 0;

        for(twice_value_type i = num_bins-1; i>=0; i-=1) {
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

        for(twice_value_type i = 0; i<num_bins; ++i) {
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


    float calc_median() const {

        T mindex = median_index(bins.begin()+smallest_populated_bin(),
                                bins.begin()+largest_populated_bin() +1
                               ) - bins.begin();
        float result = 0;
        if(mindex>0 ) //the underlying distribution has always even number of entries
            result = (bins[mindex]*mindex + bins[mindex-1]*(mindex-1))/float(bins[mindex-1] + bins[mindex]);

        return result;

    }

    float calc_median_variation() const {

        float median_variation = 0;
        const twice_value_type end = large_pop_bin_value +1;
        unsigned median_index = round<unsigned>(median());
        float median_value = 0;
        if(median_index<num_bins) {
            median_value = bins[median_index];
        }

        for(twice_value_type i = smallest_populated_bin(); i<(end); ++i) {
            float temp = bins[i] - median_value;
            median_variation += (temp)*(temp);
        }

        median_variation/=float(num_bins - 1);

        return std::sqrt(median_variation);

    }


    float median() const {
        return median_value;
    }

    float median_variation() const {
        return median_variation_value;
    }

    ~histogram() {

    }
};

template <typename T>
typename T::value_type mpicbg_median_variation(T begin, T end) {

    typedef typename T::value_type value_type;
    typedef std::vector<value_type> local_vector;

    histogram<value_type> h_original(begin, end);
    unsigned size = end - begin;
    const value_type median = h_original.median();

    typename local_vector::const_iterator intensity = begin;
    typename local_vector::const_iterator image_end = end;

    local_vector reduced(size);
    typename local_vector::iterator rbegin = reduced.begin();
    for(; intensity!=image_end; ++intensity, ++rbegin) {
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
