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


BOOST_AUTO_TEST_SUITE_END()
