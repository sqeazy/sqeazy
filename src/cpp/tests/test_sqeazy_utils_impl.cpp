#define BOOST_TEST_MODULE TEST_SQEAZY_UTILS_IMPL
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <cstdint>

#include "sqeazy_utils.hpp"

namespace sqy = sqeazy;

BOOST_AUTO_TEST_SUITE( string_algs )

BOOST_AUTO_TEST_CASE( split_by_single_character )
{

  std::string to_test = "tell-me-what";
  std::string token = "-";
  std::vector<std::string> result = sqy::split(to_test,token);
  BOOST_CHECK_EQUAL(result.size(),3);
  
}

BOOST_AUTO_TEST_CASE( split_by_word )
{
  std::vector<std::string> data = {"tell","me","what"};
  std::string to_test = "tell->me->what";
  std::string token = "->";
  std::vector<std::string> result = sqy::split(to_test,token);
  BOOST_CHECK_EQUAL(result.size(),3);

  for(size_t i = 0;i<data.size();++i)
    BOOST_CHECK_EQUAL(result[i],data[i]);
    
  
}


BOOST_AUTO_TEST_SUITE_END()
