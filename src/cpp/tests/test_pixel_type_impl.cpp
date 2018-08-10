#define BOOST_TEST_MODULE TEST_PIXEL_TYPE_IMPL
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"

#include <string>
#include <cstdint>

#include "header_utils.hpp"


BOOST_AUTO_TEST_SUITE( forward )

BOOST_AUTO_TEST_CASE( eightbit )
{

  auto enc = sqeazy::header_utils::represent<std::uint8_t>::as_string();
  BOOST_CHECK_NE(enc.size(),0);
  BOOST_CHECK_EQUAL(enc,"uint8");

  BOOST_CHECK_EQUAL(sqeazy::header_utils::represent<std::int8_t>::as_string(),"int8");

  BOOST_CHECK_EQUAL(sqeazy::header_utils::sizeof_typeinfo("uint8"),1);
  BOOST_CHECK_EQUAL(sqeazy::header_utils::sizeof_typeinfo("int8"),1);
}

BOOST_AUTO_TEST_CASE( sixteenbit )
{

  auto enc = sqeazy::header_utils::represent<std::uint16_t>::as_string();
  BOOST_CHECK_NE(enc.size(),0);
  BOOST_CHECK_EQUAL(enc,"uint16");

  BOOST_CHECK_EQUAL(sqeazy::header_utils::represent<std::int16_t>::as_string(),"int16");
  BOOST_CHECK_EQUAL(sqeazy::header_utils::sizeof_typeinfo("uint16"),2);
  BOOST_CHECK_EQUAL(sqeazy::header_utils::sizeof_typeinfo("int16"),2);
}

BOOST_AUTO_TEST_CASE( thirtytwobit )
{

  auto enc = sqeazy::header_utils::represent<std::uint32_t>::as_string();
  BOOST_CHECK_NE(enc.size(),0);
  BOOST_CHECK_EQUAL(enc,"uint32");

  BOOST_CHECK_EQUAL(sqeazy::header_utils::represent<std::int32_t>::as_string(),"int32");
  BOOST_CHECK_EQUAL(sqeazy::header_utils::sizeof_typeinfo("uint32"),4);
  BOOST_CHECK_EQUAL(sqeazy::header_utils::sizeof_typeinfo("int32"),4);
}
BOOST_AUTO_TEST_SUITE_END()
