#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TEST_HIST_IMPL
#include "boost/test/unit_test.hpp"
#include "boost/random.hpp"
#include <numeric>
#include <cmath>
#include <vector>
#include <iostream>
#include "array_fixtures.hpp"
#include "../src/hist_impl.hpp"
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/moment.hpp>

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;
typedef sqeazy::array_fixture<unsigned char> uint8_cube_of_8;



BOOST_FIXTURE_TEST_SUITE( hist_impl_unsigned, uint8_cube_of_8 )

BOOST_AUTO_TEST_CASE( implemented )
{
    sqeazy::histogram<value_type> of_constant_by_size(&constant_cube[0], uint8_cube_of_8::size );
    unsigned size = uint8_cube_of_8::size;
    BOOST_CHECK_EQUAL(of_constant_by_size.entries(),size);
    BOOST_CHECK_GT(of_constant_by_size.integral(),0);

    sqeazy::histogram<value_type> of_constant_begin_end(&constant_cube[0], &constant_cube[0] + uint8_cube_of_8::size );
    BOOST_CHECK_GT(of_constant_begin_end.integral(),0);

    sqeazy::histogram<value_type> of_constant_begin_end_copied(of_constant_by_size);
    BOOST_CHECK_EQUAL(of_constant_begin_end_copied.integral(),of_constant_by_size.integral());
    BOOST_CHECK_EQUAL(of_constant_begin_end_copied.entries(),size);

    sqeazy::histogram<value_type> of_constant_begin_end_assigned = of_constant_by_size;
    BOOST_CHECK_EQUAL(of_constant_begin_end_assigned.integral(),of_constant_by_size.integral());
    BOOST_CHECK_EQUAL(of_constant_begin_end_assigned.entries(),size);
}


int parabola(const unsigned& _index) {

    //a parabola that has its maximum at (64,171)
    int value = -1*(_index)*(_index)/256 + _index/2 + 155;
    return value;
}

BOOST_AUTO_TEST_CASE( mode_and_mean )
{
    sqeazy::histogram<value_type> of_constant(&constant_cube[0], &constant_cube[0] + uint8_cube_of_8::size );

    BOOST_CHECK_EQUAL(of_constant.mode(),1);
    BOOST_CHECK_EQUAL(of_constant.mean(),1);

    value_type upper_limit = std::numeric_limits<value_type>::max()/2;

    for(unsigned i = 0; i<uint8_cube_of_8::size; ++i) {
        int eff_index = i % upper_limit;
        int value = parabola(eff_index);
        to_play_with[i] = value;
    }

    sqeazy::histogram<value_type> of_variable(&to_play_with[0], &to_play_with[0] + uint8_cube_of_8::size );
    BOOST_CHECK_EQUAL(of_variable.mode(),170);
    BOOST_CHECK_NE(of_variable.mode(),of_variable.mean());

}

BOOST_AUTO_TEST_CASE( min_max_populated_bin )
{
    value_type upper_limit = std::numeric_limits<value_type>::max()/2;

    for(unsigned i = 0; i<uint8_cube_of_8::size; ++i) {
        int eff_index = i % upper_limit;
        int value = parabola(eff_index);
        to_play_with[i] = value;
    }

    sqeazy::histogram<value_type> of_variable(&to_play_with[0], &to_play_with[0] + uint8_cube_of_8::size/2 );
    BOOST_CHECK_EQUAL(of_variable.largest_populated_bin(),*std::max_element(&to_play_with[0], &to_play_with[0] + size));
    BOOST_CHECK_EQUAL(of_variable.smallest_populated_bin(),
                      *std::min_element(&to_play_with[0], &to_play_with[0] + size));

}



