#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TEST_HIST_IMPL
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <cmath>
#include <vector>
#include <iostream>
#include "array_fixtures.hpp"
#include "../src/hist_impl.hpp"

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

    unsigned value_index = 0;
    unsigned idx = 0;
    for(; value_index<std::numeric_limits<value_type>::max(); ++value_index) {
        for(unsigned num = 0; num<value_index; ++num) {
            if(idx<size)
                to_play_with[idx] = value_index;
	    ++idx;
        }
	if(idx>=size)
	  break;
    }

sqeazy::histogram<value_type> of_variable(&to_play_with[0], size);
    unsigned char exp_median = value_index/std::sqrt(2);
    BOOST_CHECK_NE(of_variable.calc_mean(),of_variable.calc_median());
    BOOST_CHECK_EQUAL(of_variable.calc_median(),exp_median);

}
BOOST_AUTO_TEST_SUITE_END()
