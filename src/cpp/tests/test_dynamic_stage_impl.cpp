#define BOOST_TEST_MODULE TEST_DYNAMIC_PIPELINE
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <functional>
#include <cmath>
#include <vector>
#include <cstdint>
#include <sstream>
#include "array_fixtures.hpp"
//#include "encoders/sqeazy_impl.hpp"
#include "dynamic_stage.hpp"
#include "test_dynamic_pipeline_impl.hpp"

namespace sqy = sqeazy;

BOOST_AUTO_TEST_SUITE( stage_chain )

  
BOOST_AUTO_TEST_CASE( is_constructable )
{

  //empty
  {
  sqy::stage_chain< sqy::filter<int> > local;
  BOOST_CHECK(local.empty());
  BOOST_CHECK_NE(local.valid(),true);
  BOOST_CHECK_EQUAL(local.name().size(),0);
  }
  
  //init-list
  {
  sqy::stage_chain< sqy::filter<int> > filled_with_2 = {adder_sptr,square_sptr};
  BOOST_CHECK_NE(filled_with_2.empty(),true);
  BOOST_CHECK_EQUAL(filled_with_2.size(),2);
  BOOST_CHECK_EQUAL(filled_with_2.valid(),true);
  }
}

BOOST_AUTO_TEST_CASE( copy_constructable )
{

  //from empty
  {
  sqy::stage_chain< sqy::filter<int> > empty;
  sqy::stage_chain< sqy::filter<int> > local(empty);
  
  BOOST_CHECK(local.empty());
  BOOST_CHECK_NE(local.valid(),true);
  BOOST_CHECK_EQUAL(local.name().size(),0);
  }
  
  //from filled with 2
  {
    sqy::stage_chain< sqy::filter<int> > reference = {adder_sptr,square_sptr};
    sqy::stage_chain< sqy::filter<int> > filled_with_2(reference);
    BOOST_CHECK_NE(filled_with_2.empty(),true);
    BOOST_CHECK_EQUAL(filled_with_2.size(),2);
    BOOST_CHECK_EQUAL(filled_with_2.valid(),true);
  }
}

BOOST_AUTO_TEST_CASE( assignment )
{

  //from empty
  {
  sqy::stage_chain< sqy::filter<int> > empty;
  sqy::stage_chain< sqy::filter<int> > local = empty;
  
  BOOST_CHECK(local.empty());
  BOOST_CHECK_NE(local.valid(),true);
  BOOST_CHECK_EQUAL(local.name().size(),0);
  }
  
  //from filled with 2
  {
    sqy::stage_chain< sqy::filter<int> > reference = {adder_sptr,square_sptr};
    sqy::stage_chain< sqy::filter<int> > filled_with_2 = reference ;
    BOOST_CHECK_NE(filled_with_2.empty(),true);
    BOOST_CHECK_EQUAL(filled_with_2.size(),2);
    BOOST_CHECK_EQUAL(filled_with_2.valid(),true);
  }
}

BOOST_AUTO_TEST_CASE( invalid )
{

  sqy::stage_chain< sqy::sink<int> > local = {summer_sptr,hibits_sptr};

  BOOST_CHECK_EQUAL(local.valid(),false);
}

BOOST_AUTO_TEST_CASE( push_back )
{
  sqy::stage_chain< sqy::filter<int> > local;

  local.push_back(adder_sptr);
  local.push_back(square_sptr);

  BOOST_CHECK_NE(local.empty(),true);
  BOOST_CHECK_EQUAL(local.size(),2);

  local.push_back(summer_sptr); //base of summer is sqy::sink, not sqy::filter

  BOOST_CHECK_NE(local.empty(),true);
  BOOST_CHECK_EQUAL(local.size(),2);
}


BOOST_AUTO_TEST_CASE( encode_with_filters )
{

  sqy::stage_chain< sqy::filter<int> > chain = {adder_sptr,square_sptr};
  std::vector<int> input(10,2);
  std::vector<int> output(10,0);

  auto encoded_end = chain.encode(&input[0],&output[0],input.size());
  BOOST_CHECK(encoded_end!=nullptr);
  BOOST_CHECK_EQUAL(int(encoded_end-(&output[0])),output.size());
  BOOST_CHECK_EQUAL(*(encoded_end-1),9);
}

BOOST_AUTO_TEST_CASE( roundtrip_with_filters )
{

  sqy::stage_chain< sqy::filter<int> > chain = {adder_sptr,square_sptr};
  std::vector<int> input(10,2);
  std::vector<int> intermediate(10,0);
  std::vector<int> output(10,0);

  auto encoded_end = chain.encode(&input[0],&intermediate[0],input.size());
  BOOST_CHECK(encoded_end!=nullptr);

  int err = chain.decode(intermediate.data(),
			 output.data(),
			 input.size());

  BOOST_CHECK(err==0);
  BOOST_CHECK_EQUAL_COLLECTIONS(input.begin(), input.end(), output.begin(), output.end());

}
BOOST_AUTO_TEST_SUITE_END()
