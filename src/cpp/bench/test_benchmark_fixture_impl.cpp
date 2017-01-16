#define BOOST_TEST_MODULE TEST_BENCH_FIXTURES
#include "boost/test/unit_test.hpp"
#include "benchmark_fixtures.hpp"

BOOST_FIXTURE_TEST_SUITE( fundamentals , sqeazy::benchmark::static_synthetic_data<> )

BOOST_AUTO_TEST_CASE( fixture_can_be_setup ){

  BOOST_CHECK(!shape.empty());

}
BOOST_AUTO_TEST_SUITE_END()
