#define BOOST_TEST_MODULE TEST_LZ4_ENCODING_IMPL
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <typeinfo>

#include <boost/property_tree/ptree.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <vector>
#include <iostream>
#include <sstream>
#include "array_fixtures.hpp"
#include <bitset>
#include "../src/sqeazy_header.hpp"

namespace bpt = boost::property_tree;

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;


BOOST_FIXTURE_TEST_SUITE( header_suite, uint16_cube_of_8 )
 
using namespace sqeazy;

BOOST_AUTO_TEST_CASE( simple_pack )
{
  
  
  std::string header = sqeazy::image_header::pack<value_type>(dims);
    
  BOOST_CHECK_NE(header.size(),0);
  BOOST_CHECK_EQUAL(header[header.size()-1],sqeazy::image_header::header_end_delimeter());
    
}

BOOST_AUTO_TEST_CASE( header_contructs_and_assigns )
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

BOOST_AUTO_TEST_CASE( encode_header_correct_typeid )
{

  std::string hdr = sqeazy::image_header::pack<value_type>(dims);
  std::stringstream header_stream(hdr.substr(0,hdr.size()-1));
  
  bpt::ptree tree;

  BOOST_CHECK_NO_THROW(bpt::read_json(header_stream, tree));
  BOOST_CHECK_EQUAL(tree.get<std::string>("raw.type"),typeid(value_type).name());
  
}

 BOOST_AUTO_TEST_CASE( encode_header_correct_num_dims )
{
  std::string hdr = sqeazy::image_header::pack<value_type>(dims);
  std::stringstream header_stream(hdr.substr(0,hdr.size()-1));
  
  bpt::ptree tree;

  BOOST_CHECK_NO_THROW(bpt::read_json(header_stream, tree));
  
  BOOST_CHECK_EQUAL(tree.get<unsigned>("raw.rank"),3);
    
}

 BOOST_AUTO_TEST_CASE( encode_header_correct_type_id )
{
  
  std::string header = sqeazy::image_header::pack<value_type>(dims);
  std::string type_id = sqeazy::image_header::unpack_type(header.c_str(),header.size());
  std::string expected = typeid(value_type).name();
  BOOST_CHECK_EQUAL(type_id,expected);

  header = sqeazy::image_header::pack<int>(dims);
  type_id = sqeazy::image_header::unpack_type(header.c_str(),header.size());
  expected = typeid(int).name();
  BOOST_CHECK_EQUAL(type_id,expected);

}


BOOST_AUTO_TEST_CASE( encode_header_correct_values_of_dims )
{

  sqeazy::image_header header(value_type(),dims);

  BOOST_CHECK_EQUAL_COLLECTIONS(dims.begin(), dims.end(), header.shape()->begin(), header.shape()->end());
}

BOOST_AUTO_TEST_CASE( encode_header_correct_values_of_dims_corner_cases )
{
  
  sqeazy::image_header hdr(value_type(),dims);

  std::string hstr = hdr.str();
  std::vector<unsigned long> extracted_dims_2(sqeazy::image_header::unpack_shape(hstr.c_str(), hstr.size()));
  BOOST_CHECK_EQUAL_COLLECTIONS(dims.begin(), dims.end(), extracted_dims_2.begin(), extracted_dims_2.end());
  
  std::string header_plus_separator = hdr.str();
  header_plus_separator += "|";
  std::vector<unsigned long> extracted_dims_3 = sqeazy::image_header::unpack_shape(header_plus_separator.c_str(), header_plus_separator.size());
  BOOST_CHECK_EQUAL_COLLECTIONS(dims.begin(), dims.end(), extracted_dims_3.begin(), extracted_dims_3.end());

  std::vector<unsigned> ramp_shape(dims);
  ramp_shape.front() -= 1;
  ramp_shape.back() += 1;
  sqeazy::image_header ramp_hdr(value_type(),ramp_shape);
  BOOST_CHECK_EQUAL_COLLECTIONS(ramp_shape.begin(), ramp_shape.end(), ramp_hdr.shape()->begin(), ramp_hdr.shape()->end());
}

BOOST_AUTO_TEST_CASE( pack_and_reload )
{
  sqeazy::image_header expected(value_type(),dims,"no_pipeline",1024);
  std::string given = sqeazy::image_header::pack<value_type>(dims,"no_pipeline",1024);
  
  BOOST_CHECK(expected.str() == given);
  BOOST_CHECK_EQUAL_COLLECTIONS(given.begin(), given.end(), expected.begin(), expected.end());
  
  sqeazy::image_header reloaded(given.begin(), given.end());

  BOOST_CHECK(expected == reloaded);
  BOOST_CHECK_EQUAL(expected.str(), reloaded.str());
    
}

BOOST_AUTO_TEST_CASE( type_name )
{
  sqeazy::image_header expected(value_type(),dims,"no_pipeline",1024);
  std::string hdr = expected.str();
  std::string rec = sqeazy::image_header::unpack_type(&hdr[0],hdr.size());
  std::string exp = typeid(value_type).name();
  
  BOOST_CHECK(!rec.empty());
  
  BOOST_CHECK_EQUAL(rec,exp);
}
BOOST_AUTO_TEST_SUITE_END()
