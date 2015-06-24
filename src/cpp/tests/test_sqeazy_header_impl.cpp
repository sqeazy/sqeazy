#define BOOST_TEST_MODULE TEST_LZ4_ENCODING_IMPL
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <typeinfo>


#include <vector>
#include <iostream>
#include <sstream>
#include "array_fixtures.hpp"
#include <bitset>
#include "../src/sqeazy_header.hpp"

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
 
using namespace sqeazy;

BOOST_AUTO_TEST_CASE( encode_header )
{
  
  
  std::string header = sqeazy::image_header::pack<value_type>(dims);
    
  BOOST_CHECK_NE(header.size(),0);
  
    
}

BOOST_AUTO_TEST_CASE( header_contructs_and_equal )
{
  sqeazy::image_header lhs(uint16_t(),
			   dims,
			   boost::unit_test::framework::current_test_case().p_name,
			   42);
  
  BOOST_CHECK_GT(lhs.size(),0);
  
  sqeazy::image_header rhs(uint16_t(),
			   dims,
			   boost::unit_test::framework::current_test_case().p_name,
			   42);
  
  BOOST_CHECK_GT(rhs.size(),0);
  
  sqeazy::image_header foo(uint8_t(),
			   dims,
			   "",
			   12);
  
  BOOST_CHECK_GT(foo.size(),0);
  BOOST_CHECK(lhs == rhs);
  BOOST_CHECK(lhs!=foo);
    
}

BOOST_AUTO_TEST_CASE( header_copies_and_assigns )
{
  sqeazy::image_header lhs(uint16_t(),
			   dims,
			   boost::unit_test::framework::current_test_case().p_name,
			   42);

  sqeazy::image_header copied(lhs);
  sqeazy::image_header assinged = lhs;

  BOOST_CHECK(lhs==copied);
  BOOST_CHECK(lhs==assinged);
}

// BOOST_AUTO_TEST_CASE( encode_header_correct_typeid )
// {
  
  
//   std::string header = sqeazy::image_header<value_type>::pack(dims);
    
//   size_t comma_1 = header.find_first_of(",");
//   size_t comma_2 = header.find_first_of(",",comma_1+1);
  
//   BOOST_CHECK_MESSAGE(comma_2-comma_1-1 == 1, "first 2 commas not found in right distance" << header);
//   BOOST_CHECK_EQUAL(header.substr(comma_1+1,1),typeid(value_type).name());
  
// }

//  BOOST_AUTO_TEST_CASE( encode_header_correct_num_dims )
// {
  
//   std::string header = sqeazy::image_header<value_type>::pack(dims);
 
//   size_t comma_1 = header.find_first_of(",");
//   size_t comma_2 = header.find_first_of(",",comma_1+1);

//   BOOST_CHECK_EQUAL(header.substr(comma_2+1,1),"3");
    
// }

//  BOOST_AUTO_TEST_CASE( encode_header_correct_type_id )
// {
  
//   std::string header = sqeazy::image_header<value_type>::pack(dims);
//   std::string type_id = sqeazy::image_header<value_type>::unpack_type(header.c_str(),header.size());
//   std::string expected = typeid(value_type).name();
//   BOOST_CHECK_EQUAL(type_id,expected);

//   header = sqeazy::image_header<int>::pack(dims);
//   type_id = sqeazy::image_header<int>::unpack_type(header.c_str(),header.size());
//   expected = typeid(int).name();
//   BOOST_CHECK_EQUAL(type_id,expected);

// }


// BOOST_AUTO_TEST_CASE( encode_header_correct_values_of_dims )
// {
//   std::string header = sqeazy::image_header<value_type>::pack(dims);
  
//   size_t comma_last = header.find_last_of(",");
//   std::string dim_field = header.substr(comma_last+1,header.size()-comma_last-1);

//   int num_x = std::count(dim_field.begin(), dim_field.end(), 'x');
//   BOOST_CHECK_EQUAL(num_x,2);
  
//   std::vector<int> extracted_dims(num_x+1);
//   split_string_to_vector<'x'>(dim_field, extracted_dims);
//   BOOST_CHECK_EQUAL(extracted_dims.size(),3);
//   BOOST_CHECK_EQUAL_COLLECTIONS(dims.begin(), dims.end(), extracted_dims.begin(), extracted_dims.end());
   
