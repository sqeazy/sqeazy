#define BOOST_TEST_MODULE TEST_STRING_BASE64_ENCODING
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"
#include <numeric>
#include <functional>
#include <cmath>
#include <vector>
#include <map>
#include <chrono>
#include <cstdint>
#include <sstream>
#include "base64.hpp"

namespace sqy = sqeazy;

struct string_fixture
{
  const std::string standard_text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque enim sapien, dignissim nec interdum id, dignissim nec felis. Nulla facilisi. Sed vel enim a diam aliquam euismod et ut lorem. Aliquam felis felis, ultricies interdum sapien nec, consectetur gravida est. Nulla facilisi. Sed et hendrerit sapien. Donec id pellentesque orci, ut accumsan erat. Cras euismod id risus a faucibus. Duis in neque et elit iaculis finibus ac vel urna. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; In nec nibh quis turpis luctus bibendum tempus vel nisl. Nullam massa mi, semper eu tellus eget, iaculis interdum erat. Curabitur sit amet molestie mauris. Integer molestie fringilla nisi, in tristique metus placerat eu. Nam vitae tristique leo. In nunc tortor, imperdiet vitae pharetra iaculis, pulvinar malesuada ex.";
  const std::string wikipedia1 = "any carnal pleas";
  const std::string wikipedia2 = "any carnal pleasu";
  const std::string wikipedia3 = "any carnal pleasur";

  const std::string wikipedia1_encoded = "YW55IGNhcm5hbCBwbGVhcw==";
  const std::string wikipedia2_encoded = "YW55IGNhcm5hbCBwbGVhc3U=";
  const std::string wikipedia3_encoded = "YW55IGNhcm5hbCBwbGVhc3Vy";
};

BOOST_FIXTURE_TEST_SUITE( minimal, string_fixture )

BOOST_AUTO_TEST_CASE (encoded_nbytes) {

  BOOST_CHECK_NE(   sqy::base64::encoded_bytes(wikipedia1.size()),0);
  BOOST_CHECK_NE(   sqy::base64::encoded_bytes(wikipedia1.size()),wikipedia1.size());
  BOOST_CHECK_EQUAL(sqy::base64::encoded_bytes(wikipedia1.size()),wikipedia1_encoded.size());

  BOOST_CHECK_NE(   sqy::base64::encoded_bytes(wikipedia2.size()),0);
  BOOST_CHECK_NE(   sqy::base64::encoded_bytes(wikipedia2.size()),wikipedia2.size());
  BOOST_CHECK_EQUAL(sqy::base64::encoded_bytes(wikipedia2.size()),wikipedia2_encoded.size());

  BOOST_CHECK_NE(   sqy::base64::encoded_bytes(wikipedia3.size()),0);
  BOOST_CHECK_NE(   sqy::base64::encoded_bytes(wikipedia3.size()),wikipedia3.size());
  BOOST_CHECK_EQUAL(sqy::base64::encoded_bytes(wikipedia3.size()),wikipedia3_encoded.size());
}

BOOST_AUTO_TEST_CASE (decoded_nbytes) {

  BOOST_CHECK_NE(   sqy::base64::decoded_bytes(wikipedia1_encoded.data(),wikipedia1_encoded.data() + wikipedia1_encoded.size()),0);
  BOOST_CHECK_NE(   sqy::base64::decoded_bytes(wikipedia1_encoded.data(),wikipedia1_encoded.data() + wikipedia1_encoded.size()),wikipedia1_encoded.size());
  BOOST_CHECK_EQUAL(sqy::base64::decoded_bytes(wikipedia1_encoded.data(),wikipedia1_encoded.data() + wikipedia1_encoded.size()),wikipedia1.size());


  BOOST_CHECK_NE(   sqy::base64::decoded_bytes(wikipedia2_encoded.data(),wikipedia2_encoded.data() + wikipedia2_encoded.size()),0);
  BOOST_CHECK_NE(   sqy::base64::decoded_bytes(wikipedia2_encoded.data(),wikipedia2_encoded.data() + wikipedia2_encoded.size()),wikipedia2_encoded.size());
  BOOST_CHECK_EQUAL(sqy::base64::decoded_bytes(wikipedia2_encoded.data(),wikipedia2_encoded.data() + wikipedia2_encoded.size()),wikipedia2.size());


  BOOST_CHECK_NE(   sqy::base64::decoded_bytes(wikipedia3_encoded.data(),wikipedia3_encoded.data() + wikipedia3_encoded.size()),0);
  BOOST_CHECK_NE(   sqy::base64::decoded_bytes(wikipedia3_encoded.data(),wikipedia3_encoded.data() + wikipedia3_encoded.size()),wikipedia3_encoded.size());
  BOOST_CHECK_EQUAL(sqy::base64::decoded_bytes(wikipedia3_encoded.data(),wikipedia3_encoded.data() + wikipedia3_encoded.size()),wikipedia3.size());

}


BOOST_AUTO_TEST_CASE (changes_string) {

  std::string result = wikipedia1;
  result.resize(sqy::base64::encoded_bytes(result.size()));

  sqy::base64::encode_impl(wikipedia1.data(),
                   wikipedia1.data() + wikipedia1.size(),
                   (char*)result.data());

  BOOST_CHECK_NE(wikipedia1[0],
                 result[0]);


}

BOOST_AUTO_TEST_CASE (encode_return_iterator) {

  std::string result = wikipedia1_encoded;
  std::fill(result.begin(),result.end(),' ');

  auto ritr = sqy::base64::encode_impl(wikipedia1.data(),
                                       wikipedia1.data() + wikipedia1.size(),
                                       (char*)result.data());


  BOOST_CHECK_NE(ritr,
                 result.data());

  BOOST_CHECK_EQUAL(ritr,
                    result.data()+result.size());

}

