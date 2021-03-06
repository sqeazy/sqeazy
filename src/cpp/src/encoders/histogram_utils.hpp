#ifndef HISTOGRAM_UTILS_H
#define HISTOGRAM_UTILS_H


#include <vector>
#include <iterator>
#include <numeric>
#include <cmath>

#include "sqeazy_common.hpp"

namespace sqeazy {

    namespace detail {

        template <typename T>
        struct dense_histo
        {

            static_assert(std::is_integral<T>::value,"[dense_histo] input type is not integral");
            static const std::size_t n_possible_values = (1 << (sizeof(T)*CHAR_BIT));
            static const std::size_t size = n_possible_values;
            typedef std::vector<std::uint32_t> type;


        };

        namespace serial {


/**
         *  \brief fill the histogram represented by _bins sequentially with the value counts/frequency observed in [_begin,_end); it is assumed that _bins yields a linear range of range(0,max_value(*_begin))
         *
         *  \param _begin input iterator pointing to element 0 of the values to histogram (const iterators allowed)
         *  \param _end input iterator pointing to the last+1 element of the values to histogram (const iterators allowed)
         *  \param _histo input iterator of the histogram bins (assumed to yield a linear range of range(0,max_value(*_begin)))
         *  \return void
         */
        template <typename iter_type,
                  typename histo_iter_type>
        histo_iter_type fill_histogram(iter_type _begin,
                                       iter_type _end,
                                       histo_iter_type _histo)

        {

            using value_t = typename std::iterator_traits<iter_type>::value_type;
            auto len = dense_histo< value_t >::size;

            for(;_begin!=_end;++_begin){
                *(_histo+*_begin) +=1;
            }

            return _histo + len;
        }


            /**
         *  \brief create-initialize the histogram sequentially with the value counts/frequency observed in [_begin,_end)
         *
         *  \param _begin input iterator pointing to element 0 of the values to histogram (const iterators allowed)
         *  \param _end input iterator pointing to the last+1 element of the values to histogram (const iterators allowed)
         *  \return std::vector yielding the value counts in [0,max_value(*_begin))]
         */
        template <typename iter_type>
        auto create_histogram(iter_type _begin,
                                   iter_type _end
            ) -> typename dense_histo<typename std::iterator_traits<iter_type>::value_type>::type
        {

            using value_t = typename std::iterator_traits<iter_type>::value_type;
            using histo_t = typename dense_histo<typename std::iterator_traits<iter_type>::value_type>::type;
            auto size = dense_histo< value_t >::size;

            histo_t histo(size, 0u);

            fill_histogram(_begin,_end,histo.begin());

            return histo;
        }

    }; // serial

        namespace parallel {



            /**
             *  \brief fill the histogram represented by _bins concurrently with the value counts/frequency observed in [_begin,_end); it is assumed that _bins yields a linear range of range(0,max_value(*_begin))
             *
             *  \param _begin input iterator pointing to element 0 of the values to histogram (const iterators allowed)
             *  \param _end input iterator pointing to the last+1 element of the values to histogram (const iterators allowed)
             *  \param _bins input iterator of the histogram bins (assumed to yield a linear range of range(0,max_value(*_begin)))
             *  \param nthreads number of threads to use
             *  \return void
             */
            template <typename iter_type, typename bin_iter_type>
            bin_iter_type fill_histogram(iter_type _begin,
                                iter_type _end,
                                bin_iter_type _bins,
                                int nthreads = 1
                )
            {

                if(nthreads == 1)
                    return serial::fill_histogram(_begin, _end, _bins);

                if(nthreads <= 0)
                    nthreads = std::thread::hardware_concurrency();

                typedef typename std::iterator_traits<iter_type>::value_type value_type;

                using histo_t = typename dense_histo<value_type>::type;
                histo_t histo(_bins,_bins + dense_histo<value_type>::size);

                std::vector<histo_t> histo_clones(nthreads, histo);
                auto histo_clones_itr = histo_clones.data();
                const auto len = std::distance(_begin,_end);
                const omp_size_type chunk_size = (len + nthreads - 1)/nthreads;


                //mapping the partitions of [_begin, _end) to histo_clones
#pragma omp parallel                            \
    shared( histo_clones_itr )                  \
    firstprivate( _begin, _end, chunk_size )    \
    num_threads(nthreads)
                {
                    auto tid            = omp_get_thread_num();
                    auto chunk_begin    = _begin + tid*chunk_size;
                    auto chunk_end      = chunk_begin + chunk_size >= _end ? _end : chunk_begin + chunk_size;
                    auto my_histo_itr   = &(*(histo_clones_itr+tid))[0];

                    for(;chunk_begin!=chunk_end;++chunk_begin)
                        my_histo_itr[(std::uint32_t)*chunk_begin]++;

                }

                const omp_size_type histo_len = histo.size();
                const omp_size_type n_clones = histo_clones.size();

#pragma omp parallel for                        \
    shared( _bins )                             \
    firstprivate( histo_clones_itr, n_clones )  \
    num_threads(nthreads)
            for(omp_size_type idx = 0;idx<histo_len;idx++){
                for(omp_size_type clone = 0;clone<n_clones;++clone){
                    *(_bins + idx) += (histo_clones_itr+clone)->data()[idx];
                }

            }

            return _bins + histo.size();

        }

