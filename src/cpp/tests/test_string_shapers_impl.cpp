#define BOOST_TEST_MODULE TEST_STRING_SHAPERS
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <functional>
#include <cmath>
#include <vector>
#include <map>
#include <chrono>
#include <cstdint>
#include <sstream>
#include "string_shapers.hpp"

namespace sqy = sqeazy;

struct string_fixture
{
  std::string one_liner = "Lorem ipsum dolor sit amet!";
  std::string two_liner = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque enim sapien, dignissim nec interdum id, ";
  std::string standard_text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque enim sapien, dignissim nec interdum id, dignissim nec felis. Nulla facilisi. Sed vel enim a diam aliquam euismod et ut lorem. Aliquam felis felis, ultricies interdum sapien nec, consectetur gravida est. Nulla facilisi. Sed et hendrerit sapien. Donec id pellentesque orci, ut accumsan erat. Cras euismod id risus a faucibus. Duis in neque et elit iaculis finibus ac vel urna. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; In nec nibh quis turpis luctus bibendum tempus vel nisl. Nullam massa mi, semper eu tellus eget, iaculis interdum erat. Curabitur sit amet molestie mauris. Integer molestie fringilla nisi, in tristique metus placerat eu. Nam vitae tristique leo. In nunc tortor, imperdiet vitae pharetra iaculis, pulvinar malesuada ex.";

};

BOOST_FIXTURE_TEST_SUITE( break_lines, string_fixture )

BOOST_AUTO_TEST_CASE (break_one_line_nonempty) {

  auto splitted = sqy::break_lines(one_liner.begin(),
                   one_liner.end(),
                   60);
  BOOST_CHECK_EQUAL(splitted.empty(),
            false);


}

BOOST_AUTO_TEST_CASE (break_one_word_nonempty) {

  std::string word = "Lorem";
  auto splitted = sqy::break_lines(word.begin(),
                   word.end(),
                   60);
  BOOST_CHECK_EQUAL(splitted.empty(),
            false);


}

BOOST_AUTO_TEST_CASE (break_two_line) {

  auto splitted = sqy::break_lines(two_liner.begin(),
                                   two_liner.end(),
                                   60);
  BOOST_CHECK_EQUAL(splitted.empty(),
                    false);
  BOOST_CHECK_EQUAL(splitted.size(),
                    2u);


}

BOOST_AUTO_TEST_CASE (break_text) {

  int line_size = 60;
  auto splitted = sqy::break_lines(standard_text.begin(),
                   standard_text.end(),
                   line_size);
  BOOST_CHECK_EQUAL(splitted.empty(),
                    false);
  BOOST_CHECK_GT(splitted.size(),
                 2u);

  BOOST_CHECK_EQUAL(splitted.size(),
                    14u);


}

BOOST_AUTO_TEST_SUITE_END()
