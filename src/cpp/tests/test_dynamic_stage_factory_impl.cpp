#define BOOST_TEST_MODULE TEST_DYNAMIC_STAGE_FACTORY
#include "boost/test/unit_test.hpp"

#include "test_dynamic_pipeline_impl.hpp"
#include "dynamic_stage_factory.hpp"

template <typename T>
using only_sum_up_factory = sqy::stage_factory<sum_up<T> >;


BOOST_AUTO_TEST_SUITE( factory_test_suite )

BOOST_AUTO_TEST_CASE( factory_finds_valid )
{

  auto local = int_factory_with_one_entry::create<int_filter>("add_one");
  auto result = sqy::const_stage_view(local);
  BOOST_CHECK_NE(result->name(),"");
  BOOST_CHECK_EQUAL(result->name(),"add_one");
    
}

BOOST_AUTO_TEST_CASE( factory_finds_valid_from_list )
{

  auto stage = int_factory::create<int_filter>("add_one");
  // std::shared_ptr<int_stage> result = ;
  auto result = sqy::const_stage_view(stage);
  BOOST_CHECK_NE(result->name(),"");
  BOOST_CHECK_EQUAL(result->name(),"add_one");
  BOOST_CHECK(int_factory::has("square"));
  
}


BOOST_AUTO_TEST_CASE( templated_factory_finds_valid )
{

  auto result = filter_factory<int>::create<int_filter>("square");
  BOOST_REQUIRE(result!=nullptr);
  BOOST_CHECK_NE(sqy::const_stage_view(result)->name(),"");
  BOOST_CHECK_EQUAL(sqy::const_stage_view(result)->name(),"square");
    
}


BOOST_AUTO_TEST_CASE( factory_finds_nothing )
{

  BOOST_CHECK(int_factory::create<int_filter>("dope")==nullptr);
    
}

BOOST_AUTO_TEST_CASE( factory_size )
{

  BOOST_CHECK_EQUAL(my_tail_factory<char>::size(),1);
  BOOST_CHECK_EQUAL(sink_factory<int>::size(),2);
  BOOST_CHECK_EQUAL(filter_factory<int>::size(),3);
    
}

BOOST_AUTO_TEST_CASE( factory_name_list )
{

  std::vector<std::string> names = sink_factory<int>::name_list();
  
  BOOST_CHECK_EQUAL(names.size(),sink_factory<int>::size());

  sum_up<int> sum_up_local;
  BOOST_CHECK_EQUAL(names.front(),sum_up_local.name());

  high_bits<int> high_bits_local;
  BOOST_CHECK_EQUAL(names.back(),high_bits_local.name());
}

BOOST_AUTO_TEST_CASE( factory_description )
{

  auto desc = sink_factory<int>::descriptions();
  
  BOOST_CHECK_EQUAL(desc.size(),sink_factory<int>::size());

}

BOOST_AUTO_TEST_CASE( factory_max_encoded_size )
{
  
  BOOST_CHECK_EQUAL(my_tail_factory<char>::max_encoded_size(2048),2048);
  BOOST_CHECK_EQUAL(sink_factory<char>::max_encoded_size(2048),2048);
  BOOST_CHECK_LT(only_sum_up_factory<char>::max_encoded_size(2048),2048);
  BOOST_CHECK_EQUAL(only_sum_up_factory<char>::max_encoded_size(2048),sizeof(sum_up<char>::result_type));
  
}

BOOST_AUTO_TEST_SUITE_END()
