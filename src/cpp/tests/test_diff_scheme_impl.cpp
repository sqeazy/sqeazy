#define BOOST_TEST_MODULE TEST_DIFF_SCHEMES
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include "array_fixtures.hpp"
#include "../src/sqeazy_impl.hpp"
#include "../src/neighborhood_utils.hpp"

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;
typedef sqeazy::diff_scheme<unsigned short> local_diff_scheme ;


BOOST_FIXTURE_TEST_SUITE( offset_calculation, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( offset_called )
{
    const unsigned axis_size = axis_length;
    std::vector<unsigned> offsets;

    sqeazy::halo<sqeazy::last_pixels_in_cube_neighborhood<3> , unsigned> geometry(axis_size,axis_size,axis_size);
    geometry.compute_offsets_in_x(offsets);

    BOOST_CHECK_GT(offsets.size(),0);
    unsigned expected = (axis_size-1)*(axis_size-1);
    BOOST_CHECK_EQUAL(offsets.size(),expected);
}


BOOST_AUTO_TEST_CASE( offset_exact )
{
    const unsigned axis_size = axis_length;
    std::vector<unsigned> offsets;

    sqeazy::halo<sqeazy::last_pixels_in_cube_neighborhood<3> , unsigned> geometry(axis_size,axis_size,axis_size);
    geometry.compute_offsets_in_x(offsets);

    for(int dim_idx = 0; dim_idx<3; ++dim_idx) {
        BOOST_CHECK_EQUAL(geometry.non_halo_begin(dim_idx),1);
        BOOST_CHECK_EQUAL(geometry.non_halo_end(dim_idx),axis_size);
    }

    BOOST_CHECK_EQUAL(offsets.at(0),axis_size*axis_size + axis_size + 1);
    BOOST_CHECK_EQUAL(offsets.at(1),axis_size*axis_size + 2*axis_size + 1);
    BOOST_CHECK_EQUAL(offsets.back(),(axis_size-1)*axis_size*axis_size + ((axis_size-1))*axis_size + 1);
}

BOOST_AUTO_TEST_CASE( offset_exact_last_plane )
{
    const unsigned axis_size = axis_length;
    std::vector<unsigned> offsets;

    sqeazy::halo<sqeazy::last_plane_neighborhood<3> , unsigned> geometry(axis_size,axis_size,axis_size);
    geometry.compute_offsets_in_x(offsets);

    for(int dim_idx = 0; dim_idx<2; ++dim_idx) {
        try {
            BOOST_REQUIRE_EQUAL(geometry.non_halo_begin(dim_idx),1);
        }
        catch(...) {
            std::cerr << "offset_exact_last_plane failed!\n"
                      << "non_halo_begin(" << dim_idx << "): \t " << geometry.non_halo_begin(dim_idx)
                      << ",\texpected " << 1 << "\n";
        }

        try {
            BOOST_REQUIRE_EQUAL(geometry.non_halo_end(dim_idx),axis_size-1);
        }
        catch(...) {
            std::cerr << "offset_exact_last_plane failed!\n"
                      << "non_halo_end(" << dim_idx << "): \t " << geometry.non_halo_end(dim_idx)
                      << ",\texpected " << axis_size-1 << "\n";
        }
    }

    BOOST_CHECK_EQUAL(geometry.non_halo_begin(2),1);
    BOOST_CHECK_EQUAL(geometry.non_halo_end(2),axis_size);


    BOOST_CHECK_EQUAL(offsets.at(0),axis_size*axis_size + axis_size + 1);
    BOOST_CHECK_EQUAL(offsets.size(),(axis_size-1)*(axis_size-2));
    BOOST_CHECK_EQUAL(offsets.at(1),axis_size*axis_size + 2*axis_size + 1);
    BOOST_CHECK_EQUAL(offsets.back(),(axis_size-1)*axis_size*axis_size + ((axis_size-1-1))*axis_size + 1);

}
BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( offset_calculation, uint16_cube_of_8 )
BOOST_AUTO_TEST_CASE( Neighborhood_size )
{
//   sqeazy::last_plane_neighborhood<3> local;
    unsigned last_plane_traversed_pixels = sqeazy::num_traversed_pixels<sqeazy::last_plane_neighborhood<3> >();
    BOOST_CHECK_GT(last_plane_traversed_pixels,0);
    BOOST_CHECK_EQUAL(last_plane_traversed_pixels,9);
    last_plane_traversed_pixels = sqeazy::num_traversed_pixels<sqeazy::last_plane_neighborhood<3> >();
    BOOST_CHECK_EQUAL(last_plane_traversed_pixels,9);

    unsigned last_in_cube_traversed_pixels = sqeazy::num_traversed_pixels<sqeazy::last_pixels_in_cube_neighborhood<3> >();
    BOOST_CHECK_GT(last_in_cube_traversed_pixels,0);
    BOOST_CHECK_EQUAL(last_in_cube_traversed_pixels,8);

}

BOOST_AUTO_TEST_CASE( last_pixels_Neighborhood_size )
{
    typedef sqeazy::last_pixels_on_line_neighborhood<> last_pixels_nb;
    unsigned num_traversed = sqeazy::num_traversed_pixels<last_pixels_nb>();
    BOOST_CHECK_GT(num_traversed,0);
    BOOST_CHECK_EQUAL(num_traversed,8);
    BOOST_CHECK_EQUAL(sqeazy::offset_begin_on_axis<last_pixels_nb>(0),-8);
    BOOST_CHECK_EQUAL(sqeazy::offset_end_on_axis<last_pixels_nb>(0),0);

    std::vector<unsigned> offsets;
    sqeazy::halo<last_pixels_nb, unsigned> geometry(dims.begin(), dims.end());
    geometry.compute_offsets_in_x(offsets);
    const unsigned len = size;

    BOOST_CHECK_LT(*std::max_element(offsets.begin(), offsets.end()),len);
    BOOST_CHECK_GE(*std::min_element(offsets.begin(), offsets.end()),0);

    BOOST_CHECK_GE(geometry.non_halo_begin(0),0);
    BOOST_CHECK_EQUAL(geometry.non_halo_end(0),8);
    BOOST_CHECK_EQUAL(offsets.size(),1);

}
BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( apply_diff, uint16_cube_of_8 )
BOOST_AUTO_TEST_CASE( diff_it )
{

    short* output = reinterpret_cast<short*>(&to_play_with[0]);
    sqeazy::diff_scheme<value_type, sqeazy::last_pixels_on_line_neighborhood<> >::encode(
        &incrementing_cube[0],
        output,
        dims);

    BOOST_CHECK_EQUAL(to_play_with[12],to_play_with[13]);


}
BOOST_AUTO_TEST_SUITE_END()
