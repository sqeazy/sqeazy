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

  sqy::stage_chain< sqy::filter<int> > local;
  BOOST_CHECK(local.empty());
    
}

BOOST_AUTO_TEST_SUITE_END()
