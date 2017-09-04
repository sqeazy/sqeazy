#define BOOST_TEST_MODULE TEST_SQEAZY_HEADER_IMPL
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"

#include <numeric>
#include <typeinfo>
#include <vector>
#include <iostream>
#include <sstream>
#include <bitset>

#include "array_fixtures.hpp"
#include "sqeazy_header.hpp"

namespace bpt = boost::property_tree;

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;


BOOST_FIXTURE_TEST_SUITE( header_suite, uint16_cube_of_8 )

using namespace sqeazy;

BOOST_AUTO_TEST_CASE( simple_pack )
{


  std::string header = sqeazy::header::pack<value_type>(dims);

  BOOST_CHECK_NE(header.size(),0u);

  auto hdr_delim = sqeazy::header::header_end_delimeter();
  BOOST_CHECK_NE(header.rfind(hdr_delim),0u);
  BOOST_CHECK_NE(header.rfind(hdr_delim),std::string::npos);

}

BOOST_AUTO_TEST_CASE( header_contructs_and_assigns )
{
  sqeazy::header lhs(uint16_t(),
               dims,
               boost::unit_test::framework::current_test_case().p_name,
               42);

  BOOST_CHECK_GT(lhs.size(),0u);

  sqeazy::header rhs(uint16_t(),
               dims,
               boost::unit_test::framework::current_test_case().p_name,
               42);

  BOOST_CHECK_GT(rhs.size(),0u);

  sqeazy::header foo(uint8_t(),
               dims,
               "",
               12);

  BOOST_CHECK_GT(foo.size(),0u);
  BOOST_CHECK(lhs == rhs);
  BOOST_CHECK(lhs!=foo);

}

BOOST_AUTO_TEST_CASE( header_copies_and_assigns )
{
  sqeazy::header lhs(uint16_t(),
               dims,
               boost::unit_test::framework::current_test_case().p_name,
               42);

  sqeazy::header copied(lhs);
  sqeazy::header assinged = lhs;

  BOOST_CHECK(lhs==copied);
  BOOST_CHECK(lhs==assinged);
}


// BOOST_AUTO_TEST_CASE( encode_header_correct_typeid )
// {

//   std::string hdr = sqeazy::header::pack<value_type>(dims);
//   int offset = sqeazy::header::header_end_delimeter().size();

//   auto temp = hdr.substr(0,hdr.size()-offset);
//   BOOST_CHECK_GT(temp.size(),0u);
//   BOOST_CHECK_LT(temp.size(),hdr.size());

//   sqeazy::header::tag tree = sqeazy::header::from_string(temp);
//   std::string received = tree.raw_type_id_;
//   #ifdef _WIN32
//   if (received.find("_") != std::string::npos)
//     received.replace(received.find("_"),1," ");
// #endif
//   BOOST_CHECK_EQUAL(received,typeid(value_type).name());
// }


BOOST_AUTO_TEST_CASE( encode_header_correct_typeid )
{

  std::string hdr = sqeazy::header::pack<value_type>(dims);

  int offset = sqeazy::header::header_end_delimeter().size();
  std::stringstream header_stream(hdr.substr(0,hdr.size()-offset));

  bpt::ptree tree;

  BOOST_CHECK_NO_THROW(bpt::read_json(header_stream, tree));

  std::string received = tree.get<std::string>("raw.type");
#ifdef _WIN32
  if (received.find("_") != std::string::npos)
      received.replace(received.find("_"),1," ");
#endif
  BOOST_CHECK_EQUAL(received,typeid(value_type).name());

}

BOOST_AUTO_TEST_CASE( header_size_aligned_to_type )
{

  sqeazy::header int_uneven(int(),
                  dims,
                  "no_pipelin",//wrong name to make header string no multiple of 4
                  1024);
  BOOST_CHECK(int_uneven.size() % sizeof(int) == 0);
  sqeazy::header int_uneven_reread(int_uneven.str());
  BOOST_CHECK(int_uneven_reread == int_uneven);

  sqeazy::header short_uneven(short(),
                    dims,
                    "no_pipelin",//wrong name to make header string no multiple of 2
                    1024);
  BOOST_CHECK(short_uneven.size() % sizeof(short) == 0);
  sqeazy::header short_uneven_reread(short_uneven.str());
  BOOST_CHECK(short_uneven_reread == short_uneven);

  sqeazy::header char_uneven(char(),dims,"no_pipeline",1024);
  BOOST_CHECK(char_uneven.size() % sizeof(char) == 0);
  sqeazy::header char_uneven_reread(char_uneven.str());
  BOOST_CHECK(char_uneven_reread == char_uneven);

}