            template <typename iter_type>
        auto create_histogram(iter_type _begin,
                              iter_type _end,
                              int nthreads = 1
            ) -> typename dense_histo<typename std::iterator_traits<iter_type>::value_type>::type
        {
            using value_t = typename std::iterator_traits<iter_type>::value_type;
            using histo_t = typename dense_histo<typename std::iterator_traits<iter_type>::value_type>::type;
            auto size = dense_histo< value_t >::size;

            histo_t histo(size, 0u);

            parallel::fill_histogram(_begin,_end,histo.begin(),nthreads);

            return histo;

        }

        };


        /**
         *  \brief simple accumulate clone that assumes an addition over all entries in [_begin,_end)

         *  Note: the serial version is better for small ranges of [_begin,_end), whereas the large ones tend to run faster in parallel (cache line or threading overhead effects suspected)
         *
         *  Run on (4 X 3600 MHz CPU s)
         *  ----------------------------------------------------------------------------------------
         *  Benchmark                                                 Time           CPU Iterations
         *  ----------------------------------------------------------------------------------------
         *  BM_serial_accumulate<std::uint32_t>/256                 224 ns        223 ns    3153560   4.27013GB/s
         *  BM_serial_accumulate<std::uint32_t>/65535             55585 ns      55323 ns      12172   4.41291GB/s
         *  BM_serial_accumulate<std::uint32_t>/2147483647   1984397518 ns 1961781342 ns          1   4.07793GB/s
         *  BM_parallel_accumulate<std::uint32_t>/256               898 ns        888 ns     850214   1099.42MB/s
         *  BM_parallel_accumulate<std::uint32_t>/65535           32438 ns      31136 ns      21112   7.84102GB/s
         *  BM_parallel_accumulate<std::uint32_t>/2147483647 1025681803 ns  916335018 ns          1   8.73043GB/s
         *
         *  \param _begin start of range [_begin,_end)
         *  \param _begin end of range [_begin,_end)
         *  \param _start starting value of the result
         *  \param _nthreads how many threads to use
         *  \return decltype(_start)
         */
        template <typename iter, typename value_type>
        value_type sum(iter _begin, iter _end, value_type _start, int _nthreads = 1){

            if(_nthreads == 1)
                return std::accumulate(_begin, _end, _start);

            if(_nthreads <= 0)
                _nthreads = std::thread::hardware_concurrency();

            const omp_size_type  len = std::distance(_begin,_end);
            value_type value = _start;
            omp_size_type chunk = (len + _nthreads -1)/_nthreads;

#pragma omp parallel for                        \
    shared(_begin)                              \
    schedule(static,chunk)                      \
    reduction(+:value)                          \
    num_threads(_nthreads)
            for(omp_size_type i = 0;i<len;++i){
                value += *(_begin+i);
            }

            return value;
        }

