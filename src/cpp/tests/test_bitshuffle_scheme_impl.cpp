#define BOOST_TEST_MODULE TEST_BITSHUFFLE_SCHEME_IMPL
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"
#include <cstdint>
#include <numeric>
#include <vector>
#include <iostream>
#include <bitset>
#include <map>
#include "array_fixtures.hpp"
#include "encoders/bitshuffle_scheme_impl.hpp"

typedef sqeazy::array_fixture<std::uint16_t> uint16_cube_of_8;
typedef sqeazy::array_fixture<std::uint8_t> uint8_cube_of_8;


struct incrementing_array
{
  std::vector<std::uint16_t> input;
  std::vector<std::uint16_t> plane1_encoded_by_hand;
  std::vector<std::uint16_t> plane2_encoded_by_hand;
  std::vector<std::uint16_t> plane4_encoded_by_hand;
  std::vector<std::uint16_t> output;

  static constexpr std::size_t len  = 16;

  incrementing_array():
    input(len,0),
    plane1_encoded_by_hand(len,0),
    plane2_encoded_by_hand(len,0),
    plane4_encoded_by_hand(len,0),
    output(len,0)
  {

    for (std::size_t i = 0; i < len; ++i)
      {
    input[i] = i;
      }

    plane1_encoded_by_hand[12] = 0xff;// = 255
    plane1_encoded_by_hand[13] = 0xf0f;// = 3855
    plane1_encoded_by_hand[14] = 0x3333;// = 13107
    plane1_encoded_by_hand[15] = 0x5555;// = 21845

  }


};

BOOST_FIXTURE_TEST_SUITE( encode_decode_loop_16bit, incrementing_array )

BOOST_AUTO_TEST_CASE( encoded_equals_by_hand )
{

  std::vector<std::size_t> shape(3,1);
  shape.front() = input.size();
  sqeazy::bitshuffle_scheme<std::uint16_t> swap_it;
  auto end = swap_it.encode(&input[0], &output[0],shape);


  BOOST_CHECK(end != nullptr);
  for(unsigned i = 0;i<input.size();++i){

    BOOST_CHECK_MESSAGE(output[i] == plane4_encoded_by_hand[i],
            "bswap4_scheme::encode input["<< i <<"] = " <<  input[i]
            <<  ",  output = " << output[i]
            <<  ",  by_hand = " << plane4_encoded_by_hand[i] );
  }

}

// BOOST_AUTO_TEST_CASE( decode_encoded_by_hand_planewidth4_new_api )
// {
//   std::vector<std::size_t> shape(3,1);
//   shape.front() = input.size();
//   bswap4_scheme swap_it;
//   int rv = swap_it.decode(&plane4_encoded_by_hand[0], &output[0],shape);


//   BOOST_CHECK(rv == 0);
//   for(unsigned i = 0;i<input.size();++i){

//     BOOST_CHECK_MESSAGE(output[i] == input[i],
//             "bswap4_scheme::decode input["<< i <<"] = " <<  plane4_encoded_by_hand[i]
//             <<  ",  output = " << output[i]
//             <<  ",  expected = " << input[i] );
//   }

// }


BOOST_AUTO_TEST_SUITE_END()
