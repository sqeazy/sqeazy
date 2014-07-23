#define BOOST_TEST_DYN_LINK 
#define BOOST_TEST_MODULE TEST_DIFF_SCHEMES
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
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
  unsigned char result_unsigned_char = sqeazy::rotate_left<1>(sqeazy::rotate_right<1>(test_unsigned_char));
  BOOST_CHECK_EQUAL(test_unsigned_char,result_unsigned_char);
  result_unsigned_char = sqeazy::rotate_right<1>(sqeazy::rotate_left<1>(test_unsigned_char));
  BOOST_CHECK_EQUAL(test_unsigned_char,result_unsigned_char);

  unsigned short test_unsigned_short = 23;
  unsigned short result_unsigned_short = sqeazy::rotate_left<1>(sqeazy::rotate_right<1>(test_unsigned_short));
  BOOST_CHECK_EQUAL(test_unsigned_short,result_unsigned_short);
  result_unsigned_short = sqeazy::rotate_right<1>(sqeazy::rotate_left<1>(test_unsigned_short));
  BOOST_CHECK_EQUAL(test_unsigned_short,result_unsigned_short);

  short test_short = 23;
  BOOST_CHECK_EQUAL(test_short,sqeazy::rotate_left<1>(sqeazy::rotate_right<1>(test_short)));
  short result_short = sqeazy::rotate_right<1>(test_short);
  result_short = sqeazy::rotate_left<1>(result_short);
  BOOST_CHECK_EQUAL(test_short,result_short);

  result_short = sqeazy::rotate_right<1>(sqeazy::rotate_left<1>(test_short));
  BOOST_CHECK_EQUAL(test_short,result_short);

  test_short = -23;
  BOOST_CHECK_EQUAL(test_short,sqeazy::rotate_left<1>(sqeazy::rotate_right<1>(test_short)));
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
    
    BOOST_CHECK_NE(test_signed[i], xor_if_signed(test_signed[i]));

    BOOST_CHECK_EQUAL(xor_if_signed(test_signed[i]), test_signed[i] ^ short(~(1 << 15)));

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
    
    unsigned short intermediate =  sqeazy::rotate_left<1>(xor_if_signed(test_signed[i]));

    unsigned short result =  xor_if_signed(sqeazy::rotate_right<1>(intermediate));

    BOOST_CHECK_EQUAL(test_signed[i], result);
  }

}

BOOST_AUTO_TEST_SUITE_END()
