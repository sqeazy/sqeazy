#define BOOST_TEST_DYN_LINK 
#define BOOST_TEST_MODULE TEST_APPLY
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include "../src/pipeline.hpp"

template <typename T>
struct add_one {

  static std::string name() {
    
    return std::string("add_one");
    
  }
  
  
  T operator()( const T& _in) {
    return _in + 1;
  }

  static void apply( const T* _in, T* _out, const unsigned long& _size) {
    const T* begin = _in;
    const T* end = begin + _size;
    
//     for(;begin!=end;++begin, ++_out){
//       *_out = *begin + 1;
//     }
    add_one<T> adder;
    std::transform(begin, end, _out, adder);
    
  }

  
};

template <typename T >
struct square {

  static std::string name() {
    
    return std::string("square");
    
  }
  
  T operator()( const T& _in) {
    return _in * _in;
  }

  static void apply( const T* _in, T* _out, const unsigned long& _size) {
    const T* begin = _in;
    const T* end = begin + _size;
    
    square<T> operation;
    std::transform(begin, end, _out, operation);
    
  }

  
};

typedef add_one<int> add_one_to_ints;
 
BOOST_AUTO_TEST_SUITE( access_test_suite )
   

BOOST_AUTO_TEST_CASE( without_applyer )
{
  std::vector<int> test_in(42,1);
  std::vector<int> test_out(42,0);
  
  std::transform(test_in.begin(), test_in.end(), test_out.begin(), add_one<int>() );
  
  BOOST_CHECK_EQUAL(std::accumulate(test_out.begin(), test_out.end(),0),42*2);

  std::fill(test_out.begin(), test_out.end(),0);
  
  typedef sqeazy::bmpl::vector<add_one<int>, square<int> > test_pipe;
  typedef sqeazy::pipeline<int, test_pipe> current_pipe;
  
  BOOST_CHECK_NE(current_pipe::name().size(),0);
  
  BOOST_CHECK_NE(current_pipe::name().find("square"),std::string::npos);
  
  BOOST_CHECK_NE(current_pipe::name().find("add_one"),std::string::npos);
//   current_pipe::apply(&test_in[0], &test_out[0],test_in.size());
//   
//   BOOST_CHECK_EQUAL(test_in[0],1);
//   
//   BOOST_CHECK_EQUAL(test_out[0],4);
}

BOOST_AUTO_TEST_SUITE_END()
