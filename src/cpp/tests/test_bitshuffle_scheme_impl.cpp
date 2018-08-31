#define BOOST_TEST_MODULE TEST_BITSHUFFLE_SCHEME_IMPL
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"
#include <cstdint>
#include <iterator>
#include <numeric>
#include <vector>
#include <iostream>
#include <bitset>
#include <map>
#include "array_fixtures.hpp"
#include "encoders/bitshuffle_scheme_impl.hpp"

typedef sqeazy::array_fixture<std::uint16_t> uint16_cube_of_8;
typedef sqeazy::array_fixture<std::uint8_t> uint8_cube_of_8;


BOOST_FIXTURE_TEST_SUITE( encode_decode_loop_16bit, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( roundtrip )
{

  std::vector<std::size_t> shape(3,1);
  shape.front() = 32 * (1 << 10);

  constant_cube.resize(shape.front());
  std::iota(constant_cube.begin(),constant_cube.end(),0);
  to_play_with.resize(constant_cube.size());

  sqeazy::bitshuffle_scheme<std::uint16_t> shuffle;
  auto end = shuffle.encode(&constant_cube[0], &to_play_with[0],shape);


  BOOST_CHECK(end != nullptr);

  auto items_processed = std::distance(static_cast<const std::uint16_t*>(to_play_with.data()),
                                       static_cast<const std::uint16_t*>(end));
  BOOST_CHECK_NE(items_processed,0);
  BOOST_CHECK_EQUAL(items_processed,to_play_with.size());

  auto decoded = constant_cube;
  std::fill(decoded.begin(), decoded.end(),0);
  auto dec = shuffle.decode(to_play_with.data(), decoded.data(),shape);

  BOOST_CHECK(dec == 0);
  BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(),constant_cube.end(),
                                decoded.begin(),decoded.end());

}

BOOST_AUTO_TEST_CASE( roundtrip_odd )
{

  std::vector<std::size_t> shape(3,1);
  shape.front() = 32 * (1 << 10);
  shape.front() += 1;

  constant_cube.resize(shape.front());
  std::iota(constant_cube.begin(),constant_cube.end(),0);
  to_play_with.resize(constant_cube.size());

  sqeazy::bitshuffle_scheme<std::uint16_t> shuffle;
  auto end = shuffle.encode(&constant_cube[0], &to_play_with[0],shape);


  BOOST_CHECK(end != nullptr);

  auto items_processed = std::distance(static_cast<const std::uint16_t*>(to_play_with.data()),
                                       static_cast<const std::uint16_t*>(end));
  BOOST_CHECK_NE(items_processed,0);
  BOOST_CHECK_EQUAL(items_processed,to_play_with.size());

  auto decoded = constant_cube;
  std::fill(decoded.begin(), decoded.end(),0);
  auto dec = shuffle.decode(to_play_with.data(), decoded.data(),shape);

  BOOST_CHECK(dec == 0);
  BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(),constant_cube.end(),
                                decoded.begin(),decoded.end());

}

BOOST_AUTO_TEST_CASE( roundtrip_prime )
{

  std::vector<std::size_t> shape(3,1);
  shape.front() = 32003;

  constant_cube.resize(shape.front());
  std::iota(constant_cube.begin(),constant_cube.end(),0);
  to_play_with.resize(constant_cube.size());

  sqeazy::bitshuffle_scheme<std::uint16_t> shuffle;
  auto end = shuffle.encode(&constant_cube[0], &to_play_with[0],shape);


  BOOST_CHECK(end != nullptr);

  auto items_processed = std::distance(static_cast<const std::uint16_t*>(to_play_with.data()),
                                       static_cast<const std::uint16_t*>(end));
  BOOST_CHECK_NE(items_processed,0);
  BOOST_CHECK_EQUAL(items_processed,to_play_with.size());

  auto decoded = constant_cube;
  std::fill(decoded.begin(), decoded.end(),0);
  auto dec = shuffle.decode(to_play_with.data(), decoded.data(),shape);

  BOOST_CHECK(dec == 0);
  BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(),constant_cube.end(),
                                decoded.begin(),decoded.end());

}

BOOST_AUTO_TEST_CASE( roundtrip_two_threads )
{

  std::vector<std::size_t> shape(3,1);
  shape.front() = 32 * (1 << 10);

  constant_cube.resize(shape.front());
  std::iota(constant_cube.begin(),constant_cube.end(),0);
  to_play_with.resize(constant_cube.size());

  sqeazy::bitshuffle_scheme<std::uint16_t> shuffle;shuffle.set_n_threads(2);
  auto end = shuffle.encode(&constant_cube[0], &to_play_with[0],shape);


  BOOST_CHECK(end != nullptr);

  auto items_processed = std::distance(static_cast<const std::uint16_t*>(to_play_with.data()),
                                       static_cast<const std::uint16_t*>(end));
  BOOST_CHECK_NE(items_processed,0);
  BOOST_CHECK_EQUAL(items_processed,to_play_with.size());

  auto decoded = constant_cube;
  std::fill(decoded.begin(), decoded.end(),0);
  auto dec = shuffle.decode(to_play_with.data(), decoded.data(),shape);

  BOOST_CHECK(dec == 0);
  BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(),constant_cube.end(),
                                decoded.begin(),decoded.end());

}

