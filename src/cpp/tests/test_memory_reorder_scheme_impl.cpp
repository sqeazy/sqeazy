#define BOOST_TEST_MODULE TEST_MEMORY_REORDER_SCHEME_IMPL
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include <bitset>
#include <map>
#include "array_fixtures.hpp"
#include "encoders/memory_reorder_utils.hpp"

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;

namespace sqy = sqeazy;
namespace sqyd = sqy::detail;

BOOST_FIXTURE_TEST_SUITE( basic , uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( constructs )
{

  sqyd::reorder in_tiles_of(4);
  BOOST_CHECK(in_tiles_of.tile_size == 4);


}

BOOST_AUTO_TEST_CASE( has_0_remainder )
{

  sqyd::reorder in_tiles_of(4);
  auto rem = in_tiles_of.remainder(dims);
  auto expected = dims;
  std::fill(expected.begin(), expected.end(),0);
  
  BOOST_CHECK(!rem.empty());
  BOOST_CHECK_EQUAL_COLLECTIONS(rem.begin(), rem.end(),
				expected.begin(), expected.end());


}

BOOST_AUTO_TEST_CASE( has_non0_remainder )
{

  sqyd::reorder in_tiles_of(6);
  auto rem = in_tiles_of.remainder(dims);
  auto expected = dims;
  std::fill(expected.begin(), expected.end(),2);
  
  BOOST_CHECK(!rem.empty());
  BOOST_CHECK_EQUAL_COLLECTIONS(rem.begin(), rem.end(),
				expected.begin(), expected.end());


}

BOOST_AUTO_TEST_CASE( on_const_value_buffer )
{

  sqyd::reorder in_tiles_of(2);
  auto rem = in_tiles_of.encode(constant_cube.cbegin(), constant_cube.cend(),
			       to_play_with.begin(),
			       dims);
  BOOST_REQUIRE(rem == to_play_with.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(), constant_cube.end(),
				to_play_with.begin(), to_play_with.end()); 


}
BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( on_ramp_forward , uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( tile_of_2 )
{

  sqyd::reorder in_tiles_of(2);
  auto rem = in_tiles_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
			       to_play_with.begin(),
			       dims);
  BOOST_REQUIRE(rem == to_play_with.end());

  std::vector<std::uint16_t> expected = {0,1,
					 8,9,
					 64,65,
					 72,73};
  
  BOOST_CHECK_EQUAL_COLLECTIONS(to_play_with.begin(), to_play_with.begin()+expected.size(),
				expected.begin(), expected.end()); 
  

}

BOOST_AUTO_TEST_CASE( tile_of_4 )
{

  sqyd::reorder in_tiles_of(4);
  auto rem = in_tiles_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
			       to_play_with.begin(),
			       dims);
  BOOST_REQUIRE(rem == to_play_with.end());

  std::vector<std::uint16_t> expected = {0,1,2,3,
					 8,9,10,11,
					 16,17,18,19,
					 24,25,26,27,
					 64,65,66,67,
					 72,73,74,75,
					 80,81,82,83,
					 88,89,90,91
  };
  
  BOOST_CHECK_EQUAL_COLLECTIONS(to_play_with.begin(), to_play_with.begin()+expected.size(),
				expected.begin(), expected.end()); 
  

}

BOOST_AUTO_TEST_CASE( tile_of_3 )
{

  sqyd::reorder in_tiles_of(3);
  auto rem = in_tiles_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
			       to_play_with.begin(),
			       dims);
  BOOST_REQUIRE(rem == to_play_with.end());

  std::vector<std::uint16_t> expected = {0,1,2,
					 8,9,10,
					 16,17,18,
					 24,25,26,
					 64,65,66,
					 72,73,74};
  
  BOOST_CHECK_EQUAL_COLLECTIONS(to_play_with.begin(), to_play_with.begin()+expected.size(),
				expected.begin(), expected.end()); 
  

}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( on_ramp_roundtrip , uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( tile_of_2 )
{

  sqyd::reorder in_tiles_of(2);
  auto rem = in_tiles_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
				to_play_with.begin(),
				dims);
  BOOST_REQUIRE(rem == to_play_with.end());

  auto decoded = constant_cube;
  std::fill(decoded.begin(), decoded.end(),0);

  rem = in_tiles_of.decode(to_play_with.cbegin(),to_play_with.cend(),
			   decoded.begin(),
			   dims);
  BOOST_REQUIRE(rem == decoded.end());  
  BOOST_CHECK_EQUAL_COLLECTIONS(decoded.begin(), decoded.end(),
				decoded.begin(), decoded.end()); 
  

}

BOOST_AUTO_TEST_CASE( tile_of_4 )
{

  sqyd::reorder in_tiles_of(4);
  auto rem = in_tiles_of.encode(incrementing_cube.cbegin(), incrementing_cube.cend(),
				to_play_with.begin(),
				dims);
  BOOST_REQUIRE(rem == to_play_with.end());

  auto decoded = constant_cube;
  std::fill(decoded.begin(), decoded.end(),0);

  rem = in_tiles_of.decode(to_play_with.cbegin(),to_play_with.cend(),
			   decoded.begin(),
			   dims);
  BOOST_REQUIRE(rem == decoded.end());  
  BOOST_CHECK_EQUAL_COLLECTIONS(decoded.begin(), decoded.end(),
				decoded.begin(), decoded.end()); 
  

}


BOOST_AUTO_TEST_SUITE_END()
