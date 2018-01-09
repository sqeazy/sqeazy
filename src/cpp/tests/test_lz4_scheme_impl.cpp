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

  sqeazy::lz4_scheme<value_type> local;
  auto expected_encoded_bytes = local.max_encoded_size(size_in_byte);
  std::vector<char> encoded(expected_encoded_bytes);

  auto res = local.encode(&incrementing_cube[0],
                          encoded.data(),
                          shape);

  BOOST_REQUIRE_NE(res,(char*)nullptr);
  BOOST_CHECK_NE(res,encoded.data());

}

BOOST_AUTO_TEST_CASE( decodes )
{
    std::vector<std::size_t> shape(dims.begin(), dims.end());

    sqeazy::lz4_scheme<value_type> local;
    auto expected_encoded_bytes = local.max_encoded_size(size_in_byte);
    std::vector<char> encoded(expected_encoded_bytes);

    auto res = local.encode(&incrementing_cube[0],
                            encoded.data(),
                            shape);

    BOOST_REQUIRE_NE(res,(char*)nullptr);

    auto rcode = local.decode(encoded.data(),
                              to_play_with.data(),
                              encoded.size(),
                              size_in_byte);

    BOOST_REQUIRE_NE(rcode,1);

}


BOOST_AUTO_TEST_CASE( roundtrip )
{
    std::vector<std::size_t> shape(dims.begin(), dims.end());

    sqeazy::lz4_scheme<value_type> local;
    auto expected_encoded_bytes = local.max_encoded_size(size_in_byte);
    std::vector<char> encoded(expected_encoded_bytes);

    auto res = local.encode(&incrementing_cube[0],
                            encoded.data(),
                            shape);

    BOOST_REQUIRE_NE(res,(char*)nullptr);

    auto rcode = local.decode(encoded.data(),
                              to_play_with.data(),
                              encoded.size(),
                              size_in_byte);

    BOOST_REQUIRE_NE(rcode,1);
    BOOST_REQUIRE_EQUAL_COLLECTIONS(
        incrementing_cube.begin(),
        incrementing_cube.end(),
        to_play_with.begin(),
        to_play_with.end()
        );
}

BOOST_AUTO_TEST_SUITE_END()