//   std::vector<unsigned> extracted_dims_2 = sqeazy::image_header<value_type>::unpack_shape(header.c_str(), header.size());
//   BOOST_CHECK_EQUAL_COLLECTIONS(dims.begin(), dims.end(), extracted_dims_2.begin(), extracted_dims_2.end());
// }

// BOOST_AUTO_TEST_CASE( encode_header_correct_values_of_dims_corner_cases )
// {
//   std::string header = sqeazy::image_header<value_type>::pack(dims);
  
//   std::vector<unsigned> extracted_dims_2 = sqeazy::image_header<value_type>::unpack_shape(header.c_str(), header.size());
//   BOOST_CHECK_EQUAL_COLLECTIONS(dims.begin(), dims.end(), extracted_dims_2.begin(), extracted_dims_2.end());
  
//   std::string header_plus_separator = header;
//   header_plus_separator += "|";
//   std::vector<unsigned> extracted_dims_3 = sqeazy::image_header<value_type>::unpack_shape(header_plus_separator.c_str(), header_plus_separator.size());
//   BOOST_CHECK_EQUAL_COLLECTIONS(dims.begin(), dims.end(), extracted_dims_3.begin(), extracted_dims_3.end());
// }

// BOOST_AUTO_TEST_CASE( encode_header_correct_sizeof )
// {
//   std::string given = sqeazy::image_header<value_type>::pack(dims);
//   sqeazy::image_header<sqeazy::unknown> instance_void(given);
  
//   BOOST_CHECK_EQUAL(instance_void.sizeof_header_type(),2);
//   BOOST_CHECK_EQUAL(instance_void.raw_size_byte()/instance_void.raw_size(),sizeof(value_type));
    
// }

// BOOST_AUTO_TEST_CASE( encode_custom_header )
// {
//   std::string given = "no_pipeline,s,1,32|bla";
//   sqeazy::image_header<value_type> instance(given);

//   BOOST_CHECK_EQUAL(sqeazy::image_header<value_type>::valid_header(given),true);
//   BOOST_CHECK_EQUAL(instance.raw_size(),32);
//   BOOST_CHECK_EQUAL(instance.raw_size_byte(),32*sizeof(value_type));
//   BOOST_CHECK_EQUAL(instance.dims.size(),1);
//   BOOST_CHECK_EQUAL(instance.size(),given.size()-3);
  
//   std::string given2 = "no_pipeline,s,1,32";
    
//   sqeazy::image_header<value_type> instance2(given2);

//   BOOST_CHECK_EQUAL(sqeazy::image_header<value_type>::valid_header(given2),true);
//   BOOST_CHECK_EQUAL(instance2.raw_size(),32);
//   BOOST_CHECK_EQUAL(instance2.raw_size_byte(),32*sizeof(value_type));
//   BOOST_CHECK_EQUAL(instance2.dims.size(),1);
//   BOOST_CHECK_EQUAL(instance2.size(),given2.size()+1);
  

//   sqeazy::image_header<sqeazy::unknown> instance_void(given2);

//   BOOST_CHECK_EQUAL(instance_void.raw_size(),32);
//   BOOST_CHECK_EQUAL(instance_void.raw_size_byte(),32*sizeof(value_type));
//   BOOST_CHECK_EQUAL(instance_void.dims.size(),1);
//   BOOST_CHECK_EQUAL(instance_void.size(),given2.size()+1);
  
// }

// BOOST_AUTO_TEST_CASE( decode_pipeline_from_custom_header )
// {
//   std::string given = "no_pipeline,s,1,32|bla";
//   sqeazy::image_header<value_type> instance(given);
  
//   BOOST_CHECK_EQUAL(instance.pipeline(),"no_pipeline");

// }

// BOOST_AUTO_TEST_CASE( decode_pipeline_from_void_header )
// {
//   std::string given = "no_pipeline,s,1,32|bla";
//   sqeazy::image_header<void> instance(given);
  
//   BOOST_CHECK_EQUAL(instance.pipeline(),"no_pipeline");

// }

BOOST_AUTO_TEST_SUITE_END()
