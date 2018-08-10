#define BOOST_TEST_MODULE TEST_HEADER_TAG_IMPL
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"

#include <numeric>
#include <vector>
#include <iostream>
#include <sstream>
#include <bitset>

#include "header_tag.hpp"

using namespace sqeazy;

BOOST_AUTO_TEST_SUITE( fundamentals )


BOOST_AUTO_TEST_CASE( constructs_empty )
{
  header::tag tag;

  BOOST_CHECK(tag.pipename_.empty());
  BOOST_CHECK(tag.raw_type_id_.empty());
  BOOST_CHECK(tag.raw_shape_.empty());

}


BOOST_AUTO_TEST_CASE( constructs_fill )
{
  header::tag tag("empty_pipe",
                                "type42",
                                std::vector<std::size_t>{1,2,3},
                                100
    );

  BOOST_CHECK(tag.pipename_.size());
  BOOST_CHECK(tag.raw_type_id_.size());
  BOOST_CHECK(tag.raw_shape_.size());

}

BOOST_AUTO_TEST_CASE( assignment_fill )
{
  header::tag tag;

  header::tag expected("empty_pipe",
                                "type42",
                                std::vector<std::size_t>{1,2,3},
                                100
    );

  tag = expected;

  BOOST_CHECK(tag.pipename_.size());
  BOOST_CHECK(tag.raw_type_id_.size());
  BOOST_CHECK(tag.raw_shape_.size());

}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( roundtrips )


BOOST_AUTO_TEST_CASE( simple )
{

  header::tag expected("empty_pipe",
                                "type42",
                                std::vector<std::size_t>{1,2,3},
                                100
    );

  std::string serial = header::to_string(expected);

  header::tag received = header::from_string(serial);

  BOOST_CHECK(expected == received);

}

BOOST_AUTO_TEST_CASE( property_tag_cant_do_this )
{

  std::string problematic("quantiser(decode_lut_string=<verbatim>\\u0000@\\u0000\200\\u0000<\\/verbatim>)",71);

  header::tag expected(problematic,
                             "type42",
                             std::vector<std::size_t>{1,2,3},
                             100
    );

  std::string serial = header::to_string(expected);

  header::tag received = header::from_string(serial);

  BOOST_CHECK(expected == received);

}
BOOST_AUTO_TEST_SUITE_END()
