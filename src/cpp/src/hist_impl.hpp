#ifndef _HIST_IMPL_H_
#define _HIST_IMPL_H_
#include <climits>
#include <limits>
#include <climits>
#include <vector>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <numeric>
#include <algorithm>
#include <functional>

#include "traits.hpp"
#include "sqeazy_common.hpp"
#include "encoders/histogram_utils.hpp"

namespace sqeazy {

    template <typename T, typename FT>
    T my_round(const FT& _num) {

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
                median = begin /*- 1*/;
                break;
            }
        }

        if(median<in_begin) {
            median=in_begin;
        }

        return median;

    }

    /**
       \brief calcuate the bin index of the histogram where the normalised integral is greater than the given threshold

       \param[in] begin starting iterator of the array to calculate the support of
       \param[in] end end+1 iterator of the array to calculate the support of
       \param[in] threshold

       \return iterator pointing to the index in question
       \retval

    */
    template <typename IteratorT,typename ValueT>
    IteratorT support_index(IteratorT begin, IteratorT end, ValueT threshold) {

//using double is a guess here
        double total_integral = std::accumulate(begin, end,0);

        double running_integral = 0;
        IteratorT support = end;
        IteratorT in_begin = begin;
        for(; begin!=end; ++begin) {
            running_integral += *begin;
            if((running_integral/total_integral) > threshold) {
                support = begin /*- 1*/;
                break;
            }
        }

        if(support<in_begin) {
            support=in_begin;
        }

        return support;
    }




    template <typename T, typename CounterT = unsigned int >
    struct histogram {

        typedef typename sqeazy::twice_as_wide<CounterT>::type integral_type;
        typedef T value_type;
        typedef CounterT bins_type;
        typedef typename sqeazy::twice_as_wide<T>::type twice_value_type;

        static const unsigned long long num_bins = 1 << (sizeof(T) * CHAR_BIT);
        std::vector<bins_type> bins;
        typedef typename std::vector<bins_type>::iterator bins_iter_t;
        typedef typename std::vector<bins_type>::const_iterator bins_citer_t;

        integral_type  num_entries;

        //distribution info
        integral_type integral_value;
        float mean_value;
        float mean_variation_value;
        float median_value;
        float median_variation_value;
        float entropy_value;
        float support_value;

        T mode_value;
        T small_pop_bin_value;
        T large_pop_bin_value;

        std::uint32_t nthreads;

        histogram():
            bins(num_bins,0),
            num_entries(0),
            integral_value(0),
            mean_value(0),
            mean_variation_value(0),
            median_value(0),
            median_variation_value(0),
            entropy_value(0),
            support_value(0),
            mode_value(0),
            small_pop_bin_value(0),
#ifdef _WIN32
			large_pop_bin_value((std::numeric_limits<T>::max)()),
#else
            large_pop_bin_value(std::numeric_limits<T>::max()),
#endif
            nthreads(1)
            {

            }


        histogram(T* _image, const integral_type& _size, const std::uint32_t& _nthreads = 1):
            bins(num_bins,0),
            num_entries(_size),
            integral_value(0),
            mean_value(0),
            mean_variation_value(0),
            median_value(0),
            median_variation_value(0),
            entropy_value(0),
            support_value(0),
            mode_value(0),
            small_pop_bin_value(0),
            large_pop_bin_value((std::numeric_limits<T>::max)()),
            nthreads(_nthreads)
            {
                if(_size>0)
                    fill_from_image(_image, _image + _size);

            }

        template <typename ItrT>
        histogram(ItrT _begin, ItrT _end, const std::uint32_t& _nthreads = 1):
            bins(num_bins,0),
            num_entries(_end - _begin),
            integral_value(0),
            mean_value(0),
            mean_variation_value(0),
            median_value(0),
            median_variation_value(0),
            entropy_value(0),
            support_value(0),
            mode_value(0),
            small_pop_bin_value(0),
#ifdef _WIN32
            large_pop_bin_value((std::numeric_limits<T>::max)()),
#else
			large_pop_bin_value(std::numeric_limits<T>::max()),
#endif
            nthreads(_nthreads)
            {
                if(num_entries)
                    fill_from_image(_begin, _end);

            }


        void clear(){

            num_entries= (0);
            std::fill(bins.begin(), bins.end(),0);
            integral_value= (0);
            mean_value= (0);
            mean_variation_value= (0);
            median_value= (0);
            median_variation_value= (0);
            entropy_value= (0);
            support_value= (0);
            mode_value= (0);
            small_pop_bin_value= (0);
#ifdef _WIN32
			large_pop_bin_value = ((std::numeric_limits<T>::max)());
#else
            large_pop_bin_value= (std::numeric_limits<T>::max());
#endif
        }

        template <typename ItrT>
        void add_from_image(ItrT _image_begin, ItrT _image_end) {
            num_entries += _image_end - _image_begin;

            sqeazy::detail::parallel::fill_histogram(_image_begin, _image_end, bins.data(), nthreads);

        }

        void fill_stats(){
            small_pop_bin_value = calc_smallest_populated_bin();
            large_pop_bin_value = calc_largest_populated_bin();

            integral_value = calc_integral();
            mean_value = calc_mean();
            mean_variation_value = calc_mean_variation();
            median_value = calc_median();
            median_variation_value = calc_median_variation();
            mode_value = calc_mode();
            entropy_value = calc_entropy();


        }


        //TODO: use SFINAE to fill bins for signed histo as well
        template <typename ItrT>
        void fill_from_image(ItrT _image_begin, ItrT _image_end) {
            num_entries = _image_end - _image_begin;

            std::fill(bins.begin(), bins.end(),0);

            sqeazy::detail::parallel::fill_histogram(_image_begin, _image_end, bins.data(), nthreads);

            fill_stats();

        }

        integral_type calc_integral() const {

            integral_type value = sqeazy::detail::sum(bins.begin(),bins.end(),integral_type(0));// std::accumulate(bins.begin(),bins.end(),0);

            return value;
        }

        integral_type integral() const {

            return integral_value;

        }

        CounterT entries() const {
            return num_entries;
        }

        T calc_mode() const {
            T value = sqeazy::detail::max_element_distance(bins.begin(),bins.end(),this->n_threads());
            return value;
        }

        T mode() const {
            return mode_value;
        }

        float calc_mean() const {

            const twice_value_type end = large_pop_bin_value +1;
            const float inv_integral = 1.f/integral();

            float value = detail::index_weighted_sum(bins.begin()+small_pop_bin_value,
                                                     bins.begin()+end,
                                                     0.f,
                                                     small_pop_bin_value,
                                                     n_threads());

            return value*inv_integral;
        }

        float mean() const {
            return mean_value;
        }

        float calc_mean_variation() const {

            const twice_value_type end = large_pop_bin_value +1;

            float mean_variation = detail::unnormalized_variation(bins.begin()+small_pop_bin_value,
                                                                  bins.begin()+end,
                                                                  0.f,
                                                                  mean(),
                                                                  small_pop_bin_value,
                                                                  n_threads());
            mean_variation/=integral();

            return std::sqrt(mean_variation);

        }

        float mean_variation() const {
            return mean_variation_value;
        }

        T calc_largest_populated_bin() const {

            auto fitr = std::find_if_not(bins.rbegin(),bins.rend(),[](bins_type value){ return value == 0;});
            T value = bins.size() - std::distance(bins.rbegin(),fitr) - 1;
            return value;
        }

        T largest_populated_bin() const {

            return large_pop_bin_value;
        }


        T calc_smallest_populated_bin() const {

            auto fitr = std::find_if_not(bins.begin(),bins.end(),[](bins_type value){ return value == 0;});
            T value = std::distance(bins.begin(),fitr);

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

        /**
           \brief calculate the support of the histogram
           the support is the value of the histogram x-axis at which the cumulative PDF reaches the threshold given, here the weighted mean between the support index bin found and the bin before it is calculated

           \param[in] _threshold value of cumulative PDF at which the bin value is returned (threshold is required to be inside the closed real interval [0.,1.]

           \return
           \retval

        */
        float calc_support(float _threshold = .99) const {

            float result = 0;

            if(_threshold>1.){
                return result;
            }

            if(_threshold<0.){
                return result;
            }

            T mindex = support_index(bins.begin()+smallest_populated_bin(),
                                     bins.begin()+largest_populated_bin() +1,
                                     _threshold
                ) - bins.begin();

            if(mindex>0 ) //the underlying distribution has always even number of entries
                result = (bins[mindex]*mindex + bins[mindex-1]*(mindex-1))/float(bins[mindex-1] + bins[mindex]);

            return result;

        }



        float calc_median_variation() const {


            const twice_value_type end = large_pop_bin_value +1;

            float median_variation = detail::unnormalized_variation(bins.begin()+small_pop_bin_value,
                                                                    bins.begin()+end,
                                                                    0.f,
                                                                    median(),
                                                                    small_pop_bin_value,
                                                                    n_threads());
            // float median_variation = 0;
            // float temp = 0;
            // for(twice_value_type i = smallest_populated_bin(); i<(end); ++i) {
            //     temp = i - median();
            //     median_variation += (temp)*(temp)*bins[i];
            // }

            median_variation/=integral();

            return std::sqrt(median_variation);

        }



        float median() const {
            return median_value;
        }

        float median_variation() const {
            return median_variation_value;
        }

        /**
         *  \brief calculate the shannon entropy of the bin entries
         *
         *  the definition of shannon entropy given here is based on the log2 algorithm.
         *  H_1 = - sum_i(freq_i/total * log2(freq_i/total))
         *
         *  \param param
         *  \return return type
         */
        float calc_entropy() const {

            const float inv_integral = 1.f/integral();

            bins_citer_t binsI = bins.begin() + smallest_populated_bin();
            bins_citer_t binsE = bins.begin() + largest_populated_bin() + 1;

            float value = detail::reduce_to_entropy(binsI,binsE, inv_integral, n_threads());

            return -value;
        }

        float entropy() const {

            return entropy_value;

        }



        template <typename int_type>
        void set_n_threads(int_type _nthreads){
            nthreads = _nthreads;
        }

        std::uint32_t n_threads() const {
            return nthreads;
        }

        float support() const {
            return support_value;
        }

        ~histogram() {

        }

        friend std::ostream& operator<<(std::ostream& _cout, const histogram& _h) {

            double max_compr_ratio = double(_h.integral()*sizeof(value_type));
            max_compr_ratio/=double(_h.integral()*_h.entropy())/double(CHAR_BIT);

            _cout << "\t" << _h.integral()
                  << "\t" << _h.smallest_populated_bin()
                  << "\t" << _h.largest_populated_bin()
                  << "\t" << _h.mean()
                  << "\t" << _h.mean_variation()
                  << "\t" << _h.mode()
                  << "\t" << _h.median()
                  << "\t" << _h.median_variation()
                  << "\t" << _h.entropy()
                  << "\t" << max_compr_ratio
                // << "\n"
                ;

            return _cout;
        }

        static std::string print_header() {

            std::ostringstream out;
            out << "\t" << "n_entries"
                << "\t" << "first_bin"
                << "\t" << "last_bin"
                << "\t" << "mean"
                << "\t" << "mean_var"
                << "\t" << "mode"
                << "\t" << "median"
                << "\t" << "med_var"
                << "\t" << "entropy"
                << "\t" << "max_compr_ratio"
                ;

            return out.str();
        }

    };

    template <typename T>
    float mad(T begin, T end, int _nthreads = 1) {

        typedef typename std::iterator_traits<T>::value_type value_type;
        // typedef typename std::iterator_traits<T>::pointer ptr_type;

        typedef std::vector<value_type> local_vector;

        histogram<value_type> h_original(begin, end);
        unsigned size = end - begin;
        const float median = h_original.median();

        local_vector reduced(size);

        // typename local_vector::const_iterator ibegin = begin;
        // typename local_vector::const_iterator iend = end;

        // ptr_type red = reduced.data();
        // ptr_type input = &(*begin);
        // for(; ibegin!=iend; ++ibegin, ++redbegin) {
        //     red[i] = std::fabs(*ibegin - median);
        // }
        detail::abs_diff_to(begin,end,reduced.begin(), median, _nthreads);
        histogram<value_type> h_reduced(reduced.begin(), reduced.end());

        return h_reduced.median();

    }



}//sqeazy

#endif /* _HIST_IMPL_H_ */