        /**
         *  \brief calculate the weighted sum of the array, where the weight per item is it's index
         *
         *  Example: array = {5,4,3,2,1,0};
         *           index_weighted_sum = 5*0 + 4*1 + 3*2 + 2*3 + 1*4 + 0*5;
         *
         *  Note: the serial version is better for small ranges of [_begin,_end), whereas the large ones tend to run faster in parallel (cache line or threading overhead effects suspected)
         *  Note: this could be converted to a dot-product where the second container holds the index values to weight by
         *
         *  Run on (4 X 3577.92 MHz CPU s)
         *  2017-04-20 16:24:43
         *  ***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
         *  Benchmark                                                      Time           CPU Iterations
         *  --------------------------------------------------------------------------------------------
         *  BM_serial_index_weighted_sum<std::uint32_t>/256              222 ns        222 ns    3155531   4.29228GB/s
         *  BM_serial_index_weighted_sum<std::uint32_t>/63.999k        55733 ns      55660 ns      11788   4.38621GB/s
         *  BM_serial_index_weighted_sum<std::uint32_t>/48M         45664822 ns   45367840 ns         15   4.13288GB/s
         *  BM_parallel_index_weighted_sum<std::uint32_t>/256            881 ns        872 ns     831324   1120.02MB/s
         *  BM_parallel_index_weighted_sum<std::uint32_t>/63.999k      34093 ns      33114 ns      21384   7.37264GB/s
         *  BM_parallel_index_weighted_sum<std::uint32_t>/48M       24234586 ns   23387437 ns         30   8.01712GB/s
         *
         *  \param _begin start of range [_begin,_end)
         *  \param _begin end of range [_begin,_end)
         *  \param _start_value starting value of the result
         *  \param _start_index starting offset of the index to use for weighting
         *  \param _nthreads how many threads to use
         *  \return decltype(_start)
         */
        template <typename iter, typename value_type>
        value_type index_weighted_sum(iter _begin, iter _end,
                                      value_type _start_value,
                                      omp_size_type _start_index = 0,
                                      int _nthreads = 1){

            if(_nthreads <= 0)
                _nthreads = std::thread::hardware_concurrency();

            const omp_size_type  len = std::distance(_begin,_end);
            value_type value = _start_value;

            if(len < _nthreads || _nthreads == 1){
                for(omp_size_type i = 0;i<len;++i){
                    value += (i+_start_index)*(*(_begin+i));
                }
                return value;
            }

            omp_size_type chunk = (len + _nthreads -1)/_nthreads;

#pragma omp parallel for                        \
    shared(_begin)                              \
    firstprivate(_start_index)                              \
    schedule(static,chunk)                      \
    reduction(+:value)                          \
    num_threads(_nthreads)
            for(omp_size_type i = 0;i<len;++i){
                value += (i+_start_index)*(*(_begin+i));
            }

            return value;
        }

        /**
         *  \brief calculate the variance around _mean
         *
         *  Example: array = {5,4,3,2,1,0};
         *           index_weighted_sum = 5*0*0 + 4*1*1 + 3*2*2 + 2*3*3 + 1*4*4 + 0*5*5;
         *
         *  Note: the serial version is better for small ranges of [_begin,_end), whereas the large ones tend to run faster in parallel (cache line or threading overhead effects suspected)
         *  Note: this could be converted to a dot-product where the second container holds the index values to weight by
         *
         *  Run on (4 X 3371 MHz CPU s)
         *  2017-04-20 16:39:48
         *  ***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
         *  Benchmark                                                               Time           CPU Iterations
         *  -----------------------------------------------------------------------------------------------------
         *  BM_serial_unnormalized_mean_variation<std::uint32_t>/256              386 ns        384 ns    1879320    2.4816GB/s
         *  BM_serial_unnormalized_mean_variation<std::uint32_t>/63.999k        96004 ns      95688 ns       7089   2.55138GB/s
         *  BM_serial_unnormalized_mean_variation<std::uint32_t>/48M         73735729 ns   73143725 ns         10   2.56345GB/s
         *  BM_parallel_unnormalized_mean_variation<std::uint32_t>/256            970 ns        945 ns     815958   1032.98MB/s
         *  BM_parallel_unnormalized_mean_variation<std::uint32_t>/63.999k      69403 ns      61754 ns      13774   3.95336GB/s
         *  BM_parallel_unnormalized_mean_variation<std::uint32_t>/48M       40995208 ns   36816906 ns         19   5.09277GB/s
         *
         *  \param _begin start of range [_begin,_end)
         *  \param _begin end of range [_begin,_end)
         *  \param _start_value starting value of the result
         *  \param _start_index starting offset of the index to use for weighting
         *  \param _nthreads how many threads to use
         *  \return decltype(_start)
         */
        template <typename iter, typename value_type>
        value_type unnormalized_variation(iter _begin, iter _end,
                                          value_type _start_value,
                                          value_type _mean,
                                          omp_size_type _start_index = 0,
                                          int _nthreads = 1){

            if(_nthreads <= 0)
                _nthreads = std::thread::hardware_concurrency();

            const omp_size_type  len = std::distance(_begin,_end);
            value_type value = _start_value;

            if(len < _nthreads || _nthreads == 1){
                for(omp_size_type i = 0;i<len;++i){
                    auto weight = value_type(i+_start_index) - _mean;
                    value += weight*weight*(*(_begin+i));
                }
                return value;
            }

            omp_size_type chunk = (len + _nthreads -1)/_nthreads;

#pragma omp parallel for                        \
    shared(_begin)                              \
    firstprivate(_start_index, _mean)                 \
    schedule(static,chunk)                      \
    reduction(+:value)                          \
    num_threads(_nthreads)
            for(omp_size_type i = 0;i<len;++i){
                auto weight = value_type(i+_start_index) - _mean;
                value += weight*(*(_begin+i));
            }

            return value;
        }

