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
            static const std::uint32_t n_possible_values = (1 << (sizeof(T)*CHAR_BIT));
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
#pragma omp parallel                         \
    shared( histo_clones_itr )                     \
    firstprivate( _begin, _end, chunk_size )        \
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
                shared(histo_itr )                     \
                firstprivate( histo_clones_itr, n_clones )      \
                num_threads(nthreads)
            for(omp_size_type idx = 0;idx<histo_len;idx++){
                for(omp_size_type clone = 0;clone<n_clones;++clone){
                    *(histo_itr + idx) += (histo_clones_itr+clone)->data()[idx];
                }

            }

            return histo;

        }
    }  // detail

}  // sqeazy

#endif /* HISTOGRAM_UTILS_H */
