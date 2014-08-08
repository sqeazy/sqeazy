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
  
  
  std::vector<long> dims(3,uint16_cube_of_8::axis_length);
  size_t face_size = uint16_cube_of_8::axis_length*uint16_cube_of_8::axis_length;
  std::vector<unsigned short> face(face_size);
  std::fill(face.begin(), face.end(), 0);
  
  sqeazy::remove_background<unsigned short>::extract_darkest_face(constant_cube, dims, face);
  
  BOOST_CHECK_EQUAL(face.size(),face_size);
  BOOST_CHECK_EQUAL_COLLECTIONS(face.begin(),face.end(), constant_cube, &constant_cube[0] + face.size());

}

BOOST_AUTO_TEST_CASE( selects_correct_plane_in_z )
{
  
  
  std::vector<long> dims(3,uint16_cube_of_8::axis_length);
  size_t face_size = uint16_cube_of_8::axis_length*uint16_cube_of_8::axis_length;
  std::vector<unsigned short> face(face_size);
  std::fill(face.begin(), face.end(), 0);
  
  sqeazy::remove_background<unsigned short>::extract_darkest_face(incrementing_cube, dims, face);
  
  BOOST_CHECK_EQUAL(face.size(),face_size);
  BOOST_CHECK_EQUAL_COLLECTIONS(face.begin(),face.end(), incrementing_cube, &incrementing_cube[0] + face.size());

}


BOOST_AUTO_TEST_SUITE_END()