BOOST_AUTO_TEST_CASE (decode_return_iterator) {

  std::string result = wikipedia1;

  auto ritr = sqy::base64::decode_impl(wikipedia1_encoded.data(),
                                       wikipedia1_encoded.data() + wikipedia1_encoded.size(),
                                       (char*)result.data());


  BOOST_CHECK_NE(ritr,
                 result.data());

  std::int32_t dist = std::distance((const char*)ritr,result.data()+result.size());
  BOOST_CHECK_EQUAL(dist,0);

}
BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( encoder_works, string_fixture )

BOOST_AUTO_TEST_CASE (on_wikipedia1) {

  std::string result = wikipedia1_encoded;
  std::fill(result.begin(),result.end(),' ');

  sqy::base64::encode_impl(wikipedia1.data(),
                   wikipedia1.data() + wikipedia1.size(),
                   (char*)result.data());

  BOOST_CHECK_EQUAL(result,
                    wikipedia1_encoded);


}

BOOST_AUTO_TEST_CASE (on_wikipedia2) {

  std::string result = wikipedia2_encoded;
  std::fill(result.begin(),result.end(),' ');

  sqy::base64::encode_impl(wikipedia2.data(),
                   wikipedia2.data() + wikipedia2.size(),
                   (char*)result.data());

  BOOST_CHECK_EQUAL(result,
                    wikipedia2_encoded);


}

BOOST_AUTO_TEST_CASE (on_wikipedia3) {

  std::string result = wikipedia3_encoded;
  std::fill(result.begin(),result.end(),' ');

  sqy::base64::encode_impl(wikipedia3.data(),
                   wikipedia3.data() + wikipedia3.size(),
                   (char*)result.data());

  BOOST_CHECK_EQUAL(result,
                    wikipedia3_encoded);


}

BOOST_AUTO_TEST_CASE (fast_on_wikipedia1) {

  std::string result = wikipedia1_encoded;
  std::fill(result.begin(),result.end(),' ');

  sqy::base64::fast_encode_impl(wikipedia1.data(),
                   wikipedia1.data() + wikipedia1.size(),
                   (char*)result.data());

  BOOST_CHECK_EQUAL(result,
                    wikipedia1_encoded);


}

BOOST_AUTO_TEST_CASE (fast_on_wikipedia2) {

  std::string result = wikipedia2_encoded;
  std::fill(result.begin(),result.end(),' ');

  sqy::base64::fast_encode_impl(wikipedia2.data(),
                   wikipedia2.data() + wikipedia2.size(),
                   (char*)result.data());

  BOOST_CHECK_EQUAL(result,
                    wikipedia2_encoded);


}

BOOST_AUTO_TEST_CASE (fast_on_wikipedia3) {

  std::string result = wikipedia3_encoded;
  std::fill(result.begin(),result.end(),' ');

  sqy::base64::fast_encode_impl(wikipedia3.data(),
                   wikipedia3.data() + wikipedia3.size(),
                   (char*)result.data());

  BOOST_CHECK_EQUAL(result,
                    wikipedia3_encoded);


}
BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( decoder_works, string_fixture )

BOOST_AUTO_TEST_CASE (on_wikipedia1) {

  std::string result = wikipedia1;
  std::fill(result.begin(),result.end(),'x');

  sqy::base64::decode_impl(wikipedia1_encoded.data(),
                   wikipedia1_encoded.data() + wikipedia1_encoded.size(),
                   (char*)result.data());

  BOOST_CHECK_EQUAL(result,
                    wikipedia1);


}

BOOST_AUTO_TEST_CASE (on_wikipedia2) {

  std::string result = wikipedia2;
  std::fill(result.begin(),result.end(),'x');

  sqy::base64::decode_impl(wikipedia2_encoded.data(),
                     wikipedia2_encoded.data() + wikipedia2_encoded.size(),
                     (char*)result.data());

  BOOST_CHECK_EQUAL(result,
                    wikipedia2);


}

BOOST_AUTO_TEST_CASE (on_wikipedia3) {

  std::string result = wikipedia3;
  std::fill(result.begin(),result.end(),'x');

  sqy::base64::decode_impl(wikipedia3_encoded.data(),
                     wikipedia3_encoded.data() + wikipedia3_encoded.size(),
                     (char*)result.data());

  BOOST_CHECK_EQUAL(result,
                    wikipedia3);


}


BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( roundtrips, string_fixture )
BOOST_AUTO_TEST_CASE (on_standard_text) {

  std::string result = standard_text;
  std::fill(result.begin(),result.end(),' ');

  std::string intermediate;intermediate.resize(4*std::ceil(result.size()/3.));

  sqy::base64::encode_impl(standard_text.data(),
                   standard_text.data() + standard_text.size(),
                   (char*)intermediate.data());

  sqy::base64::decode_impl(intermediate.data(),
                   intermediate.data() + intermediate.size(),
                   (char*)result.data());

  BOOST_CHECK_EQUAL(result,
                    standard_text);


}

BOOST_AUTO_TEST_CASE (encode_with_mine_decode_with_boost) {

  std::string result = standard_text;
  std::fill(result.begin(),result.end(),' ');

  std::string intermediate;intermediate.resize(4*std::ceil(result.size()/3.));

  sqy::base64::fast_encode_impl(standard_text.data(),
                   standard_text.data() + standard_text.size(),
                   (char*)intermediate.data());

  sqy::base64::decode_impl(intermediate.data(),
                     intermediate.data() + intermediate.size(),
                     (char*)result.data());

  BOOST_CHECK_EQUAL(result,
                    standard_text);


}

BOOST_AUTO_TEST_SUITE_END()
