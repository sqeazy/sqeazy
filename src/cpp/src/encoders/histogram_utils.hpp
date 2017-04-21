#ifndef HISTOGRAM_UTILS_H
#define HISTOGRAM_UTILS_H

#include <array>
#include <iterator>

#include "sqeazy_common.hpp"

namespace sqeazy {

    namespace detail {

        template <typename T>
        struct dense_histo
        {

            static_assert(std::is_integral<T>::value,"[dense_histo] input type is not integral");
            static const std::size_t n_possible_values = (1 << (sizeof(T)*CHAR_BIT));
            typedef std::array<std::uint32_t,n_possible_values> type;


        };

        template <typename iter_type>
        auto serial_fill_histogram(iter_type _begin,
                                   iter_type _end
            ) -> typename dense_histo<typename std::iterator_traits<iter_type>::value_type>::type
        {

            using histo_t = typename dense_histo<typename std::iterator_traits<iter_type>::value_type>::type;

            histo_t histo;
            histo.fill(0u);

            for(;_begin!=_end;++_begin){
                histo[*_begin]++;
            }

            return histo;
        }

        template <typename iter_type>
        auto parallel_fill_histogram(iter_type _begin,
                                     iter_type _end,
                                     int nthreads = 1
            ) -> typename dense_histo<typename std::iterator_traits<iter_type>::value_type>::type
        {
            using histo_t = typename dense_histo<typename std::iterator_traits<iter_type>::value_type>::type;

            histo_t histo;
            histo.fill(0u);

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
                    my_histo_itr[*chunk_begin]++;

            }

            auto histo_itr = histo.data();
            const omp_size_type histo_len = histo.size();
            const omp_size_type n_clones = histo_clones.size();

#pragma omp parallel for                        \
    shared(histo_itr )                          \
    firstprivate( histo_clones_itr, n_clones )  \
    num_threads(nthreads)
            for(omp_size_type idx = 0;idx<histo_len;idx++){
                for(omp_size_type clone = 0;clone<n_clones;++clone){
                    *(histo_itr + idx) += (histo_clones_itr+clone)->data()[idx];
                }

            }

            return histo;

        }

        /**
         *  \brief fill the histogram represented by _bins sequentially with the value counts/frequency observed in [_begin,_end); it is assumed that _bins yields a linear range of range(0,max_value(*_begin))
         *
         *  \param _begin input iterator pointing to element 0 of the values to histogram (const iterators allowed)
         *  \param _end input iterator pointing to the last+1 element of the values to histogram (const iterators allowed)
         *  \param _bins input iterator of the histogram bins (assumed to yield a linear range of range(0,max_value(*_begin)))
         *  \return void
         */
        template <typename iter_type, typename bin_iter_type>
        void dense_serial_fill_histogram(iter_type _begin,
                                         iter_type _end,
                                         bin_iter_type _bins
            )
        {

            // typedef typename std::iterator_traits<iter_type>::value_type value_type;
            // static const std::uint32_t n_possible_values = (1 << (sizeof(value_type)*CHAR_BIT));

            for(;_begin!=_end;++_begin){
                *(_bins + *_begin)+=1;
            }

            return;
        }

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
        void dense_parallel_fill_histogram(iter_type _begin,
                                           iter_type _end,
                                           bin_iter_type _bins,
                                           int nthreads = 1
            )
        {

            if(nthreads == 1)
                return dense_serial_fill_histogram(_begin, _end, _bins);

            if(nthreads <= 0)
                nthreads = std::thread::hardware_concurrency();

            typedef typename std::iterator_traits<iter_type>::value_type value_type;

            using histo_t = typename dense_histo<value_type>::type;
            histo_t histo;
            histo.fill(0u);

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
                    my_histo_itr[*chunk_begin]++;

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

            return ;

        }


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
        value_type sum(iter _begin, iter _end, value_type _start, int _nthreads = 0){

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
                                      int _nthreads = 0){

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
                                          int _nthreads = 0){

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
        std::size_t max_element_distance(iter _begin, iter _end, int _nthreads = 0)
        {

            typedef typename std::iterator_traits<iter>::value_type value_t;

            if(_nthreads == 1)
                return std::distance(_begin,
                                     std::max_element(_begin,_end));

            if(_nthreads <= 0)
                _nthreads = std::thread::hardware_concurrency();

            const omp_size_type len = std::distance(_begin,_end);
            value_t max_value = std::numeric_limits<value_t>::min();
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

        template <typename iter_t, typename result_t = float>
        result_t reduce_to_entropy(iter_t _begin, iter_t _end, result_t _inv_integral, int _nthreads = 0){

            if(_nthreads <= 0)
                _nthreads = std::thread::hardware_concurrency();

            const omp_size_type len = std::distance(_begin,_end);
            const result_t inv_log_2 = result_t(1.)/std::log(result_t(2.f));
            const result_t const_zero = result_t(0.);
            const omp_size_type chunk = (len + _nthreads -1)/_nthreads;
            result_t value = 0;

#pragma omp parallel for                        \
    shared(_begin)                              \
    firstprivate(_inv_integral, const_zero)     \
    schedule(static,chunk)                      \
    reduction(+:value)                          \
    num_threads(_nthreads)
            for(omp_size_type i = 0;i<len;++i){
                const result_t temp = (*(_begin + i)) * _inv_integral;
                value += (*(_begin + i)) > 0 ? temp*std::log(temp)*inv_log_2 : const_zero;
            }

            return value;
        }

    }  // detail

}  // sqeazy

#endif /* HISTOGRAM_UTILS_H */