        /**
         *  \brief function to determine the offset of the largest element in [_begin,_end)
         *
         *  Detailed description
         *  Run on (4 X 3600 MHz CPU s)
         *  -----------------------------------------------------------------------------------------
         *  Benchmark                                                  Time           CPU Iterations
         *  -----------------------------------------------------------------------------------------
         *  BM_serial_max_element<std::uint32_t>/256                 629 ns        626 ns    1113816   1.52368GB/s
         *  BM_serial_max_element<std::uint32_t>/65535            166827 ns     166020 ns       4198   1.47053GB/s
         *  BM_serial_max_element<std::uint32_t>/2147483647   5521365086 ns 5500957139 ns          1   1.45429GB/s
         *  BM_parallel_max_element<std::uint32_t>/256               862 ns        856 ns     680501   1.11451GB/s
         *  BM_parallel_max_element<std::uint32_t>/65535           26867 ns      25819 ns      24894   9.45553GB/s
         *  BM_parallel_max_element<std::uint32_t>/2147483647  942780948 ns  901011798 ns          1   8.87891GB/s

         *  \param _begin start of range [_begin,_end)
         *  \param _begin end of range [_begin,_end)
         *  \param _nthreads how many threads to use
         *  \return std::size_t
         */
        template <typename iter>
        std::size_t max_element_distance(iter _begin, iter _end, int _nthreads = 1)
        {

            typedef typename std::iterator_traits<iter>::value_type value_t;

            if(_nthreads == 1)
                return std::distance(_begin,
                                     std::max_element(_begin,_end));

            if(_nthreads <= 0)
                _nthreads = std::thread::hardware_concurrency();

            const omp_size_type len = std::distance(_begin,_end);
#ifdef _WIN32
            value_t max_value = (std::numeric_limits<value_t>::min)();
#else
			value_t max_value = std::numeric_limits<value_t>::min();
#endif
            std::size_t value = len;

#pragma omp parallel for                        \
    shared(_begin, value)
            for (omp_size_type i = 0; i < len; ++i)
            {
                auto current = *(_begin + i);
                if (current > max_value)
                {
#pragma omp critical
                    {
                        max_value = *(_begin + i);
                        value = i;
                    }
                }
            }

            return value;
        }

