#define BOOST_TEST_DYN_LINK 
#define BOOST_TEST_MODULE TEST_LZ4_ENCODING_IMPL
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include <sstream>
#include "array_fixtures.hpp"
#include <bitset>
#include "../src/external_encoders.hpp"

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;

template <char delimiter, typename T>
void split_string_to_vector(const std::string& _buffer, std::vector<T>& _v){
  
  int num_del = std::count(_buffer.begin(), _buffer.end(), delimiter);
  if(_v.size() != num_del + 1)
  _v.resize(num_del+1);
    
  size_t begin = 0;
  size_t end = _buffer.find(delimiter);
  
  std::string token;
  
  for(int id = 0;id<_v.size();++id){
    std::istringstream converter(_buffer.substr(begin,end - begin));
    converter >> _v[id];
    begin = end +1;
    end = _buffer.find(delimiter,begin);
  }

}

BOOST_FIXTURE_TEST_SUITE( lz4_out_of_place, uint16_cube_of_8 )
 

BOOST_AUTO_TEST_CASE( encode_header )
{
  
  
  std::string header = sqeazy::image_header<value_type>::pack(dims);
    
  BOOST_CHECK_NE(header.size(),0);
  
    
}

BOOST_AUTO_TEST_CASE( encode_header_correct_typeid )
{
  
  
  std::string header = sqeazy::image_header<value_type>::pack(dims);
  
  
  size_t first_comma = header.find(",");
  BOOST_CHECK_EQUAL(header.substr(0,first_comma),typeid(value_type).name());
  
  
  
}

 BOOST_AUTO_TEST_CASE( encode_header_correct_num_dims )
{
  
  std::string header = sqeazy::image_header<value_type>::pack(dims);
  
  
  size_t first_comma = header.find(",");
  BOOST_CHECK_EQUAL(header.substr(0,first_comma),typeid(value_type).name());
  
  size_t second_comma = header.find(",",first_comma);
  std::istringstream converter(header.substr(first_comma+1,second_comma));
  int num_dims;
  converter >> num_dims;
  BOOST_CHECK_EQUAL(num_dims,3);
    
}

BOOST_AUTO_TEST_CASE( encode_header_correct_values_of_dims )
{
  std::string header = sqeazy::image_header<value_type>::pack(dims);
  
  size_t first_comma = header.find(",");
  size_t second_comma = header.find(",",first_comma);
  size_t third_comma = header.find(",",second_comma+1);
  std::string dim_field = header.substr(third_comma+1,header.size());
   
  int num_x = std::count(dim_field.begin(), dim_field.end(), 'x');
  BOOST_CHECK_EQUAL(num_x,2);
  
  std::vector<int> extracted_dims(num_x+1);
  split_string_to_vector<'x'>(dim_field, extracted_dims);
  BOOST_CHECK_EQUAL(extracted_dims.size(),3);
  BOOST_CHECK_EQUAL_COLLECTIONS(dims.begin(), dims.end(), extracted_dims.begin(), extracted_dims.end());

   
  std::vector<unsigned> extracted_dims_2 = sqeazy::image_header<value_type>::unpack(dim_field);
  BOOST_CHECK_EQUAL_COLLECTIONS(dims.begin(), dims.end(), extracted_dims_2.begin(), extracted_dims_2.end());
}

BOOST_AUTO_TEST_CASE( encode_header_correct_values_of_dims_corner_cases )
{
  std::string header = sqeazy::image_header<value_type>::pack(dims);
  
  std::vector<unsigned> extracted_dims_2 = sqeazy::image_header<value_type>::unpack(header.c_str(), header.size());
  BOOST_CHECK_EQUAL_COLLECTIONS(dims.begin(), dims.end(), extracted_dims_2.begin(), extracted_dims_2.end());
  
  std::string header_plus_separator = header;
  header_plus_separator += "|";
  std::vector<unsigned> extracted_dims_3 = sqeazy::image_header<value_type>::unpack(header_plus_separator.c_str(), header_plus_separator.size());
  BOOST_CHECK_EQUAL_COLLECTIONS(dims.begin(), dims.end(), extracted_dims_3.begin(), extracted_dims_3.end());
}

BOOST_AUTO_TEST_CASE( encode_custom_header )
{
  std::string given = "h,1,32|bla";
  sqeazy::image_header<value_type> instance(given);
  
  BOOST_CHECK_EQUAL(instance.payload_size(),32);
  BOOST_CHECK_EQUAL(instance.payload_size_byte(),32*sizeof(value_type));
  BOOST_CHECK_EQUAL(instance.dims.size(),1);
  BOOST_CHECK_EQUAL(instance.size(),given.size()-3);
  
  std::string given2 = "h,1,32";
  sqeazy::image_header<value_type> instance2(given);
  BOOST_CHECK_EQUAL(instance2.payload_size(),32);
  BOOST_CHECK_EQUAL(instance2.payload_size_byte(),32*sizeof(value_type));
  BOOST_CHECK_EQUAL(instance2.dims.size(),1);
  BOOST_CHECK_EQUAL(instance2.size(),given.size()-3);
}

BOOST_AUTO_TEST_CASE( decode_dimensions )
{
  std::string given = "h,1,32x32x16|bla";
  std::vector<unsigned> dims = sqeazy::lz4_scheme<short>::decode_dimensions(given.c_str(),given.size());
  BOOST_CHECK_EQUAL(dims.size(),3);
  BOOST_CHECK_EQUAL(dims[0],32);
  BOOST_CHECK_EQUAL(dims[1],32);
  BOOST_CHECK_EQUAL(dims[2],16);
}


BOOST_AUTO_TEST_SUITE_END()


// BOOST_AUTO_TEST_SUITE_END()