BOOST_AUTO_TEST_CASE( encode_header_correct_num_dims )
{
  std::string hdr = sqeazy::header::pack<value_type>(dims);

  int offset  = sqeazy::header::header_end_delimeter().size();
  std::stringstream header_stream(hdr.substr(0,hdr.size()-offset));

  //header::tag tree;
  bpt::ptree tree;

  // BOOST_CHECK_NO_THROW(tree = header::from_string(header_stream.str()));
  BOOST_CHECK_NO_THROW(bpt::read_json(header_stream.str(),tree));
  BOOST_CHECK_EQUAL(tree.get<unsigned>("raw.rank"),3u);

}

 BOOST_AUTO_TEST_CASE( encode_header_correct_type_id )
{

  std::string header = sqeazy::header::pack<value_type>(dims);
  std::string type_id = sqeazy::header::unpack_type(header.c_str(),header.size());
  std::string expected = typeid(value_type).name();
  BOOST_CHECK_EQUAL(type_id,expected);

  header = sqeazy::header::pack<int>(dims);
  type_id = sqeazy::header::unpack_type(header.c_str(),header.size());
  expected = typeid(int).name();
  BOOST_CHECK_EQUAL(type_id,expected);

}


BOOST_AUTO_TEST_CASE( encode_header_correct_values_of_dims )
{

  sqeazy::header header(value_type(),dims);

  BOOST_CHECK_EQUAL_COLLECTIONS(dims.begin(), dims.end(), header.shape()->begin(), header.shape()->end());
}

BOOST_AUTO_TEST_CASE( encode_header_correct_values_of_dims_corner_cases )
{

  sqeazy::header hdr(value_type(),dims);

  std::string hstr = hdr.str();
  std::vector<std::size_t> extracted_dims_2(sqeazy::header::unpack_shape(hstr.c_str(), hstr.size()));
  BOOST_CHECK_EQUAL_COLLECTIONS(dims.begin(), dims.end(), extracted_dims_2.begin(), extracted_dims_2.end());

  std::string header_plus_separator = hdr.str();
  header_plus_separator += "|";
  std::vector<std::size_t> extracted_dims_3 = sqeazy::header::unpack_shape(header_plus_separator.c_str(), header_plus_separator.size());
  BOOST_CHECK_EQUAL_COLLECTIONS(dims.begin(), dims.end(), extracted_dims_3.begin(), extracted_dims_3.end());

  std::vector<unsigned> ramp_shape(dims);
  ramp_shape.front() -= 1;
  ramp_shape.back() += 1;
  sqeazy::header ramp_hdr(value_type(),ramp_shape);
  BOOST_CHECK_EQUAL_COLLECTIONS(ramp_shape.begin(), ramp_shape.end(), ramp_hdr.shape()->begin(), ramp_hdr.shape()->end());
}

BOOST_AUTO_TEST_CASE( pack_and_reload )
{
  sqeazy::header expected(value_type(),dims,"no_pipeline",1024);
  std::string given = sqeazy::header::pack<value_type>(dims,"no_pipeline",1024);

  BOOST_CHECK(expected.str() == given);
  BOOST_CHECK_EQUAL_COLLECTIONS(given.begin(), given.end(), expected.begin(), expected.end());

  sqeazy::header reloaded(given.begin(), given.end());

  BOOST_CHECK(expected == reloaded);
  BOOST_CHECK_EQUAL(expected.str(), reloaded.str());

}

BOOST_AUTO_TEST_CASE( type_name )
{
  sqeazy::header expected(value_type(),dims,"no_pipeline",1024);
  std::string hdr = expected.str();
  std::string rec = sqeazy::header::unpack_type(&hdr[0],hdr.size());
  std::string exp = typeid(value_type).name();

  BOOST_CHECK(!rec.empty());

  BOOST_CHECK_EQUAL(rec,exp);
}


BOOST_AUTO_TEST_CASE( do_not_touch_whitespaces_in_verbatim )
{

  std::string msg = "these whitespaces should stay";
  std::string pipeline = "step1(junk=<verbatim>these whitespaces should stay</verbatim>)";


  std::string hdr_str = sqeazy::header::pack<value_type>(dims,pipeline,1024);
  BOOST_CHECK(sqeazy::ends_with(hdr_str.begin(),hdr_str.end(),sqeazy::header::header_end_delimeter()));

  sqeazy::header rec_hdr = sqeazy::header::unpack(hdr_str);
  std::string rec = rec_hdr.str();

  auto fpos = rec.find("these whitespaces should stay");
  BOOST_CHECK(fpos!=std::string::npos);
  // std::cout << "rec = " << rec << "\n";

}


BOOST_AUTO_TEST_CASE( property_tree_conserves_whitespaces )
{

  bpt::ptree tree;
  std::string msg = "these whitespaces should stay";

  tree.put("test_str", msg);

  std::string received = tree.get<std::string>("test_str");

  BOOST_CHECK_EQUAL(received,msg);

}

