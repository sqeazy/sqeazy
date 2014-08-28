#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TEST_DIFF_SCHEMES
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include <bitset>
#include <map>
#include "array_fixtures.hpp"
#include "../src/sqeazy_impl.hpp"

#include "boost/random.hpp"

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;

BOOST_FIXTURE_TEST_SUITE( extract_face, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( success )
{



    size_t face_size = uint16_cube_of_8::axis_length*uint16_cube_of_8::axis_length;
    std::vector<unsigned short> face(face_size);
    std::fill(face.begin(), face.end(), 0);
    const value_type* input = &constant_cube[0];
    sqeazy::remove_estimated_background<unsigned short>::extract_darkest_face(input, dims, face);

    BOOST_CHECK_EQUAL(face.size(),face_size);
    BOOST_CHECK_EQUAL_COLLECTIONS(face.begin(),face.end(), constant_cube.begin(),constant_cube.begin() + face_size);

}

BOOST_AUTO_TEST_CASE( selects_correct_plane_in_z )
{



    size_t face_size = uint16_cube_of_8::axis_length*uint16_cube_of_8::axis_length;
    std::vector<unsigned short> face(face_size);
    std::fill(face.begin(), face.end(), 0);
    const value_type* input = &incrementing_cube[0];
    sqeazy::remove_estimated_background<unsigned short>::extract_darkest_face(input, dims, face);

    BOOST_CHECK_EQUAL(face.size(),face_size);
    BOOST_CHECK_EQUAL_COLLECTIONS(face.begin(),face.end(), incrementing_cube.begin(), incrementing_cube.begin() + face_size);

}

BOOST_AUTO_TEST_CASE( crops_correct_values )
{
    boost::random::mt19937 rng;
    boost::random::normal_distribution<float> norm(16,8);

    unsigned current_value = 0;
    unsigned index = 0;
    while(index < size) {
        for(unsigned num = index; num < current_value || num < size; ++num) {
            to_play_with[num] = current_value;
        }
        index += current_value;
        ++current_value;
    }


    float result = sqeazy::mad(to_play_with.begin(), to_play_with.end());
    BOOST_CHECK_GT(result,0);
    sqeazy::histogram<value_type> h_to_play_with(to_play_with.begin(), to_play_with.end());
    BOOST_CHECK_LT(result,h_to_play_with.median());
    BOOST_CHECK_NE(result,h_to_play_with.median_variation());
//     std::cout << "mad : " << result << "\n"
//               << sqeazy::histogram<value_type>::print_header() << "\n"
//               << h_to_play_with << "\n";

}

BOOST_AUTO_TEST_CASE( free_mean_var )
{
    boost::random::mt19937 rng;
    boost::random::normal_distribution<float> norm(32,4);

    for(unsigned num = 0; num < size; ++num) {
        to_play_with[num] = norm(rng);
    }

    float mean = 0;
    float var = 0;
    sqeazy::remove_estimated_background<value_type>::mean_and_var(to_play_with.begin(),
            to_play_with.end(),
            mean,
            var

                                                                 );

    BOOST_CHECK_CLOSE(mean,32.f,2);
    BOOST_CHECK_CLOSE(var,4.f,4);



}

BOOST_AUTO_TEST_SUITE_END()