BOOST_AUTO_TEST_CASE( roundtrip_odd_2threads )
{

  std::vector<std::size_t> shape(3,1);
  shape.front() = 32 * (1 << 10);
  shape.front() += 1;

  constant_cube.resize(shape.front());
  std::iota(constant_cube.begin(),constant_cube.end(),0);
  to_play_with.resize(constant_cube.size());

  sqeazy::bitshuffle_scheme<std::uint16_t> shuffle;shuffle.set_n_threads(2);
  auto end = shuffle.encode(&constant_cube[0], &to_play_with[0],shape);


  BOOST_CHECK(end != nullptr);

  auto items_processed = std::distance(static_cast<const std::uint16_t*>(to_play_with.data()),
                                       static_cast<const std::uint16_t*>(end));
  BOOST_CHECK_NE(items_processed,0);
  BOOST_CHECK_EQUAL(items_processed,to_play_with.size());

  auto decoded = constant_cube;
  std::fill(decoded.begin(), decoded.end(),0);
  auto dec = shuffle.decode(to_play_with.data(), decoded.data(),shape);

  BOOST_CHECK(dec == 0);
  BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(),constant_cube.end(),
                                decoded.begin(),decoded.end());

}

BOOST_AUTO_TEST_CASE( roundtrip_prime_two_threads )
{

  std::vector<std::size_t> shape(3,1);
  shape.front() = 32003;

  constant_cube.resize(shape.front());
  std::iota(constant_cube.begin(),constant_cube.end(),0);
  to_play_with.resize(constant_cube.size());

  sqeazy::bitshuffle_scheme<std::uint16_t> shuffle;shuffle.set_n_threads(2);
  auto end = shuffle.encode(&constant_cube[0], &to_play_with[0],shape);


  BOOST_CHECK(end != nullptr);

  auto items_processed = std::distance(static_cast<const std::uint16_t*>(to_play_with.data()),
                                       static_cast<const std::uint16_t*>(end));
  BOOST_CHECK_NE(items_processed,0);
  BOOST_CHECK_EQUAL(items_processed,to_play_with.size());

  auto decoded = constant_cube;
  std::fill(decoded.begin(), decoded.end(),0);
  auto dec = shuffle.decode(to_play_with.data(), decoded.data(),shape);

  BOOST_CHECK(dec == 0);
  BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(),constant_cube.end(),
                                decoded.begin(),decoded.end());

}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( encode_decode_loop_8bit, uint8_cube_of_8 )



BOOST_AUTO_TEST_CASE( roundtrip )
{

  std::vector<std::size_t> shape(3,1);
  shape.front() = 32 * (1 << 10);

  constant_cube.resize(shape.front());
  std::iota(constant_cube.begin(),constant_cube.end(),0);
  to_play_with.resize(constant_cube.size());

  sqeazy::bitshuffle_scheme<std::uint8_t> shuffle;
  auto end = shuffle.encode(&constant_cube[0], &to_play_with[0],shape);


  BOOST_CHECK(end != nullptr);

  auto items_processed = std::distance(static_cast<const std::uint8_t*>(to_play_with.data()),
                                       static_cast<const std::uint8_t*>(end));
  BOOST_CHECK_NE(items_processed,0);
  BOOST_CHECK_EQUAL(items_processed,to_play_with.size());

  auto decoded = constant_cube;
  std::fill(decoded.begin(), decoded.end(),0);
  auto dec = shuffle.decode(to_play_with.data(), decoded.data(),shape);

  BOOST_CHECK(dec == 0);
  BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(),constant_cube.end(),
                                decoded.begin(),decoded.end());

}

BOOST_AUTO_TEST_CASE( roundtrip_odd )
{

  std::vector<std::size_t> shape(3,1);
  shape.front() = 32 * (1 << 10);
  shape.front() += 1;

  constant_cube.resize(shape.front());
  std::iota(constant_cube.begin(),constant_cube.end(),0);
  to_play_with.resize(constant_cube.size());

  sqeazy::bitshuffle_scheme<std::uint8_t> shuffle;
  auto end = shuffle.encode(&constant_cube[0], &to_play_with[0],shape);


  BOOST_CHECK(end != nullptr);

  auto items_processed = std::distance(static_cast<const std::uint8_t*>(to_play_with.data()),
                                       static_cast<const std::uint8_t*>(end));
  BOOST_CHECK_NE(items_processed,0);
  BOOST_CHECK_EQUAL(items_processed,to_play_with.size());

  auto decoded = constant_cube;
  std::fill(decoded.begin(), decoded.end(),0);
  auto dec = shuffle.decode(to_play_with.data(), decoded.data(),shape);

  BOOST_CHECK(dec == 0);
  BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(),constant_cube.end(),
                                decoded.begin(),decoded.end());

}

