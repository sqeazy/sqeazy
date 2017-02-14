#ifndef HISTOGRAM_UTILS_H
#define HISTOGRAM_UTILS_H

#include <array>
#include <iterator>

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

            return serial_fill_histogram(_begin, _end);
        }
    }  // detail

}  // sqeazy

#endif /* HISTOGRAM_UTILS_H */
