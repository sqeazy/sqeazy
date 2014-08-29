#ifndef _BENCH_UTILS_HPP_
#define _BENCH_UTILS_HPP_
#include <chrono>
#include <numeric>
#include <vector>
#include <string>
#include <array>
#include <climits>
#include <boost/concept_check.hpp>
#include "hist_impl.hpp"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>

extern "C" {
#include "sqeazy.h"
}

namespace sqeazy_bench {

template <typename T>
bool pop_if_contained(std::vector<T>& _data, const T& _pred) {

    auto found = std::find(_data.begin(), _data.end(), _pred);
    if(found!=_data.end()) {
        _data.erase(found);
        return true;
    }
    else
        return false;
}

template <typename T>
T pop_next_if_contained(std::vector<T>& _data, const T& _pred) {

    T value;
    typename std::vector<T>::iterator found = std::find(_data.begin(), _data.end(), _pred);
    if(found!=_data.end()) {
        value = *(found + 1);
        if(found + 2 <= _data.end())
            _data.erase(found,found + 2);
        else
            _data.erase(found, _data.end());
        return value;
    }
    else
        return value;
}

std::array<std::string,2> split_last_of(const std::string& _data, const std::string& _del) {

    std::array<std::string,2> value;
    size_t pos_last_del = _data.rfind(_del);
    value[0] = _data.substr(0,pos_last_del);
    value[1] = _data.substr(pos_last_del );
    return value;
}

template <typename It, typename V>
V sample_variance(const It& _begin,
                  const It& _end,
                  const V& _mean) {

    typedef typename std::iterator_traits<It>::value_type return_type;

    return_type variance = 0;
    auto begin = _begin;
    for(; begin!=_end; ++begin) {
        variance += (*begin - _mean)*(*begin - _mean);
    }

    return_type value = std::sqrt(variance)/((_end - _begin) - 1);
    return value;
}

template <typename ValueT>
struct bcase {

    typedef ValueT value_type;

    std::string filename;
    std::vector<int> dims;
    sqeazy::histogram<value_type> histogram;
    unsigned long raw_size_in_byte = 0;
    unsigned long compressed_size_in_byte = 0;

    decltype(std::chrono::high_resolution_clock::now()) start_time;
    double time_us = 0.;

    int return_code = 0;

    template <typename T = int>
    bcase(const std::string& _filename = "",
          const value_type* _data = 0,
          const std::vector<T>& _extents = {0,0,0}):
        filename(_filename),
        dims(_extents.begin(), _extents.end()),
        raw_size_in_byte(std::accumulate(_extents.begin(), _extents.end(),1,std::multiplies<unsigned long>())*sizeof(value_type))
    {
        histogram = sqeazy::histogram<value_type>(_data, _data + (raw_size_in_byte/sizeof(value_type)));
        start_time = std::chrono::high_resolution_clock::now();
    }

    bool has_run() const {

        return compressed_size_in_byte > 0 && raw_size_in_byte > 0;

    }

    void stop(const unsigned long& _compr_size) {

        auto end_time = std::chrono::high_resolution_clock::now();
        time_us = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
        compressed_size_in_byte = _compr_size;

    }

    double time_in_microseconds() const {

        return time_us;

    }

    double compress_ratio() const {

        if(compressed_size_in_byte)
            return double(raw_size_in_byte)/double(compressed_size_in_byte);
        else
            return 0;
    }

    double max_compress_ratio() const {

        const double sample_entropy = histogram.entropy();
        double best_compressed_size_in_byte = sample_entropy*histogram.integral()/double(CHAR_BIT);
        if(best_compressed_size_in_byte>0)
            return double(raw_size_in_byte)/best_compressed_size_in_byte;
        else
            return 0;
    }

    double compress_speed_mb_per_sec()  {

        double time_sec = time_in_microseconds()/double(1e6);
        double value_mb = raw_size_in_byte/double(1 << 20);

        if(time_sec)
            return value_mb/time_sec;
        else
            return 0.;
    }


    friend std::ostream& operator<<(std::ostream& _cout, const bcase& _in) {

        _cout << _in.raw_size_in_byte << "\t";

        for( const int& n : _in.dims ) {
            if(n != _in.dims.back())
                _cout << n << "x";
            else
                _cout << n << "\t";
        }

        _cout << _in.histogram.smallest_populated_bin() << "\t"
              << _in.histogram.largest_populated_bin() << "\t"
              << _in.histogram.mode() << "\t"
              << _in.histogram.mean() << "\t"
              << _in.time_us << "\t"
              << _in.compressed_size_in_byte << "\t"
              << _in.compress_ratio()<< "\t"
	      << _in.compress_speed_mb_per_sec()<< "\t"
              << _in.max_compress_ratio()<< "\t"
              << _in.filename;

        return _cout;
    }

    static const std::string print_header() {
        std::ostringstream _cout;

        _cout << "raw_size/Byte\textents\tsmallest-pop-bin\tlargest-pop-bin\tmode\tmean\t"
              << "time-to-compress/us\tcompressed_size/Byte\tcompress-ratio\tspeed/[MB/s]\tmax-compress-ratio\tfilename";

        return _cout.str();
    }

};

template <typename T>
struct bsuite {

    typedef T value_type;

    std::vector<bcase<value_type> > cases;
    boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::mean, boost::accumulators::tag::median(boost::accumulators::with_p_square_quantile), boost::accumulators::tag::min, boost::accumulators::tag::max > > compression_accumulator;
    boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::mean, boost::accumulators::tag::median(boost::accumulators::with_p_square_quantile), boost::accumulators::tag::min, boost::accumulators::tag::max > > speed_accumulator;

    bsuite(const int& _total=0)
    {
        cases.reserve(_total);

    }

    void at(const unsigned int& _index, bcase<value_type>& _new) {

        cases.at(_index) = (_new);
        compression_accumulator(_new.compress_ratio());
        speed_accumulator(_new.compress_speed_mb_per_sec());

    }

    void push_back(bcase<value_type>& _new) {

        cases.push_back(_new);
        compression_accumulator(_new.compress_ratio());
        speed_accumulator(_new.compress_speed_mb_per_sec());

    }

    void compression_summary() const {


        std::cout << boost::accumulators::min( compression_accumulator ) << "\t"
                  << boost::accumulators::mean( compression_accumulator ) << "\t"
                  << boost::accumulators::max( compression_accumulator ) << "\t"
                  << boost::accumulators::median( compression_accumulator ) << "\t";

    }

    void speed_summary() const {


        std::cout << boost::accumulators::min( speed_accumulator ) << "\t"
                  << boost::accumulators::mean( speed_accumulator ) << "\t"
                  << boost::accumulators::max( speed_accumulator ) << "\t"
                  << boost::accumulators::median( speed_accumulator )  << "\t" ;

    }

    void print_cases() const {

        std::cout << sqeazy_bench::bcase<value_type>::print_header() << "\n";
        for(const bcase<value_type>& c : cases) {

            std::cout << c<< "\n";

        }

    }

    unsigned size() const {
        return cases.size() ;
    }
    unsigned empty() const {
        return cases.empty() ;
    }

};

};
#endif /* _BENCH_UTILS_HPP_ */