BOOST_AUTO_TEST_CASE( median_vs_mean )
{
    boost::accumulators::accumulator_set<float,
          boost::accumulators::stats<boost::accumulators::tag::mean,
          boost::accumulators::tag::median
          >
          > to_play_with_acc;

    unsigned value_index = 0;
    unsigned idx = 0;
    for(; value_index<std::numeric_limits<value_type>::max(); ++value_index) {
        for(unsigned num = 0; num<value_index; ++num) {
            if(idx<size) {
                to_play_with[idx] = value_index;
            }
            ++idx;
        }
        if(idx>=size)
            break;
    }

    for(unsigned i=0; i<to_play_with.size(); ++i)
        to_play_with_acc(to_play_with[i]);

    sqeazy::histogram<value_type> of_variable(&to_play_with[0], size);
    value_type exp_median = sqeazy::round<value_type>(boost::accumulators::median(to_play_with_acc));
    value_type rec_median = sqeazy::round<value_type>(of_variable.calc_median());
    BOOST_CHECK_NE(of_variable.calc_mean(),of_variable.calc_median());
    try {
        BOOST_REQUIRE_EQUAL(rec_median,exp_median);
    }
    catch(...) {

        unsigned integral = of_variable.integral();
        unsigned running_int = 0;
        std::cout.setf(std::ios::dec);
        for(unsigned i = 0; i<of_variable.bins.size(); ++i) {
            if(of_variable.bins[i]) {
                running_int += of_variable.bins[i];
                std::cout << (int)i << "/" << (int)of_variable.bins.size() << " " << (int)of_variable.bins[i] << " - ";
                std::cout << running_int << "(" << running_int/float(integral) <<") ";
                if(of_variable.bins[i] == exp_median)
                    std::cout << " boost_median";
                if(rec_median == of_variable.bins[i])
                    std::cout << " hist_median";
                std::cout << "\n";
            }

        }

        std::cout << "running int = " << running_int << ", hist int = " << of_variable.integral() << "\n";
        std::cout << "boost_median = " << boost::accumulators::median(to_play_with_acc) << "\n";
        std::cout << "hist_median = " <<  of_variable.calc_median() << "\n";
    }

}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( hist_impl_unsigned16, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( median_variation )
{
    boost::accumulators::accumulator_set<float,
          boost::accumulators::stats<boost::accumulators::tag::mean,
          boost::accumulators::tag::median,
          boost::accumulators::tag::variance
          >
          > to_play_with_acc;
    boost::random::mt19937 rng;
    boost::random::lognormal_distribution<float> lnorm(1.f,1.f);

    for(unsigned num = 0; num<size; ++num) {

        to_play_with[num] = lnorm(rng);
        to_play_with_acc(to_play_with[num]);
    }

    sqeazy::histogram<value_type> of_variable(&to_play_with[0], size);

    BOOST_CHECK_NE(boost::accumulators::mean(to_play_with_acc),boost::accumulators::median(to_play_with_acc));
    BOOST_CHECK_NE(of_variable.median(),of_variable.median_variation());
    BOOST_CHECK_NE(of_variable.median(),of_variable.mean());
    BOOST_CHECK_NE(of_variable.mean_variation(),of_variable.median_variation());


}

BOOST_AUTO_TEST_CASE( rounding )
{

    BOOST_CHECK_EQUAL(sqeazy::round<int>(.55),1);
    BOOST_CHECK_EQUAL(sqeazy::round<int>(.05),0);
}

BOOST_AUTO_TEST_CASE( data_of_all_zeroes )
{
    std::fill(constant_cube.begin(), constant_cube.end(), 0);
    sqeazy::histogram<value_type> of_variable(constant_cube.begin(), constant_cube.end());
    typedef typename sqeazy::histogram<value_type>::bins_type h_bins_t;
    typename std::vector<h_bins_t>::const_iterator median_itr = sqeazy::median_index(of_variable.bins.begin(),of_variable.bins.end());
    BOOST_CHECK_EQUAL(median_itr - of_variable.bins.begin(),0);
    BOOST_CHECK_EQUAL(of_variable.smallest_populated_bin(),0);
    BOOST_CHECK_EQUAL(of_variable.largest_populated_bin(),0);
    BOOST_CHECK_EQUAL(of_variable.calc_median(),0);
    BOOST_CHECK_EQUAL(of_variable.calc_mean(),0);
    BOOST_CHECK_EQUAL(of_variable.calc_median_variation(),0.f);
    BOOST_CHECK_EQUAL(of_variable.calc_mean_variation(),0.f);
}


BOOST_AUTO_TEST_CASE( median_variance_vs_boost )
{
    boost::accumulators::accumulator_set<float,
          boost::accumulators::stats<boost::accumulators::tag::mean,
          boost::accumulators::tag::variance,boost::accumulators::tag::moment<2>
          >
          > to_play_with_acc;

    boost::random::mt19937 rng;
    boost::random::normal_distribution<float> norm(128,8);

    for(unsigned num = 0; num<size; ++num) {

        to_play_with[num] = norm(rng);
        to_play_with_acc(to_play_with[num]);

    }
    sqeazy::histogram<value_type> of_norm(&to_play_with[0], size);
    BOOST_CHECK_CLOSE(of_norm.mean(), 128, 10);
    BOOST_CHECK_CLOSE(of_norm.mean(), boost::accumulators::mean(to_play_with_acc),1);
    BOOST_CHECK_CLOSE(of_norm.mean_variation(),
                      std::sqrt(boost::accumulators::variance(to_play_with_acc)),1e-1);

}

BOOST_AUTO_TEST_CASE( entropy )
{
    boost::accumulators::accumulator_set<float,
          boost::accumulators::stats<boost::accumulators::tag::mean,
          boost::accumulators::tag::variance,boost::accumulators::tag::moment<2>
          >
          > to_play_with_acc;


    for(unsigned num = 0; num<size; ++num) {

        to_play_with[num] = (num % 2 == 0) ? 1 : 0;
	incrementing_cube[num] = (num) % 4;

    }
    sqeazy::histogram<value_type> of_norm(&to_play_with[0], size);
    BOOST_CHECK_EQUAL(of_norm.calc_entropy(), 1);
sqeazy::histogram<value_type> of_inc(incrementing_cube.begin(), incrementing_cube.end());
    BOOST_CHECK_EQUAL(of_inc.calc_entropy(), 2);
}
BOOST_AUTO_TEST_SUITE_END()
