#define BOOST_TEST_MODULE TEST_STRING_PARSERS
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <functional>
#include <cmath>
#include <vector>
#include <map>
#include <cstdint>
#include <sstream>
#include "string_parsers.hpp"

namespace sqy = sqeazy;

struct string_fixture
{
  
  std::string four_digits = "0->1->2->3";
  std::vector<std::string> four_digits_as_strings = {"0","1","2","3"};
  std::string four_commands = "ls(test=0)->cd(any=..1..)->cp(from=to2)->rm(what=4)";
  std::vector<std::string> four_keys = {"ls","cd","cp","rm"};
  std::vector<std::string> four_values = {"test=0","any=..1..","from=to2","what=4"};
  std::string four_keywords = "ls->cd->cp->rm";
  
};

BOOST_FIXTURE_TEST_SUITE( splitters, string_fixture )

BOOST_AUTO_TEST_CASE (split_to_strings) {
  auto splitted = sqy::split_by(four_digits.begin(),
				four_digits.end(),
				"->");
  BOOST_CHECK_EQUAL(splitted.empty(),
		    false);
  
  BOOST_CHECK_EQUAL_COLLECTIONS(splitted.begin(), splitted.end(),
				four_digits_as_strings.begin(), four_digits_as_strings.end());
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( parsing, string_fixture )

BOOST_AUTO_TEST_CASE (parse_digits) {
  auto parsed_to_map = sqy::unordered_parse_by(four_keywords.begin(),
				     four_keywords.end(),
				     "->");
  BOOST_CHECK_EQUAL(parsed_to_map.empty(),
		    false);
  BOOST_CHECK_EQUAL(parsed_to_map.size(),
		    four_keys.size());
  
}

BOOST_AUTO_TEST_CASE (parse_correctly) {
  auto parsed_to_map = sqy::unordered_parse_by(four_keywords.begin(),
				     four_keywords.end(),
				     "->");
  BOOST_REQUIRE_EQUAL(parsed_to_map.empty(),
		    false);

  int count = 0;
  for( auto & key : four_keys ){
    if(parsed_to_map.find(key) != parsed_to_map.end())
      count++;
  }

  BOOST_CHECK_EQUAL(count,
		    four_keys.size());
}

BOOST_AUTO_TEST_CASE (parse_preserves_order) {
  auto parsed_to_vec = sqy::parse_by(four_keywords.begin(),
				     four_keywords.end(),
				     "->");
  BOOST_REQUIRE_EQUAL(parsed_to_vec.empty(),
		      false);

  int count = 0;
  for( std::size_t i = 0;i<parsed_to_vec.size();++i ){
    BOOST_CHECK_EQUAL(parsed_to_vec[i].first,four_keys[i]) ;
  }

  
}

BOOST_AUTO_TEST_CASE (parse_preserves_order_and_value) {
  auto parsed_to_vec = sqy::parse_by(four_commands.begin(),
				     four_commands.end(),
				     "->");
  BOOST_REQUIRE_EQUAL(parsed_to_vec.empty(),
		      false);

  int count = 0;
  for( std::size_t i = 0;i<parsed_to_vec.size();++i ){
    BOOST_CHECK_EQUAL(parsed_to_vec[i].first,four_keys[i]) ;
    BOOST_CHECK_EQUAL(parsed_to_vec[i].second,four_values[i]) ;
  }

  
}
BOOST_AUTO_TEST_SUITE_END()


