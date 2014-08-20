#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TEST_HUFFMAN_UTILS_IMPL
#include "boost/test/unit_test.hpp"
#include <climits>
#include <vector>
#include <iostream>
#include <bitset>
#include <map>
#include "../src/hist_impl.hpp"
#include "../src/huffman_utils.hpp"

struct huffman_fixture {

    typedef char value_type;

    typedef sqeazy::huffman_scheme<value_type>::HuffCodeMap map_type;
    typedef typename sqeazy::huffman_scheme<value_type>::HuffCodeMap::iterator map_itr_type;
    typedef typename sqeazy::huffman_scheme<value_type>::HuffCodeMap::const_iterator map_citr_type;


    static const int num_bits = (CHAR_BIT*sizeof(value_type));
    std::string sample;
    std::map<value_type,std::bitset<num_bits> > expected_map;
    huffman_fixture():
        sample("this is an example for huffman encoding")
    {
        expected_map[(value_type)' '] = std::bitset<num_bits>(std::string("110"));
        expected_map[(value_type)'a'] = std::bitset<num_bits>(std::string("1001"));
        expected_map[(value_type)'c'] = std::bitset<num_bits>(std::string("101010"));
        expected_map[(value_type)'d'] = std::bitset<num_bits>(std::string("10001"));
        expected_map[(value_type)'e'] = std::bitset<num_bits>(std::string("1111"));
        expected_map[(value_type)'f'] = std::bitset<num_bits>(std::string("1011"));
        expected_map[(value_type)'g'] = std::bitset<num_bits>(std::string("101011"));
        expected_map[(value_type)'h'] = std::bitset<num_bits>(std::string("0101"));
        expected_map[(value_type)'i'] = std::bitset<num_bits>(std::string("1110"));
        expected_map[(value_type)'l'] = std::bitset<num_bits>(std::string("01110"));
        expected_map[(value_type)'m'] = std::bitset<num_bits>(std::string("0011"));
        expected_map[(value_type)'n'] = std::bitset<num_bits>(std::string("000"));
        expected_map[(value_type)'o'] = std::bitset<num_bits>(std::string("0010"));
        expected_map[(value_type)'p'] = std::bitset<num_bits>(std::string("01000"));
        expected_map[(value_type)'r'] = std::bitset<num_bits>(std::string("01001"));
        expected_map[(value_type)'s'] = std::bitset<num_bits>(std::string("0110"));
        expected_map[(value_type)'t'] = std::bitset<num_bits>(std::string("01111"));
        expected_map[(value_type)'u'] = std::bitset<num_bits>(std::string("10100"));
        expected_map[(value_type)'x'] = std::bitset<num_bits>(std::string("10000"));


    }

};


BOOST_FIXTURE_TEST_SUITE( measurements, huffman_fixture )

BOOST_AUTO_TEST_CASE( created_huff_map )
{

    map_type codes;
    std::string dummy = sample;
    unsigned size = sample.size();
    sqeazy::huffman_scheme<value_type>::encode(sample.c_str(),&dummy[0],size, codes);


    BOOST_CHECK_EQUAL(codes.size(),expected_map.size());


    const int un_sym = sqeazy::huffman_scheme<value_type>::num_bits;
    const int exp_un_sym = num_bits;
    BOOST_CHECK_EQUAL(un_sym,exp_un_sym);

}

BOOST_AUTO_TEST_CASE( huff_map_correct )
{
    map_type codes;
    std::string dummy = sample;
    unsigned size = sample.size();
    sqeazy::huffman_scheme<value_type>::encode(sample.c_str(),&dummy[0],size, codes);



    map_citr_type exp_itr = expected_map.begin();
    map_citr_type rec_itr = codes.begin();
    map_citr_type rec_itrE = codes.end();

    for (; rec_itr != rec_itrE; ++rec_itr, ++exp_itr)
    {
        try {
            BOOST_REQUIRE_EQUAL(rec_itr->first, exp_itr->first);
        }
        catch(...) {
            std::cerr << "[mismatching key] expected " << rec_itr->first << ", received " << exp_itr->first << "\n";
        }

        try {
            BOOST_REQUIRE_EQUAL(rec_itr->second.to_ulong(), expected_map[rec_itr->first].to_ulong());
        }
        catch(...) {
            std::cerr << "[mismatching value] expected " << rec_itr->second.to_string() << ", received " << expected_map[rec_itr->first].to_string() << "\n";
        }
    }
}

BOOST_AUTO_TEST_CASE( reduces_entropy )
{
    sqeazy::histogram<char> sample_hist(sample.begin(), sample.end());

    
    std::string encoded = sample;
    unsigned size = sample.size();
    sqeazy::huffman_scheme<value_type>::encode(sample.c_str(),&encoded[0],size);

    sqeazy::histogram<char> encoded_hist(encoded.begin(), encoded.end());
    
    BOOST_CHECK_NE(sample_hist.median(),encoded_hist.median());
}

BOOST_AUTO_TEST_SUITE_END()
