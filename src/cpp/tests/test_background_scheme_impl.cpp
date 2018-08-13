#define BOOST_TEST_MODULE TEST_BACKGROUND_SCHEME_IMPL
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include <bitset>
#include <map>
#include "array_fixtures.hpp"
#include "encoders/sqeazy_impl.hpp"
#include "encoders/flatten_to_neighborhood_scheme_impl.hpp"
#include "encoders/remove_estimated_background_scheme_impl.hpp"

#include "boost/random.hpp"

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;


BOOST_FIXTURE_TEST_SUITE( extract_face, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( success_with_supports )
{
    size_t face_size = uint16_cube_of_8::axis_length*uint16_cube_of_8::axis_length;
    std::vector<unsigned short> face(face_size);
    std::fill(face.begin(), face.end(), 0);
    const value_type* input = &constant_cube[0];
    std::vector<std::uint32_t> shape = dims;
    auto supports = sqeazy::extract_darkest_face_supports(input, shape);

    BOOST_CHECK_EQUAL(supports.empty(),false);

    for( float val : supports ){
        BOOST_CHECK_CLOSE(val, constant_cube.front(),.01f);
    }
}

BOOST_AUTO_TEST_CASE( success_with_supports_2threads )
{
    size_t face_size = uint16_cube_of_8::axis_length*uint16_cube_of_8::axis_length;
    std::vector<unsigned short> face(face_size);
    std::fill(face.begin(), face.end(), 0);
    const value_type* input = &constant_cube[0];
    std::vector<std::uint32_t> shape = dims;
    auto supports = sqeazy::extract_darkest_face_supports(input, shape, 0.99f, 2);

    BOOST_CHECK_EQUAL(supports.empty(),false);

    for( float val : supports ){
        BOOST_CHECK_CLOSE(val, constant_cube.front(),.01f);
    }
}

BOOST_AUTO_TEST_CASE( success )
{

    size_t face_size = uint16_cube_of_8::axis_length*uint16_cube_of_8::axis_length;
    std::vector<unsigned short> face(face_size);
    std::fill(face.begin(), face.end(), 0);
    const value_type* input = &constant_cube[0];
    sqeazy::extract_darkest_face(input, dims, face);

    BOOST_CHECK_EQUAL(face.size(),face_size);
    BOOST_CHECK_EQUAL_COLLECTIONS(face.begin(),face.end(), constant_cube.begin(),constant_cube.begin() + face_size);

}

BOOST_AUTO_TEST_CASE( selects_correct_plane_in_z )
{

  std::fill(constant_cube.begin(), constant_cube.end(), 1024);

    size_t face_size = uint16_cube_of_8::axis_length*uint16_cube_of_8::axis_length;
    boost::random::mt19937 rng;
    boost::random::normal_distribution<float> front_data(128,8);
    boost::random::normal_distribution<float> back_data(128+64,8);

    for(unsigned num = 0; num < face_size; ++num) {
        constant_cube[num] = front_data(rng);
    constant_cube[size-face_size+num] = back_data(rng);
    }

    std::vector<unsigned short> face(face_size);
    std::fill(face.begin(), face.end(), 0);
    const value_type* input = &constant_cube[0];
    sqeazy::extract_darkest_face(input, dims, face);

    sqeazy::histogram<value_type> faceh(face.begin(), face.end());

    BOOST_CHECK_EQUAL(face.size(),face_size);
    BOOST_CHECK_CLOSE(faceh.mean(), 128,3.f);

}

BOOST_AUTO_TEST_CASE( supports_correct_in_z )
{

    std::fill(constant_cube.begin(), constant_cube.end(), 1024);

    size_t face_size = uint16_cube_of_8::axis_length*uint16_cube_of_8::axis_length;
    boost::random::mt19937 rng;
    boost::random::normal_distribution<float> front_data(128,8);
    boost::random::normal_distribution<float> back_data (192,8);

    for(unsigned num = 0; num < face_size; ++num) {
        constant_cube[num] = front_data(rng);
        constant_cube[size-face_size+num] = back_data(rng);
    }
    auto supports = sqeazy::extract_darkest_face_supports(constant_cube.data(), dims, 0.95f);

    BOOST_CHECK_CLOSE(supports.at(0), 128+16, 5);
    BOOST_CHECK_CLOSE(supports.at(1), 192+16, 5);

    supports = sqeazy::extract_darkest_face_supports(constant_cube.data(), dims);

    BOOST_CHECK_CLOSE(supports.at(3), 1024,3.f);
    BOOST_CHECK_CLOSE(supports.at(2), 1024,3.f);

    BOOST_CHECK_GT(supports.at(0), 128+1*8);
    BOOST_CHECK_LT(supports.at(0), 128+5*8);

    BOOST_CHECK_GT(supports.at(1), 192+1*8);
    BOOST_CHECK_LT(supports.at(1), 192+5*8);

}

BOOST_AUTO_TEST_CASE( supports_correct_in_z_2threads )
{

    std::fill(constant_cube.begin(), constant_cube.end(), 1024);

    size_t face_size = uint16_cube_of_8::axis_length*uint16_cube_of_8::axis_length;
    boost::random::mt19937 rng;
    boost::random::normal_distribution<float> front_data(128,8);
    boost::random::normal_distribution<float> back_data (192,8);

    for(unsigned num = 0; num < face_size; ++num) {
        constant_cube[num] = front_data(rng);
        constant_cube[size-face_size+num] = back_data(rng);
    }

    auto supports = sqeazy::extract_darkest_face_supports(constant_cube.data(), dims, 0.99f, 2);

    BOOST_CHECK_CLOSE(supports.at(3), 1024,3.f);
    BOOST_CHECK_CLOSE(supports.at(2), 1024,3.f);

    BOOST_CHECK_GT(supports.at(0), 128+1*8);
    BOOST_CHECK_LT(supports.at(0), 128+5*8);

    BOOST_CHECK_GT(supports.at(1), 192+1*8);
    BOOST_CHECK_LT(supports.at(1), 192+5*8);

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


}

BOOST_AUTO_TEST_CASE( stamp_removal_newapi )
{

    std::fill(constant_cube.begin(), constant_cube.end(), 0);
    constant_cube[constant_cube.size()/2] = 1 << 14;
    float input_sum = std::accumulate(constant_cube.begin(), constant_cube.end(),0);

    typedef sqeazy::cube_neighborhood<3> nb_t;

    for(int i = 0; i<3; ++i) {
        BOOST_CHECK_EQUAL(sqeazy::offset_begin_on_axis<nb_t>(i), -1);
        BOOST_CHECK_EQUAL(sqeazy::offset_end_on_axis<nb_t>(i), 2);
    }

    std::vector<std::size_t> shape(dims.begin(), dims.end());
    sqeazy::flatten_to_neighborhood_scheme<value_type> flatten(42);
    auto end = flatten.encode(&constant_cube[0],
                  &to_play_with[0],
                  shape);

    BOOST_CHECK(end!=nullptr);

    float sum = std::accumulate(to_play_with.begin(), to_play_with.end(),0);

    BOOST_CHECK_EQUAL(sum, 0);
    BOOST_CHECK_NE(sum, input_sum);

}


BOOST_AUTO_TEST_CASE( stamp_removal_fraction_newapi )
{

    std::fill(constant_cube.begin(), constant_cube.end(), 0);
    std::fill(incrementing_cube.begin(), incrementing_cube.end(), 0);

    unsigned central_index = 3*axis_length*axis_length + 3*axis_length +3;
    constant_cube[central_index] = 1 << 14;
    incrementing_cube[central_index] = 1 << 14;
    incrementing_cube[central_index - 1] = 1 << 14;
    incrementing_cube[central_index + 1] = 1 << 14;
    incrementing_cube[central_index - 8] = 1 << 14;
    incrementing_cube[central_index + 8] = 1 << 14;
    incrementing_cube[central_index - 64] = 1 << 14;
    incrementing_cube[central_index + 64] = 1 << 14;

    std::vector<std::size_t> shape(dims.begin(), dims.end());
    sqeazy::flatten_to_neighborhood_scheme<value_type> fraction_default(42);
    auto end = fraction_default.encode(&constant_cube[0],
                       &to_play_with[0],
                       shape);

    BOOST_CHECK(end!=nullptr);
    BOOST_CHECK_EQUAL(to_play_with[central_index], 0);

    sqeazy::flatten_to_neighborhood_scheme<value_type,sqeazy::cube_neighborhood<3> > fraction_a(42,6/(26.f));

    end = fraction_a.encode(&incrementing_cube[0],
                &to_play_with[0],
                shape);

    BOOST_CHECK(end!=nullptr);
    BOOST_CHECK_EQUAL(to_play_with[central_index], 0);

    sqeazy::flatten_to_neighborhood_scheme<value_type,sqeazy::cube_neighborhood<3> > fraction_b(42, 22/(26.f));
    end = fraction_b.encode(&incrementing_cube[0],
              &to_play_with[0],
              shape);

    BOOST_CHECK(end!=nullptr);
    BOOST_CHECK_MESSAGE(to_play_with[central_index] != 0, "flatten_to_neighborhood did not keep intensity of interest");


}

BOOST_AUTO_TEST_SUITE_END()
