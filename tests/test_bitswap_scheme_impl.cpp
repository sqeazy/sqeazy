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

BOOST_FIXTURE_TEST_SUITE( shift_signed_bits, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( rotate_left )
{
  unsigned char test_unsigned_char = 23;
  unsigned char result_char = sqeazy::rotate_left<1>(test_unsigned_char);
  BOOST_CHECK_EQUAL(46,result_char);

  unsigned short test_unsigned_short = 23;
  unsigned short result = sqeazy::rotate_left<1>(test_unsigned_short);
  BOOST_CHECK_EQUAL(46,result);

  short test_signed_short = 23;
  short result_signed = sqeazy::rotate_left<1>(test_signed_short);
  BOOST_CHECK_EQUAL(46,result_signed);

}

BOOST_AUTO_TEST_CASE( rotate_right )
{
  unsigned char test_unsigned_char = 23;
  unsigned char result_char = sqeazy::rotate_right<1>(test_unsigned_char);
  BOOST_CHECK_EQUAL(139,result_char);

  unsigned short test_unsigned_short = 23;
  unsigned short result = sqeazy::rotate_right<1>(test_unsigned_short);
  BOOST_CHECK_EQUAL(32779,result);

  short test_signed_short = 23;
  short result_signed = sqeazy::rotate_right<1>(test_signed_short);
  BOOST_CHECK_EQUAL(-32757,result_signed);
  unsigned short result_casted = static_cast<unsigned short>(result_signed);
  BOOST_CHECK_EQUAL(result_casted,result);
}

BOOST_AUTO_TEST_CASE( rotate_is_injective )
{
  unsigned char test_unsigned_char = 23;
  unsigned char result_unsigned_char = 
sqeazy::rotate_left<1>(sqeazy::rotate_right<1>(test_unsigned_char));
  BOOST_CHECK_EQUAL(test_unsigned_char,result_unsigned_char);
  result_unsigned_char = 
sqeazy::rotate_right<1>(sqeazy::rotate_left<1>(test_unsigned_char));
  BOOST_CHECK_EQUAL(test_unsigned_char,result_unsigned_char);

  unsigned short test_unsigned_short = 23;
  unsigned short result_unsigned_short = 
sqeazy::rotate_left<1>(sqeazy::rotate_right<1>(test_unsigned_short));
  BOOST_CHECK_EQUAL(test_unsigned_short,result_unsigned_short);
  result_unsigned_short = 
sqeazy::rotate_right<1>(sqeazy::rotate_left<1>(test_unsigned_short));
  BOOST_CHECK_EQUAL(test_unsigned_short,result_unsigned_short);

  short test_short = 23;
  
BOOST_CHECK_EQUAL(test_short,sqeazy::rotate_left<1>(sqeazy::rotate_right<1>(
test_short)));
  short result_short = sqeazy::rotate_right<1>(test_short);
  result_short = sqeazy::rotate_left<1>(result_short);
  BOOST_CHECK_EQUAL(test_short,result_short);

  result_short = sqeazy::rotate_right<1>(sqeazy::rotate_left<1>(test_short));
  BOOST_CHECK_EQUAL(test_short,result_short);

  test_short = -23;
  
BOOST_CHECK_EQUAL(test_short,sqeazy::rotate_left<1>(sqeazy::rotate_right<1>(
test_short)));
  result_short = sqeazy::rotate_right<1>(test_short);
  result_short = sqeazy::rotate_left<1>(result_short);
  BOOST_CHECK_EQUAL(test_short,result_short);

  result_short = sqeazy::rotate_right<1>(sqeazy::rotate_left<1>(test_short));
  BOOST_CHECK_EQUAL(test_short,result_short);


}

BOOST_AUTO_TEST_CASE( offset_called )
{
  unsigned short test_unsigned = 42;
  short test_signed = -42;

  unsigned short result_unsigned = sqeazy::xor_if_signed(test_unsigned);
  short result_signed = sqeazy::xor_if_signed(test_signed);
  
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
      
      BOOST_CHECK_NE(test_signed[i], xor_if_signed(test_signed[i]));
      BOOST_CHECK_EQUAL(xor_if_signed(test_signed[i]), test_signed[i] ^ short(~(1 << 15)));
      
    }
    else
      BOOST_CHECK_EQUAL(test_signed[i], xor_if_signed(test_signed[i]));

    

    short intermediate = xor_if_signed(test_signed[i]);

    intermediate = sqeazy::rotate_left<1>(intermediate);
    
    expected = xor_if_signed(test_signed[i]);
    BOOST_CHECK_EQUAL(expected, sqeazy::rotate_right<1>(intermediate));
    intermediate = sqeazy::rotate_right<1>(intermediate);

    short result =  xor_if_signed(intermediate);

    BOOST_CHECK_EQUAL(test_signed[i], result);
  }

}

BOOST_AUTO_TEST_CASE( encoding_decoding_injective_on_unsigned )
{
  using namespace sqeazy;

  const unsigned short test_signed[8] = {42,16,11,4,1,0,3,5};

  for(int i = 0;i<8;++i){
    
    unsigned short intermediate =  
sqeazy::rotate_left<1>(xor_if_signed(test_signed[i]));

    unsigned short result =  
xor_if_signed(sqeazy::rotate_right<1>(intermediate));

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


BOOST_FIXTURE_TEST_SUITE( requirements_upon_bitset, xor_expected_results )

BOOST_AUTO_TEST_CASE( apply_xor )
{
  using namespace sqeazy;

  
  const short test_signed[16] = {7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6, 
-7, -8};

  for(int i = 0;i<16;++i){
    
    short result =  sqeazy::xor_if_signed(test_signed[i]);
    std::bitset<16> result_repr(result);
    short expected = expected_bit_map[test_signed[i]].to_ulong();
        
    BOOST_CHECK_MESSAGE(result == expected,  
                              "unable to treat " <<  test_signed[i] <<  ",  received: " 
                              << result << "("<< result_repr.to_string() 
                              << ") - expected: " <<  expected <<  "(" << expected_bit_map[test_signed[i]].to_string() <<")" );
  }

}

BOOST_AUTO_TEST_SUITE_END()