        /**
         *  \brief compute the shannon entropy of the histogram or frequencies specified by the range [_begin, _end)
         *
         *  the definition of shannon entropy given here is based on the log2 algorithm.
         *  H_1 = - sum_i(freq_i/total * log2(freq_i/total))
         *
         *  Run on (4 X 2114.29 MHz CPU s)
         *  2017-04-21 14:59:57
         *  ***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
         *  Benchmark                                                     Time           CPU Iterations
         *  -------------------------------------------------------------------------------------------
         *  BM_serial_reduce_to_entropy<std::uint32_t>/256             3646 ns       3636 ns     190316   268.582MB/s
         *  BM_serial_reduce_to_entropy<std::uint32_t>/63.999k       876031 ns     872896 ns        805   286.399MB/s
         *  BM_serial_reduce_to_entropy<std::uint32_t>/48M        677572517 ns  674886160 ns          1   284.492MB/s
         *  BM_parallel_reduce_to_entropy<std::uint32_t>/256           7015 ns       6860 ns     109097   142.356MB/s
         *  BM_parallel_reduce_to_entropy<std::uint32_t>/63.999k    1578043 ns    1488346 ns        498   167.969MB/s
         *  BM_parallel_reduce_to_entropy<std::uint32_t>/48M     1099013001 ns  293529946 ns          2   654.107MB/s
         *
         *  \param _begin start of range [_begin,_end)
         *  \param _end end of range [_begin,_end)
         *  \param _inv_integral normalisation constant to replace 1/(total) from above to *_inv_integral
         *  \param _nthreads how many threads to use
         *  \return result_t
         */
        template <typename iter_t, typename result_t = float>
        result_t reduce_to_entropy(iter_t _begin, iter_t _end, const result_t _inv_integral, int _nthreads = 1){

            if(_nthreads <= 0)
                _nthreads = std::thread::hardware_concurrency();

            const omp_size_type len = std::distance(_begin,_end);

            if(len < _nthreads)
                _nthreads = 1;

            // const result_t inv_log_2 = result_t(1.)/std::log(result_t(2.f));
            // const result_t const_zero = result_t(0.);
            const omp_size_type chunk = (len + _nthreads -1)/_nthreads;
            result_t value = 0;

#pragma omp parallel for                        \
    shared(_begin)                              \
    firstprivate(_inv_integral)     \
    schedule(static,chunk)                      \
    reduction(+:value)                          \
    num_threads(_nthreads)
            for(omp_size_type i = 0;i<len;++i){
                result_t temp = (*(_begin + i)) > 0 ? (*(_begin + i)) * _inv_integral : 1.;
                value += temp*std::log2(temp);
            }

            return value;
        }

                /**
         *  \brief calculate the absolute distance to value
         *
         *  Run on (4 X 3371 MHz CPU s)
         *  2017-04-20 16:39:48
         *  ***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
         *  Benchmark                                                               Time           CPU Iterations
         *  -----------------------------------------------------------------------------------------------------
         *  BM_serial_unnormalized_mean_variation<std::uint32_t>/256              386 ns        384 ns    1879320    2.4816GB/s
         *  BM_serial_unnormalized_mean_variation<std::uint32_t>/63.999k        96004 ns      95688 ns       7089   2.55138GB/s
         *  BM_serial_unnormalized_mean_variation<std::uint32_t>/48M         73735729 ns   73143725 ns         10   2.56345GB/s
         *  BM_parallel_unnormalized_mean_variation<std::uint32_t>/256            970 ns        945 ns     815958   1032.98MB/s
         *  BM_parallel_unnormalized_mean_variation<std::uint32_t>/63.999k      69403 ns      61754 ns      13774   3.95336GB/s
         *  BM_parallel_unnormalized_mean_variation<std::uint32_t>/48M       40995208 ns   36816906 ns         19   5.09277GB/s
         *
         *  \param _begin start of range [_begin,_end)
         *  \param _end end of range [_begin,_end)
         *  \param _obegin start iterator to output range
         *  \param _start_index starting offset of the index to use for weighting
         *  \param _nthreads how many threads to use
         *  \return decltype(_start)
         */
        template <typename iter, typename out_iter, typename value_type>
        out_iter abs_diff_to(iter _begin, iter _end,
                               out_iter _obegin,
                               value_type _diff_to,
                               int _nthreads = 1){

            using iter_value_t = typename std::iterator_traits<iter>::value_type;
            // using oiter_value_t = typename std::iterator_traits<out_iter>::value_type;
            using iter_pointer_t = typename std::iterator_traits<iter>::pointer;
            using oiter_pointer_t = typename std::iterator_traits<out_iter>::pointer;

            if(_nthreads <= 0)
                _nthreads = std::thread::hardware_concurrency();

            const omp_size_type  len = std::distance(_begin,_end);

            if(len < _nthreads || _nthreads == 1){
                return std::transform(_begin, _end, _obegin, [&]( iter_value_t _element ){ return std::fabs(_element - _diff_to);});
            }

            iter_pointer_t idata = &(*_begin);
            oiter_pointer_t odata = &(*_obegin);

            omp_size_type chunk = (len + _nthreads -1)/_nthreads;

#pragma omp parallel for                        \
    shared(odata,idata)                                    \
    firstprivate(_diff_to)                 \
    schedule(static,chunk)                      \
    num_threads(_nthreads)
            for(omp_size_type i = 0;i<len;++i){
                odata[i] = std::fabs(idata[i] - _diff_to);
            }

            return _obegin+len;
        }


    }  // detail

}  // sqeazy

#endif /* HISTOGRAM_UTILS_H */
