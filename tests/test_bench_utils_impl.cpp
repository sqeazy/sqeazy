#define BOOST_TEST_MODULE TEST_BENCH_UTILS_IMPL
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include <bitset>
#include <map>
#include "array_fixtures.hpp"
#include "../bench/bench_utils.hpp"

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;

BOOST_FIXTURE_TEST_SUITE( measurements, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( success )
{
  
  sqeazy_bench::bcase<value_type> testme; 
  
  BOOST_CHECK_EQUAL(testme.has_run(),false);
  BOOST_CHECK_EQUAL(testme.filename.empty(),true);
  
  sqeazy_bench::bcase<value_type> testme2("/tmp/something", &constant_cube[0], dims);
  BOOST_CHECK_EQUAL(testme2.has_run(),false);
  BOOST_CHECK_EQUAL(testme2.filename.empty(),false);
  testme2.stop(1024);
  BOOST_CHECK_MESSAGE(testme2.time_in_microseconds()>0.,"success received " << testme2.time_in_microseconds() << " ms");
  BOOST_CHECK_EQUAL(testme2.histogram.mean()==1,true);
  
}

BOOST_AUTO_TEST_CASE( copied )
{
   
  sqeazy_bench::bcase<value_type> testme("/tmp/something", &constant_cube[0], dims);
  
  sqeazy_bench::bcase<value_type> testme2 = testme;
  
  BOOST_CHECK_EQUAL(testme2.has_run(),false);
  BOOST_CHECK_EQUAL(testme2.filename.empty(),false);
  
  testme2.stop(1024);
  BOOST_CHECK_EQUAL(testme2.time_in_microseconds()>0.,true);
  BOOST_CHECK_EQUAL(testme2.histogram.mean(),testme.histogram.mean());
  
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( suite, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( success )
{
  
  sqeazy_bench::bsuite<value_type> testme; 
  BOOST_CHECK_EQUAL(testme.empty(),true);
  
}

BOOST_AUTO_TEST_CASE( load )
{
  sqeazy_bench::bcase<value_type> const_case("/tmp/something", &constant_cube[0], dims);
  sqeazy_bench::bcase<value_type> inc_case("/tmp/something", &incrementing_cube[0], dims);
  
  sqeazy_bench::bsuite<value_type> testme(2); 
  
  const_case.stop(42);
  inc_case.stop(42);
  
  testme.push_back(const_case);
  testme.push_back(inc_case);
  
  
  BOOST_CHECK_EQUAL(testme.size(),2);
  
  BOOST_CHECK_GT(boost::accumulators::mean(testme.speed_accumulator),0);
  
  
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE( head_tail_split ){
  
  std::string name_head = "/path/to/file";
  std::string name_tail = ".suffix";
  std::string name = name_head + name_tail;
  
  std::array<std::string,2> head_tail = sqeazy_bench::split_last_of(name, ".");
  
  BOOST_CHECK_EQUAL(head_tail[0],name_head);
  BOOST_CHECK_EQUAL(head_tail[1],name_tail);
  
  
}


BOOST_AUTO_TEST_CASE( pop_next ){
  
  std::string path = "/somewhere/to/go";
  std::vector<std::string> args = { "-v", "-r", path.c_str()};
  
  std::string success = sqeazy_bench::pop_next_if_contained(args,std::string("-r"));
  
  
  BOOST_CHECK_EQUAL(success.empty(),false);
  BOOST_CHECK_EQUAL(success,path);
  
  
  
}
