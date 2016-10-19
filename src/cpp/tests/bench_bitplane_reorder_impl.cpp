#define BOOST_TEST_MODULE BENCH_BITPLANE_REORDER
#include "boost/test/unit_test.hpp"

#include <vector>
#include <iostream>
#include <algorithm> // for copy
#include <iterator> // for ostream_iterator
#include "encoders/sqeazy_impl.hpp"
#include "sse_test_utils.hpp"



typedef const_anyvalue_fixture<(1 << 15), 0xff> default_cv_fixture; 
typedef ramp_fixture<(1 << 15)>			default_ramp_fixture; 



BOOST_FIXTURE_TEST_SUITE( bench, default_cv_fixture )

BOOST_AUTO_TEST_CASE( scalar_version ){


  int ret = 0;

  std::chrono::duration<double> time;
  
  for(int i = 0;i<20;++i){

    auto start = std::chrono::high_resolution_clock::now();
    ret = sqeazy::detail::scalar_bitplane_reorder_encode<1>(&input[0],
							    &output[0],
							    input.size());
    auto end = std::chrono::high_resolution_clock::now();

    BOOST_CHECK(ret==0);
    std::fill(output.begin(), output.end(),0);

    time += end-start;
  }

  BOOST_CHECK(time.count()>0);
  std::cout << "scalar version: " << time.count()/20 << " s\n";
  std::cout << "\t\t" << input.size()*sizeof(input[0])/(1024.*1024.) << " MB, "
	    << input.size()*sizeof(input[0])/(1024.*1024.)/(time.count()/20.)<<" MB/s\n";

}


BOOST_AUTO_TEST_CASE( sse_version ){

  int ret = 0;

  std::chrono::duration<double> time;
  
  for(int i = 0;i<20;++i){

    auto start = std::chrono::high_resolution_clock::now();
    ret = sqeazy::detail::sse_bitplane_reorder_encode<1>(&input[0],
							 &output[0],
							 input.size());
    auto end = std::chrono::high_resolution_clock::now();

    BOOST_CHECK(ret==0);
    std::fill(output.begin(), output.end(),0);

    time += end-start;
  }

  BOOST_CHECK(time.count()>0);
  std::cout << "sse version:    " << time.count()/20 << " s\n";
  std::cout << "\t\t" << input.size()*sizeof(input[0])/(1024.*1024.) << " MB, "
	    << input.size()*sizeof(input[0])/(1024.*1024.)/(time.count()/20.)<<" MB/s\n";

  
}

BOOST_AUTO_TEST_SUITE_END()
