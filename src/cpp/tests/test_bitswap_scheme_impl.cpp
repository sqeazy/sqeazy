#define BOOST_TEST_MODULE TEST_BITSWAP_SCHEME_IMPL
#include "boost/test/unit_test.hpp"
#include <cstdint>
#include <numeric>
#include <vector>
#include <iostream>
#include <bitset>
#include <map>
#include "array_fixtures.hpp"
#include "encoders/bitswap_scheme_impl.hpp"

typedef sqeazy::array_fixture<std::uint16_t> uint16_cube_of_8;
typedef sqeazy::array_fixture<std::uint8_t> uint8_cube_of_8;

typedef sqeazy::bitswap_scheme<std::uint8_t> bswap1_scheme_uint8;
typedef sqeazy::bitswap_scheme<std::uint16_t> bswap1_scheme;
typedef sqeazy::bitswap_scheme<std::uint16_t,2> bswap2_scheme;
typedef sqeazy::bitswap_scheme<std::uint16_t,4> bswap4_scheme;

BOOST_FIXTURE_TEST_SUITE( shift_signed_bits_16bit, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( rotate_left )
{
  unsigned char test_unsigned_char = 23;
  unsigned char result_char = sqeazy::detail::rotate_left<1>(test_unsigned_char);
  BOOST_CHECK_EQUAL(46,result_char);

  std::uint16_t test_unsigned_short = 23;
  std::uint16_t result = sqeazy::detail::rotate_left<1>(test_unsigned_short);
  BOOST_CHECK_EQUAL(46,result);

  short test_signed_short = 23;
  short result_signed = sqeazy::detail::rotate_left<1>(test_signed_short);
  BOOST_CHECK_EQUAL(46,result_signed);

}

BOOST_AUTO_TEST_CASE( rotate_right )
{
  unsigned char test_unsigned_char = 23;
  unsigned char result_char = sqeazy::detail::rotate_right<1>(test_unsigned_char);
  BOOST_CHECK_EQUAL(139,result_char);

  std::uint16_t test_unsigned_short = 23;
  std::uint16_t result = sqeazy::detail::rotate_right<1>(test_unsigned_short);
  BOOST_CHECK_EQUAL(32779,result);

  short test_signed_short = 23;
  short result_signed = sqeazy::detail::rotate_right<1>(test_signed_short);
  BOOST_CHECK_EQUAL(-32757,result_signed);
  std::uint16_t result_casted = static_cast<std::uint16_t>(result_signed);
  BOOST_CHECK_EQUAL(result_casted,result);
}

BOOST_AUTO_TEST_CASE( rotate_is_injective )
{
  unsigned char test_unsigned_char = 23;
  unsigned char result_unsigned_char = 
sqeazy::detail::rotate_left<1>(sqeazy::detail::rotate_right<1>(test_unsigned_char));
  BOOST_CHECK_EQUAL(test_unsigned_char,result_unsigned_char);
  result_unsigned_char = 
sqeazy::detail::rotate_right<1>(sqeazy::detail::rotate_left<1>(test_unsigned_char));
  BOOST_CHECK_EQUAL(test_unsigned_char,result_unsigned_char);

  std::uint16_t test_unsigned_short = 23;
  std::uint16_t result_unsigned_short = 
sqeazy::detail::rotate_left<1>(sqeazy::detail::rotate_right<1>(test_unsigned_short));
  BOOST_CHECK_EQUAL(test_unsigned_short,result_unsigned_short);
  result_unsigned_short = 
sqeazy::detail::rotate_right<1>(sqeazy::detail::rotate_left<1>(test_unsigned_short));
  BOOST_CHECK_EQUAL(test_unsigned_short,result_unsigned_short);

  short test_short = 23;
  
BOOST_CHECK_EQUAL(test_short,sqeazy::detail::rotate_left<1>(sqeazy::detail::rotate_right<1>(
test_short)));
  short result_short = sqeazy::detail::rotate_right<1>(test_short);
  result_short = sqeazy::detail::rotate_left<1>(result_short);
  BOOST_CHECK_EQUAL(test_short,result_short);

  result_short = sqeazy::detail::rotate_right<1>(sqeazy::detail::rotate_left<1>(test_short));
  BOOST_CHECK_EQUAL(test_short,result_short);

  test_short = -23;
  
BOOST_CHECK_EQUAL(test_short,sqeazy::detail::rotate_left<1>(sqeazy::detail::rotate_right<1>(
test_short)));
  result_short = sqeazy::detail::rotate_right<1>(test_short);
  result_short = sqeazy::detail::rotate_left<1>(result_short);
  BOOST_CHECK_EQUAL(test_short,result_short);

  result_short = sqeazy::detail::rotate_right<1>(sqeazy::detail::rotate_left<1>(test_short));
  BOOST_CHECK_EQUAL(test_short,result_short);


}

BOOST_AUTO_TEST_CASE( offset_called )
{
  std::uint16_t test_unsigned = 42;
  short test_signed = -42;

  std::uint16_t result_unsigned = sqeazy::detail::xor_if_signed(test_unsigned);
  short result_signed = sqeazy::detail::xor_if_signed(test_signed);
  
  BOOST_CHECK_EQUAL(result_unsigned,test_unsigned);
  BOOST_CHECK_EQUAL(result_signed,-32727);

}

BOOST_AUTO_TEST_CASE( encoding_decoding_injective_on_signed )
{
  using namespace sqeazy;

  const short test_signed[8] = {-42,-16,-11,-4,-1,0,2,4};
  short expected = 0;
  
  for(int i = 0;i<8;++i){
    
    if(test_signed[i]<0){
      
      BOOST_CHECK_NE(test_signed[i], sqeazy::detail::xor_if_signed(test_signed[i]));
      BOOST_CHECK_EQUAL(sqeazy::detail::xor_if_signed(test_signed[i]), test_signed[i] ^ short(~(1 << 15)));
      
    }
    else
      BOOST_CHECK_EQUAL(test_signed[i], sqeazy::detail::xor_if_signed(test_signed[i]));

    

    short intermediate = sqeazy::detail::xor_if_signed(test_signed[i]);

    intermediate = sqeazy::detail::rotate_left<1>(intermediate);
    
    expected = sqeazy::detail::xor_if_signed(test_signed[i]);
    BOOST_CHECK_EQUAL(expected, sqeazy::detail::rotate_right<1>(intermediate));
    intermediate = sqeazy::detail::rotate_right<1>(intermediate);

    short result =  sqeazy::detail::xor_if_signed(intermediate);

    BOOST_CHECK_EQUAL(test_signed[i], result);
  }

}

BOOST_AUTO_TEST_CASE( encoding_decoding_injective_on_unsigned )
{
  using namespace sqeazy;

  const std::uint16_t test_signed[8] = {42,16,11,4,1,0,3,5};

  for(int i = 0;i<8;++i){
    
    std::uint16_t intermediate =  
sqeazy::detail::rotate_left<1>(sqeazy::detail::xor_if_signed(test_signed[i]));

    std::uint16_t result =  
      sqeazy::detail::xor_if_signed(sqeazy::detail::rotate_right<1>(intermediate));

    BOOST_CHECK_EQUAL(test_signed[i], result);
  }

}



BOOST_AUTO_TEST_SUITE_END()

struct xor_expected_results {
  
//   Good catch Peter, there is indeed need for an additional twist, you are 
// right, 
// the ordering of negative numbers in 2s complement
// is inverted: -1d ~ 11111111b. But, there is still a sign bit: it's the most 
// significant bit:
// 
// see this table from wikipedia for 4 bit integers:
// 0111    7 
// 0110    6 
// 0101    5 
// 0100    4 
// 0011    3 
// 0010    2 
// 0001    1 
// 0000    0 
// 1111    −1 
// 1110    −2 
// 1101    −3 
// 1100    −4 
// 1011    −5 
// 1010    −6 
// 1001    −7 
// 1000    −8 
// 
// Now you are right that there is a problem with the fact that -1 has a lot of 'ones'
// in its representation, that very much the opposite of what we want.
// It actually could very well explain why we did not get a bit more compression than expected!
// 
// We would want instead to have an encoding more along the lines of:
// 
// 0111    7 
// 0110    6 
// 0101    5 
// 0100    4 
// 0011    3 
// 0010    2 
// 0001    1 
// 0000    0 
// 1000    −1 
// 1001    −2 
// 1010    −3 
// 1011    −4 
// 1100    −5 
// 1101    −6 
// 1110    −7 
// 1111    −8 
// 
// 
// So the simple trick I propose is to xor all lowly significant bits:
// For 16bit integer x:
// 
// if sign bit of x is 1:
//         return x^01111111b
// else return x
// 
// This converts the standard two-complemet representation to a representation
// where we keep the sign bit but inverts the encoding of the negative numbers.
  
  
  std::map<short,  std::bitset<16> > expected_bit_map;
  std::bitset<16> mask;
  
  xor_expected_results() :
    expected_bit_map(), 
    mask(~(1 << (sizeof(short)*8 - 1))){
      
        expected_bit_map[7] = std::bitset<16> ( std::string ( "0111" ) );
        expected_bit_map[6] = std::bitset<16> ( std::string ( "0110" ) );
        expected_bit_map[5] = std::bitset<16> ( std::string ( "0101" ) );
        expected_bit_map[4] = std::bitset<16> ( std::string ( "0100" ) );
        expected_bit_map[3] = std::bitset<16> ( std::string ( "0011" ) );
        expected_bit_map[2] = std::bitset<16> ( std::string ( "0010" ) );
        expected_bit_map[1] = std::bitset<16> ( std::string ( "0001" ) );
        expected_bit_map[0] = std::bitset<16> ( std::string ( "0000" ) );
        expected_bit_map[-1] = std::bitset< 16 > ( std::string ( "1000000000000000" ) );
        expected_bit_map[-2] = std::bitset< 16 > ( std::string ( "1000000000000001" ) );
        expected_bit_map[-3] = std::bitset< 16 > ( std::string ( "1000000000000010" ) );
        expected_bit_map[-4] = std::bitset< 16 > ( std::string ( "1000000000000011" ) );
        expected_bit_map[-5] = std::bitset< 16 > ( std::string ( "1000000000000100" ) );
        expected_bit_map[-6] = std::bitset< 16 > ( std::string ( "1000000000000101" ) );
        expected_bit_map[-7] = std::bitset< 16 > ( std::string ( "1000000000000110" ) );
        expected_bit_map[-8] = std::bitset< 16 > ( std::string ( "1000000000000111" ) );
//       
    }
};


BOOST_FIXTURE_TEST_SUITE( requirements_upon_bitset_16bit, xor_expected_results )

BOOST_AUTO_TEST_CASE( apply_xor )
{
  using namespace sqeazy;

  
  const short test_signed[16] = {7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6, 
-7, -8};

  for(int i = 0;i<16;++i){
    
    short result =  sqeazy::detail::xor_if_signed(test_signed[i]);
    std::bitset<16> result_repr(result);
    short expected = expected_bit_map[test_signed[i]].to_ulong();
        
    BOOST_CHECK_MESSAGE(result == expected,  
                              "unable to treat " <<  test_signed[i] <<  ",  received: " 
                              << result << "("<< result_repr.to_string() 
                              << ") - expected: " <<  expected <<  "(" << expected_bit_map[test_signed[i]].to_string() <<")" );
  }

}

BOOST_AUTO_TEST_SUITE_END()


struct incrementing_array
{
  std::vector<std::uint16_t> input;
  std::vector<std::uint16_t> plane1_encoded_by_hand;
  std::vector<std::uint16_t> plane2_encoded_by_hand;
  std::vector<std::uint16_t> plane4_encoded_by_hand;
  std::vector<std::uint16_t> output;

  incrementing_array():
    input(16,0),
    plane1_encoded_by_hand(16,0),
    plane2_encoded_by_hand(16,0),
    plane4_encoded_by_hand(16,0),
    output(16,0)
  {

    for (int i = 0; i < 16; ++i)
      {
	input[i] = i;
      }

    plane1_encoded_by_hand[11] = 0xff;// = 255
    plane1_encoded_by_hand[12] = 0xf0f;// = 3855
    plane1_encoded_by_hand[13] = 0x3333;// = 13107
    plane1_encoded_by_hand[14] = 0x5555;// = 21845

    //bswap2
    plane2_encoded_by_hand[11] = 0x5555;
    plane2_encoded_by_hand[12] = 0x05af;
    plane2_encoded_by_hand[13] = plane2_encoded_by_hand[12];
    plane2_encoded_by_hand[14] = 0x2222;
    plane2_encoded_by_hand[15] = plane2_encoded_by_hand[14];

    //bswap4
    plane4_encoded_by_hand[10] = 0x1111;
    plane4_encoded_by_hand[11] = 0x1111;
    plane4_encoded_by_hand[12] = 0x0246;
    plane4_encoded_by_hand[13] = 0x8ace;
    plane4_encoded_by_hand[14] = 0x0246;
    plane4_encoded_by_hand[15] = 0x8ace;

  }

  
};

BOOST_FIXTURE_TEST_SUITE( encode_decode_loop_16bit, incrementing_array )

BOOST_AUTO_TEST_CASE( setbits_on_integertype )
{
  std::uint16_t zero = 0;
  std::uint16_t one = 1;
  BOOST_CHECK(sqeazy::detail::setbits_of_integertype(zero, one, 5u, 1u) == 1 << 5);

  std::uint16_t max_char = 0xff;
  BOOST_CHECK(sqeazy::detail::setbits_of_integertype(max_char, one, 10u, 1u) == (max_char + (1 << 10)));

  BOOST_CHECK(sqeazy::detail::setbits_of_integertype(max_char, zero, 4u, 4u) == 0xf);
  
  std::uint16_t three = 3;
  //three is truncated if it maps to more than 16 bits (here)
  BOOST_CHECK(sqeazy::detail::setbits_of_integertype(zero, three, 15u, 2u) == 0x8000);
}


BOOST_AUTO_TEST_CASE( encoded_equals_by_hand_planewidth1 )
{

  int rv = bswap1_scheme::static_encode(&input[0], &output[0],input.size());


  BOOST_CHECK(rv == 0);
  for(unsigned i = 0;i<input.size();++i){
        
    BOOST_CHECK_MESSAGE(output[i] == plane1_encoded_by_hand[i],  
			"bswap1_scheme::static_encode input["<< i <<"] = " <<  input[i] 
			<<  ",  output = " << output[i]
			<<  ",  by_hand = " << plane1_encoded_by_hand[i] );
  }

}

BOOST_AUTO_TEST_CASE( decode_encoded_by_hand_planewidth1 )
{

  int rv = bswap1_scheme::static_decode(&plane1_encoded_by_hand[0], &output[0],input.size());


  BOOST_CHECK(rv == 0);
  for(unsigned i = 0;i<input.size();++i){
        
    BOOST_CHECK_MESSAGE(output[i] == input[i],  
			"bswap1_scheme::static_decode input["<< i <<"] = " <<  plane1_encoded_by_hand[i] 
			<<  ",  output = " << output[i]
			<<  ",  expected = " << input[i] );
  }

}

BOOST_AUTO_TEST_CASE( encoded_equals_by_hand_planewidth2 )
{

  int rv = bswap2_scheme::static_encode(&input[0], &output[0],input.size());


  BOOST_CHECK(rv == 0);
  for(unsigned i = 0;i<input.size();++i){
        
    BOOST_CHECK_MESSAGE(output[i] == plane2_encoded_by_hand[i],  
			"bswap2_scheme::static_encode input["<< i <<"] = " <<  input[i] 
			<<  ",  output = " << output[i]
			<<  ",  by_hand = " << plane2_encoded_by_hand[i] );
  }

}

BOOST_AUTO_TEST_CASE( decode_encoded_by_hand_planewidth2 )
{

  int rv = bswap2_scheme::static_decode(&plane2_encoded_by_hand[0], &output[0],input.size());


  BOOST_CHECK(rv == 0);
  for(unsigned i = 0;i<input.size();++i){
        
    BOOST_CHECK_MESSAGE(output[i] == input[i],  
			"bswap2_scheme::static_decode input["<< i <<"] = " <<  plane2_encoded_by_hand[i] 
			<<  ",  output = " << output[i]
			<<  ",  expected = " << input[i] );
  }

}

BOOST_AUTO_TEST_CASE( encoded_equals_by_hand_planewidth4 )
{

  int rv = bswap4_scheme::static_encode(&input[0], &output[0],input.size());


  BOOST_CHECK(rv == 0);
  for(unsigned i = 0;i<input.size();++i){
        
    BOOST_CHECK_MESSAGE(output[i] == plane4_encoded_by_hand[i],  
			"bswap4_scheme::static_encode input["<< i <<"] = " <<  input[i] 
			<<  ",  output = " << output[i]
			<<  ",  by_hand = " << plane4_encoded_by_hand[i] );
  }

}

BOOST_AUTO_TEST_CASE( decode_encoded_by_hand_planewidth4 )
{

  int rv = bswap4_scheme::static_decode(&plane4_encoded_by_hand[0], &output[0],input.size());


  BOOST_CHECK(rv == 0);
  for(unsigned i = 0;i<input.size();++i){
        
    BOOST_CHECK_MESSAGE(output[i] == input[i],  
			"bswap4_scheme::static_decode input["<< i <<"] = " <<  plane4_encoded_by_hand[i] 
			<<  ",  output = " << output[i]
			<<  ",  expected = " << input[i] );
  }

}

BOOST_AUTO_TEST_CASE( encoded_equals_by_hand_planewidth4_new_api )
{

  std::vector<std::size_t> shape(3,1);
  shape.front() = input.size();
  bswap4_scheme swap_it;
  auto end = swap_it.encode(&input[0], &output[0],shape);


  BOOST_CHECK(end != nullptr);
  for(unsigned i = 0;i<input.size();++i){
        
    BOOST_CHECK_MESSAGE(output[i] == plane4_encoded_by_hand[i],  
			"bswap4_scheme::static_encode input["<< i <<"] = " <<  input[i] 
			<<  ",  output = " << output[i]
			<<  ",  by_hand = " << plane4_encoded_by_hand[i] );
  }

}

BOOST_AUTO_TEST_CASE( decode_encoded_by_hand_planewidth4_new_api )
{
  std::vector<std::size_t> shape(3,1);
  shape.front() = input.size();
  bswap4_scheme swap_it;
  int rv = swap_it.decode(&plane4_encoded_by_hand[0], &output[0],shape);


  BOOST_CHECK(rv == 0);
  for(unsigned i = 0;i<input.size();++i){
        
    BOOST_CHECK_MESSAGE(output[i] == input[i],  
			"bswap4_scheme::static_decode input["<< i <<"] = " <<  plane4_encoded_by_hand[i] 
			<<  ",  output = " << output[i]
			<<  ",  expected = " << input[i] );
  }

}


BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_CASE( setbits_on_integertype )
{
  std::uint16_t zero = 0;
  std::uint16_t one = 1;
  BOOST_CHECK(sqeazy::detail::setbits_of_integertype(zero, one, 5u, 1u) == 1 << 5);

  std::uint16_t max_char = 0xff;
  BOOST_CHECK(sqeazy::detail::setbits_of_integertype(max_char, one, 10u, 1u) == (max_char + (1 << 10)));

  BOOST_CHECK(sqeazy::detail::setbits_of_integertype(max_char, zero, 4u, 4u) == 0xf);
  
  std::uint16_t three = 3;
  //three is truncated if it maps to more than 16 bits (here)
  BOOST_CHECK(sqeazy::detail::setbits_of_integertype(zero, three, 15u, 2u) == 0x8000);
}


BOOST_AUTO_TEST_CASE( roundtrip_ramp_8 )
{
  using namespace sqeazy;

  std::vector<unsigned> shape(3);
  shape[0] = 7;
  shape[1] = 9;
  shape[2] = 11;

  const unsigned long flat_size = std::accumulate(shape.begin(), shape.end(),1,std::multiplies<unsigned>());
  std::vector<std::uint8_t> expected(flat_size);
  for(unsigned i = 0;i<flat_size;++i)
    expected[i] = static_cast<std::uint8_t>(i);

  std::vector<std::uint8_t> compressed(expected);

  bswap1_scheme_uint8 encoder;
  std::uint8_t* end = encoder.encode(expected.data(),
				     compressed.data(),
				     flat_size);

  BOOST_CHECK(end!=nullptr);
  size_t n_elements_encoded = end - compressed.data();
  BOOST_CHECK_EQUAL(n_elements_encoded,expected.size());

  std::vector<std::uint8_t> decoded(flat_size,0);

  int err = encoder.decode(&compressed[0],
			   &decoded[0] ,
			   flat_size);

  BOOST_CHECK(!err);
  BOOST_CHECK_EQUAL_COLLECTIONS(decoded.begin(), decoded.end(), expected.begin(), expected.end());
}


//BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE( roundtrip_ramp_16 )
{
  using namespace sqeazy;

  std::vector<unsigned> shape(3);
  shape[0] = 7;
  shape[1] = 9;
  shape[2] = 11;

  const unsigned long flat_size = std::accumulate(shape.begin(), shape.end(),1,std::multiplies<unsigned>());
  std::vector<std::uint16_t> expected(flat_size);
  for(unsigned i = 0;i<flat_size;++i)
    expected[i] = static_cast<std::uint16_t>(i);

  std::vector<std::uint16_t> compressed(expected);
  
  int res = bswap1_scheme::static_encode(&expected[0], &compressed[0],flat_size);

  BOOST_CHECK(!res);

  std::vector<std::uint16_t> decoded(flat_size,0);

  res = bswap1_scheme::static_decode(&compressed[0], &decoded[0] ,flat_size);

  BOOST_CHECK(!res);
  BOOST_CHECK_EQUAL_COLLECTIONS(decoded.begin(), decoded.end(), expected.begin(), expected.end());
}