BOOST_AUTO_TEST_CASE( roundtrip_prime )
{

  std::vector<std::size_t> shape(3,1);
  shape.front() = 32003;

  constant_cube.resize(shape.front());
  std::iota(constant_cube.begin(),constant_cube.end(),0);
  to_play_with.resize(constant_cube.size());

  sqeazy::bitshuffle_scheme<std::uint8_t> shuffle;
  auto end = shuffle.encode(&constant_cube[0], &to_play_with[0],shape);


  BOOST_CHECK(end != nullptr);

  auto items_processed = std::distance(static_cast<const std::uint8_t*>(to_play_with.data()),
                                       static_cast<const std::uint8_t*>(end));
  BOOST_CHECK_NE(items_processed,0);
  BOOST_CHECK_EQUAL(items_processed,to_play_with.size());

  auto decoded = constant_cube;
  std::fill(decoded.begin(), decoded.end(),0);
  auto dec = shuffle.decode(to_play_with.data(), decoded.data(),shape);

  BOOST_CHECK(dec == 0);
  BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(),constant_cube.end(),
                                decoded.begin(),decoded.end());

}

BOOST_AUTO_TEST_CASE( roundtrip_two_threads )
{

  std::vector<std::size_t> shape(3,1);
  shape.front() = 32 * (1 << 10);

  constant_cube.resize(shape.front());
  std::iota(constant_cube.begin(),constant_cube.end(),0);
  to_play_with.resize(constant_cube.size());

  sqeazy::bitshuffle_scheme<std::uint8_t> shuffle;shuffle.set_n_threads(2);
  auto end = shuffle.encode(&constant_cube[0], &to_play_with[0],shape);


  BOOST_CHECK(end != nullptr);

  auto items_processed = std::distance(static_cast<const std::uint8_t*>(to_play_with.data()),
                                       static_cast<const std::uint8_t*>(end));
  BOOST_CHECK_NE(items_processed,0);
  BOOST_CHECK_EQUAL(items_processed,to_play_with.size());

  auto decoded = constant_cube;
  std::fill(decoded.begin(), decoded.end(),0);
  auto dec = shuffle.decode(to_play_with.data(), decoded.data(),shape);

  BOOST_CHECK(dec == 0);
  BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(),constant_cube.end(),
                                decoded.begin(),decoded.end());

}

BOOST_AUTO_TEST_CASE( roundtrip_odd_2threads )
{

  std::vector<std::size_t> shape(3,1);
  shape.front() = 32 * (1 << 10);
  shape.front() += 1;

  constant_cube.resize(shape.front());
  std::iota(constant_cube.begin(),constant_cube.end(),0);
  to_play_with.resize(constant_cube.size());

  sqeazy::bitshuffle_scheme<std::uint8_t> shuffle;shuffle.set_n_threads(2);
  auto end = shuffle.encode(&constant_cube[0], &to_play_with[0],shape);


  BOOST_CHECK(end != nullptr);

  auto items_processed = std::distance(static_cast<const std::uint8_t*>(to_play_with.data()),
                                       static_cast<const std::uint8_t*>(end));
  BOOST_CHECK_NE(items_processed,0);
  BOOST_CHECK_EQUAL(items_processed,to_play_with.size());

  auto decoded = constant_cube;
  std::fill(decoded.begin(), decoded.end(),0);
  auto dec = shuffle.decode(to_play_with.data(), decoded.data(),shape);

  BOOST_CHECK(dec == 0);
  BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(),constant_cube.end(),
                                decoded.begin(),decoded.end());

}

BOOST_AUTO_TEST_CASE( roundtrip_prime_two_threads )
{

  std::vector<std::size_t> shape(3,1);
  shape.front() = 32003;

  constant_cube.resize(shape.front());
  std::iota(constant_cube.begin(),constant_cube.end(),0);
  to_play_with.resize(constant_cube.size());

  sqeazy::bitshuffle_scheme<std::uint8_t> shuffle;shuffle.set_n_threads(2);
  auto end = shuffle.encode(&constant_cube[0], &to_play_with[0],shape);


  BOOST_CHECK(end != nullptr);

  auto items_processed = std::distance(static_cast<const std::uint8_t*>(to_play_with.data()),
                                       static_cast<const std::uint8_t*>(end));
  BOOST_CHECK_NE(items_processed,0);
  BOOST_CHECK_EQUAL(items_processed,to_play_with.size());

  auto decoded = constant_cube;
  std::fill(decoded.begin(), decoded.end(),0);
  auto dec = shuffle.decode(to_play_with.data(), decoded.data(),shape);

  BOOST_CHECK(dec == 0);
  BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(),constant_cube.end(),
                                decoded.begin(),decoded.end());

}

BOOST_AUTO_TEST_SUITE_END()
