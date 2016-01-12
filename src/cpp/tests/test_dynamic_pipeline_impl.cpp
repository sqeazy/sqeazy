#define BOOST_TEST_MODULE TEST_APPLY
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <cstdint>
#include "../src/dynamic_pipeline.hpp"
#include "array_fixtures.hpp"
#include "../src/sqeazy_impl.hpp"

namespace sqy = sqeazy;

template <typename T>
struct add_one : public sqy::stage {

  typedef T raw_type;
  typedef T compressed_type;
  static const bool is_compressor = false;

  std::string name() const {

    return std::string("add_one");

  }


  T operator()( const T& _in) {
    return _in + 1;
  }

  static int encode( const T* _in, T* _out, const unsigned long& _size) {
    const T* begin = _in;
    const T* end = begin + _size;
   
    add_one<T> adder;
    std::transform(begin, end, _out, adder);

    return 0;
  }

  static const unsigned long max_encoded_size(unsigned long _size_bytes){
    return _size_bytes;
  }

  ~add_one(){};


  std::string input_type() const {

    return typeid(raw_type).name();
    
  }

  std::string output_type() const {

    return typeid(compressed_type).name();
    
  }
  
};

template <typename T >
struct square : public sqy::stage {

  typedef T raw_type;
  typedef T compressed_type;
  static const bool is_compressor = false;

  std::string name() const {

    return std::string("square");

  }

  T operator()( const T& _in) {
    return _in * _in;
  }

  static int encode( const T* _in, T* _out, const unsigned long& _size) {
    const T* begin = _in;
    const T* end = begin + _size;

    square<T> operation;
    std::transform(begin, end, _out, operation);

    return 0;
  }

  static const unsigned long max_encoded_size(unsigned long _size_bytes){
    return _size_bytes;
  }

  ~square(){};

  

  std::string input_type() const {

    return typeid(raw_type).name();
    
  }

  std::string output_type() const {

    return typeid(compressed_type).name();
    
  }
  
};

typedef add_one<int> add_one_to_ints;

BOOST_AUTO_TEST_SUITE( access_test_suite )




BOOST_AUTO_TEST_CASE( without_pipeline )
{
    std::vector<int> test_in(42,1);
    std::vector<int> test_out(42,0);

    std::transform(test_in.begin(), test_in.end(), test_out.begin(), add_one<int>() );

    BOOST_CHECK_EQUAL(std::accumulate(test_out.begin(), test_out.end(),0),42*2);

    std::copy(test_out.begin(), test_out.end(), test_in.begin());
    std::transform(test_in.begin(), test_in.end(), test_out.begin(), square<int>() );
    BOOST_CHECK_EQUAL(std::accumulate(test_out.begin(), test_out.end(),0),42*4);
}

BOOST_AUTO_TEST_CASE (construct) {

  sqy::dynamic_pipeline empty_pipe;
  BOOST_CHECK_EQUAL(empty_pipe.size(), 0);
  BOOST_CHECK_EQUAL(empty_pipe.size(), 0);
  BOOST_CHECK_EQUAL(empty_pipe.empty(), true);

 
  sqy::dynamic_pipeline setup_pipe = {std::make_shared<add_one<int> >(),std::make_shared<square<int> >()};
  BOOST_CHECK_EQUAL(setup_pipe.size(), 2);
  BOOST_CHECK_NE(setup_pipe.empty(), true);

  
}

BOOST_AUTO_TEST_CASE (copy_construct) {

  sqy::dynamic_pipeline filled_pipe = {std::make_shared<add_one<int> >(),std::make_shared<square<int> >()};
  BOOST_CHECK_EQUAL(filled_pipe.size(), 2);
  
  sqy::dynamic_pipeline assigned_pipe = filled_pipe;
  BOOST_CHECK_EQUAL(assigned_pipe.size(), 2);
  BOOST_CHECK_NE(assigned_pipe.empty(), true);

  sqy::dynamic_pipeline copied_pipe(filled_pipe);
  BOOST_CHECK_EQUAL(copied_pipe.size(), 2);
  BOOST_CHECK_NE(copied_pipe.empty(), true);

}

BOOST_AUTO_TEST_CASE (names_correct) {

  sqy::dynamic_pipeline setup_pipe = {std::make_shared<add_one<int> >(),std::make_shared<square<int> >()};

  for( auto i : setup_pipe )
    BOOST_CHECK_NE(i->name(), "void");

}

BOOST_AUTO_TEST_CASE (types_match) {

  sqy::dynamic_pipeline empty_step;
  BOOST_CHECK_NE(empty_step.valid_types(),true);
  
  sqy::dynamic_pipeline single_step = {std::make_shared<add_one<int> >()};
  BOOST_CHECK(single_step.valid_types());
  
  sqy::dynamic_pipeline working_pipe = {std::make_shared<add_one<int> >(),std::make_shared<square<int> >()};
  BOOST_CHECK(working_pipe.valid_types());
  
  sqy::dynamic_pipeline failing_pipe = {std::make_shared<add_one<int> >(),std::make_shared<square<char> >()};
  BOOST_CHECK_NE(failing_pipe.valid_types(),true);
  
}

BOOST_AUTO_TEST_CASE (name_given) {

  sqy::dynamic_pipeline empty_step;
  BOOST_CHECK_EQUAL(empty_step.name(),"");

  sqy::dynamic_pipeline working_pipe = {std::make_shared<add_one<int> >(),std::make_shared<square<int> >()};
  BOOST_CHECK_EQUAL(working_pipe.name(),"add_one->square");

}

BOOST_AUTO_TEST_CASE (input_type) {

  sqy::dynamic_pipeline empty_step;
  BOOST_CHECK_EQUAL(empty_step.input_type(),"");

  sqy::dynamic_pipeline working_pipe = {std::make_shared<add_one<int> >(),std::make_shared<square<int> >()};
  BOOST_CHECK_EQUAL(working_pipe.input_type(),typeid(int).name());

}

BOOST_AUTO_TEST_CASE (output_type) {

  sqy::dynamic_pipeline empty_step;
  BOOST_CHECK_EQUAL(empty_step.output_type(),"");

  sqy::dynamic_pipeline working_pipe = {std::make_shared<add_one<int> >(),std::make_shared<square<int> >()};
  BOOST_CHECK_EQUAL(working_pipe.output_type(),typeid(int).name());

}


BOOST_AUTO_TEST_CASE (push_back) {

  sqy::dynamic_pipeline pipe_to_fill;
  BOOST_CHECK_EQUAL(pipe_to_fill.empty(),true);
  BOOST_CHECK_NE(pipe_to_fill.valid_types(),true);

  pipe_to_fill.push_back(std::make_shared<add_one<int> >());
  BOOST_CHECK_NE(pipe_to_fill.empty(),true);
  BOOST_CHECK_NE(pipe_to_fill.size(),0);

  BOOST_CHECK_THROW(pipe_to_fill.push_back(std::make_shared<square<char> >()),std::runtime_error);
  pipe_to_fill.push_back(std::make_shared<square<int> >());
  BOOST_CHECK_EQUAL(pipe_to_fill.valid_types(),true);
}

BOOST_AUTO_TEST_SUITE_END()


// BOOST_AUTO_TEST_SUITE( stage_suite )


// BOOST_AUTO_TEST_CASE( stages_in_vector )
// {

//   BOOST_CHECK_EQUAL();
    
// }

// BOOST_AUTO_TEST_SUITE_END()
