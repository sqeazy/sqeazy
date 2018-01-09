#define BOOST_TEST_MODULE TEST_LZ4_SCHEMES
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include "array_fixtures.hpp"
#include "encoders/lz4.hpp"

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;


BOOST_FIXTURE_TEST_SUITE( simple, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( encodes )
{
  std::vector<std::size_t> shape(dims.begin(), dims.end());
  // short* output = reinterpret_cast<short*>(&to_play_with[0]);
  sqeazy::lz4_scheme<value_type> local;
  auto res = local.encode(&incrementing_cube[0],
                          (char*)to_play_with.data(),
                          shape);

  BOOST_CHECK_NE(res,(char*)to_play_with.data());

}

BOOST_AUTO_TEST_SUITE_END()
