#define BOOST_TEST_DYN_LINK 
#define BOOST_TEST_MODULE TEST_APPLY
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>

template <typename T>
struct add_one {

  
  T operator()( const T& _in) {
    return _in + 1;
  }

  void apply( const T* _in, T* _out, const unsigned long& _size) {
    const T* begin = _in;
    const T* end = begin + _size;
    
    for(;begin!=end;++begin, ++_out){
      *_out = *begin + 1;
    }
    
  }

  
};

typedef add_one<int> add_one_to_ints;
 
BOOST_AUTO_TEST_SUITE( access_test_suite )
   

BOOST_AUTO_TEST_CASE( without_applyer )
{
  std::vector<int> test_in(42,0);
  std::vector<int> test_out(42,0);
  
  std::transform(test_in.begin(), test_in.end(), test_out.begin(), add_one<int>() );
  
  BOOST_CHECK_EQUAL(std::accumulate(test_out.begin(), test_out.end(),0),42);

  std::fill(test_out.begin(), test_out.end(),0);
  
  add_one_to_ints test;
  test.apply(&test_in[0], &test_out[0], 42);
  
  BOOST_CHECK_EQUAL(std::accumulate(test_out.begin(), test_out.end(),0),42);
}

BOOST_AUTO_TEST_SUITE_END()