BOOST_AUTO_TEST_CASE( property_tree_conserves_whitespaces_in_json )
{

  bpt::ptree tree;
  std::string msg = "these whitespaces should stay";

  tree.put("test_str", msg);

  std::stringstream json("");
  bpt::write_json(json, tree);

  std::string json_str = json.str();

  std::stringstream rec_json("");
  rec_json << json_str;

  bpt::ptree rec_tree;
  bpt::read_json(rec_json,rec_tree);

  std::string received = rec_tree.get<std::string>("test_str");

  BOOST_CHECK_EQUAL(received,msg);

}

// BOOST_AUTO_TEST_CASE( remove_whitespaces )
// {

//   std::string plain_pipeline = "step1(  junk=123  )  ";
//   std::string plain_stripped = sqeazy::remove_whitespace(plain_pipeline);
//   BOOST_CHECK_LT(plain_stripped.size(),plain_pipeline.size());
//   BOOST_CHECK_EQUAL(plain_stripped.size(),plain_pipeline.size()-6);

// }

// BOOST_AUTO_TEST_CASE( remove_whitespaces_ignoring_verbatim )
// {


//   std::string pipeline = "step1(  junk=<verbatim>these whitespaces should stay</verbatim>  )  ";
//   std::string stripped = sqeazy::remove_whitespace(pipeline);
//   BOOST_CHECK_LT(stripped.size(),pipeline.size());
//   BOOST_CHECK_EQUAL(stripped.size(),pipeline.size()-6);

// }

BOOST_AUTO_TEST_CASE( ends_with )
{
  std::string empty = "";
  std::string at_start = "123 Hello World";
  std::string at_end = "123 Hello World 123";

  std::string match = "123";

  BOOST_CHECK(!sqeazy::ends_with(empty.begin(),empty.end(),match));
  BOOST_CHECK(!sqeazy::ends_with(at_start.begin(),at_start.end(),match));
  BOOST_CHECK(sqeazy::ends_with(at_end.begin(),at_end.end(),match));
}


BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( valid_headers, uint16_cube_of_8 )


BOOST_AUTO_TEST_CASE( is_valid )
{
  std::string header = sqeazy::header::pack<value_type>(dims);
  BOOST_CHECK(sqeazy::header::valid_header(header));

  header = sqeazy::header::pack<value_type>(dims,boost::unit_test::framework::current_test_case().p_name);
  BOOST_CHECK(sqeazy::header::valid_header(header));

  header = sqeazy::header::pack<value_type>(dims,boost::unit_test::framework::current_test_case().p_name,1024);
  BOOST_CHECK(sqeazy::header::valid_header(header));

}

BOOST_AUTO_TEST_CASE( removed_end )
{
  std::string header = sqeazy::header::pack<value_type>(dims,
                                                              boost::unit_test::framework::current_test_case().p_name,
                                                              1024);
  BOOST_CHECK(sqeazy::header::valid_header(header));
  BOOST_CHECK(!sqeazy::header::valid_header(header.begin(),header.end()-10));
  BOOST_CHECK(!sqeazy::header::valid_header(header.begin(),header.end()-1));
  BOOST_CHECK(!sqeazy::header::valid_header(header.begin(),header.end()-2));
  BOOST_CHECK(!sqeazy::header::valid_header(header.begin(),header.end()-4));
}

BOOST_AUTO_TEST_CASE( removed_start )
{
  std::string header = sqeazy::header::pack<value_type>(dims,
                                                              boost::unit_test::framework::current_test_case().p_name,
                                                              1024);
  BOOST_CHECK(sqeazy::header::valid_header(header));
  BOOST_CHECK(!sqeazy::header::valid_header(header.begin()+10,header.end()));
  BOOST_CHECK(!sqeazy::header::valid_header(header.begin()+1,header.end()));
  BOOST_CHECK(!sqeazy::header::valid_header(header.begin()+2,header.end()));
  BOOST_CHECK(!sqeazy::header::valid_header(header.begin()+4,header.end()));
}



BOOST_AUTO_TEST_CASE( header_roundtrip )
{

  sqeazy::header hdr(value_type(),
                           dims,
                           boost::unit_test::framework::current_test_case().p_name,
                           1024);

  auto serialized_hdr = hdr.str();
  BOOST_CHECK(sqeazy::header::valid_header(hdr.str()));

  sqeazy::header rt_hdr(serialized_hdr);
  BOOST_CHECK(sqeazy::header::valid_header(rt_hdr.str()));

  BOOST_CHECK(rt_hdr == hdr);
}

BOOST_AUTO_TEST_SUITE_END()
