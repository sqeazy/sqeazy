#define BOOST_TEST_MODULE TEST_DYNAMIC_PIPELINE
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"
#include <numeric>
#include <functional>
#include <cmath>
#include <vector>
#include <cstdint>
#include <sstream>
#include "array_fixtures.hpp"
//#include "encoders/sqeazy_impl.hpp"
#include "dynamic_stage.hpp"
#include "dynamic_stage_chain.hpp"
#include "test_dynamic_pipeline_impl.hpp"

namespace sqy = sqeazy;

BOOST_AUTO_TEST_SUITE( stage )

BOOST_AUTO_TEST_CASE( to_other_type )
{

  add_one<int> adder;
  add_one<char> sadder(adder);

  BOOST_CHECK(adder.input_type()!=sadder.input_type());

}

BOOST_AUTO_TEST_CASE( n_threads_defaults_to_1 )
{

  add_one<int> adder;

  BOOST_CHECK_NE(adder.n_threads(),0u);
  BOOST_CHECK_EQUAL(adder.n_threads(),1u);

}

BOOST_AUTO_TEST_CASE( n_threads_is_mutable )
{

  add_one<int> adder;

  auto old_value = adder.n_threads();
  adder.set_n_threads(2);

  BOOST_CHECK_NE(adder.n_threads(),old_value);
  BOOST_CHECK_EQUAL(adder.n_threads(),2u);

}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( stage_chain )


BOOST_AUTO_TEST_CASE( is_constructable )
{

  //empty
  {
  sqy::stage_chain< sqy::filter<int> > local;
  BOOST_CHECK(local.empty());
  BOOST_CHECK_NE(local.valid(),true);
  BOOST_CHECK_EQUAL(local.name().size(),0u);
  }

  //init-list
  {
  sqy::stage_chain< sqy::filter<int> > filled_with_2 = {adder_sptr,square_sptr};
  BOOST_CHECK_NE(filled_with_2.empty(),true);
  BOOST_CHECK_EQUAL(filled_with_2.size(),2u);
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
  BOOST_CHECK_EQUAL(local.name().size(),0u);
  }

  //from filled with 2
  {
    sqy::stage_chain< sqy::filter<int> > reference = {adder_sptr,square_sptr};
    sqy::stage_chain< sqy::filter<int> > filled_with_2(reference);
    BOOST_CHECK_NE(filled_with_2.empty(),true);
    BOOST_CHECK_EQUAL(filled_with_2.size(),2u);
    BOOST_CHECK_EQUAL(filled_with_2.valid(),true);
  }

  //TODO: from different type
  // {
  //   sqy::stage_chain< sqy::filter<int> > int_reference = {adder_sptr,square_sptr};
  //   sqy::stage_chain< sqy::filter<short> > short_reference(int_reference);
  //   BOOST_CHECK_EQUAL(short_reference.size(),2);
  //   BOOST_CHECK_EQUAL(short_reference.valid(),true);

  // }

}

BOOST_AUTO_TEST_CASE( assignment )
{

  //from empty
  {
  sqy::stage_chain< sqy::filter<int> > empty;
  sqy::stage_chain< sqy::filter<int> > local = empty;

  BOOST_CHECK(local.empty());
  BOOST_CHECK_NE(local.valid(),true);
  BOOST_CHECK_EQUAL(local.name().size(),0u);
  }

  //from filled with 2
  {
    sqy::stage_chain< sqy::filter<int> > reference = {adder_sptr,square_sptr};
    sqy::stage_chain< sqy::filter<int> > filled_with_2 = reference ;
    BOOST_CHECK_NE(filled_with_2.empty(),true);
    BOOST_CHECK_EQUAL(filled_with_2.size(),2u);
    BOOST_CHECK_EQUAL(filled_with_2.valid(),true);
  }

  //TODO: from different type
  // {
  //   sqy::stage_chain< sqy::filter<int> > int_reference = {adder_sptr,square_sptr};
  //   sqy::stage_chain< sqy::filter<short> > short_reference = int_reference;
  //   BOOST_CHECK_EQUAL(short_reference.size(),2);
  //   BOOST_CHECK_EQUAL(short_reference.valid(),true);

  // }
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
  BOOST_CHECK_EQUAL(local.size(),2u);

  local.push_back(summer_sptr); //base of summer is sqy::sink, not sqy::filter

  BOOST_CHECK_NE(local.empty(),true);
  BOOST_CHECK_EQUAL(local.size(),2u);
}


BOOST_AUTO_TEST_CASE( encode_with_filters )
{

  sqy::stage_chain< sqy::filter<int> > chain = {adder_sptr,square_sptr};
  std::vector<int> input(10,2);
  std::vector<int> output(10,0u);

  auto encoded_end = chain.encode(&input[0],&output[0],input.size());
  BOOST_CHECK(encoded_end!=nullptr);
  BOOST_CHECK_EQUAL(std::size_t(encoded_end-(&output[0])),output.size());
  BOOST_CHECK_EQUAL(*(encoded_end-1),9);
}

BOOST_AUTO_TEST_CASE( roundtrip_with_filters )
{

  sqy::stage_chain< sqy::filter<int> > chain = {adder_sptr,square_sptr};
  std::vector<int> input(10,2);
  std::vector<int> intermediate(10,0u);
  std::vector<int> output(10,0u);

  auto encoded_end = chain.encode(&input[0],&intermediate[0],input.size());
  BOOST_CHECK(encoded_end!=nullptr);

  int err = chain.decode(intermediate.data(),
			 output.data(),
			 input.size());

  BOOST_CHECK(err==0);
  BOOST_CHECK_EQUAL_COLLECTIONS(input.begin(), input.end(), output.begin(), output.end());

}
BOOST_AUTO_TEST_SUITE_END()
