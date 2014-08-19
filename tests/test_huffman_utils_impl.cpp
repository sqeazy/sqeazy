#define BOOST_TEST_DYN_LINK 
#define BOOST_TEST_MODULE TEST_HUFFMAN_UTILS_IMPL
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include <bitset>
#include <map>
#include "array_fixtures.hpp"
#include "../src/huffman_utils.hpp"

struct huffman_fixture {

  std::string sample;
  std::map<char,std::bitset<8> > expected_map;
  huffman_fixture():
    sample("this is an example for huffman encoding")
  {
    expected_map[(char)' '] = std::bitset<8>(std::string("110"));		
    expected_map[(char)'a'] = std::bitset<8>(std::string("1001"));
    expected_map[(char)'c'] = std::bitset<8>(std::string("101010"));
    expected_map[(char)'d'] = std::bitset<8>(std::string("10001"));
    expected_map[(char)'e'] = std::bitset<8>(std::string("1111"));
    expected_map[(char)'f'] = std::bitset<8>(std::string("1011"));
    expected_map[(char)'g'] = std::bitset<8>(std::string("101011"));
    expected_map[(char)'h'] = std::bitset<8>(std::string("0101"));
    expected_map[(char)'i'] = std::bitset<8>(std::string("1110"));
    expected_map[(char)'l'] = std::bitset<8>(std::string("01110"));
    expected_map[(char)'m'] = std::bitset<8>(std::string("0011"));
    expected_map[(char)'n'] = std::bitset<8>(std::string("000"));
    expected_map[(char)'o'] = std::bitset<8>(std::string("0010"));
    expected_map[(char)'p'] = std::bitset<8>(std::string("01000"));
    expected_map[(char)'r'] = std::bitset<8>(std::string("01001"));
    expected_map[(char)'s'] = std::bitset<8>(std::string("0110"));
    expected_map[(char)'t'] = std::bitset<8>(std::string("01111"));
    expected_map[(char)'u'] = std::bitset<8>(std::string("10100"));
    expected_map[(char)'x'] = std::bitset<8>(std::string("10000"));
    

  }

};

BOOST_FIXTURE_TEST_SUITE( measurements, huffman_fixture )

BOOST_AUTO_TEST_CASE( success )
{
  sqeazy::huffman_scheme<char>::HuffCodeMap codes;
  std::string dummy = sample;
  unsigned size = sample.size();
  sqeazy::huffman_scheme<char>::encode(sample.c_str(),&dummy[0],size, codes);
 
  for (sqeazy::huffman_scheme<char>::HuffCodeMap::const_iterator it = codes.begin(); it != codes.end(); ++it)
    {
      std::cout << it->first << " " << it->second;
      // std::copy(it->second.begin(), it->second.end(),
      // 		std::ostream_iterator<bool>(std::cout));
      
      std::cout << std::endl;
    }

    BOOST_CHECK_EQUAL(codes.size(),expected_map.size());
  
}

BOOST_AUTO_TEST_SUITE_END()
