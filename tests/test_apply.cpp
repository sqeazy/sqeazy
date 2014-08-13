#define BOOST_TEST_DYN_LINK 
#define BOOST_TEST_MODULE TEST_APPLY
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include "../src/pipeline.hpp"
#include "array_fixtures.hpp"
#include "../src/sqeazy_impl.hpp"

template <typename T>
struct add_one {

  static std::string name() {
    
    return std::string("add_one");
    
  }
  
  
  T operator()( const T& _in) {
    return _in + 1;
  }

  static void encode( const T* _in, T* _out, const unsigned long& _size) {
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

  static void encode( const T* _in, T* _out, const unsigned long& _size) {
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

  
  
  std::copy(test_out.begin(), test_out.end(), test_in.begin());
  std::transform(test_in.begin(), test_in.end(), test_out.begin(), square<int>() );
  BOOST_CHECK_EQUAL(std::accumulate(test_out.begin(), test_out.end(),0),42*4);
}

BOOST_AUTO_TEST_CASE (with_pipeline_apply){

  std::vector<int> test_in(42,1);
  std::vector<int> test_out(42,0);
  
  typedef sqeazy::bmpl::vector<add_one<int>, square<int> > test_pipe;
  typedef sqeazy::pipeline<int, test_pipe> current_pipe;
  
  BOOST_CHECK_NE(current_pipe::name().size(),0);
  
  BOOST_CHECK_NE(current_pipe::name().find("square"),std::string::npos);
  
  BOOST_CHECK_NE(current_pipe::name().find("add_one"),std::string::npos);
  current_pipe::encode(&test_in[0], &test_out[0],test_in.size());
  
  BOOST_CHECK_EQUAL(test_in[0],1);
  
  std::vector<int> expected(42,4);
  
  BOOST_CHECK_EQUAL_COLLECTIONS(test_out.begin(),test_out.end(),
    expected.begin(),expected.end()
  );
  
}
BOOST_AUTO_TEST_SUITE_END()

#include "../src/external_encoders.hpp"

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;
typedef sqeazy::array_fixture<unsigned char> uint8_cube_of_8;

BOOST_FIXTURE_TEST_SUITE( pipeline_on_chars, uint8_cube_of_8 )

BOOST_AUTO_TEST_CASE( encode_bitswap1 )
{
  typedef sqeazy::bmpl::vector<sqeazy::bitswap_scheme<char,8>, sqeazy::lz4_scheme<char,long> > test_pipe;
  typedef sqeazy::pipeline<char, test_pipe> current_pipe;
  
  std::string pipe_name = current_pipe::name();
  
  BOOST_CHECK_NE(pipe_name.size(),0);
  
  BOOST_CHECK_NE(pipe_name.find("lz4"),std::string::npos);
  
  BOOST_CHECK_NE(current_pipe::name().find("bitswap1"),std::string::npos);
  
  const char* input = reinterpret_cast<const char*>(&constant_cube[0]);
  char* output = reinterpret_cast<char*>(&to_play_with[0]);
  
  const unsigned local_size = uint8_cube_of_8::size;
  int ret = current_pipe::encode(input, output, local_size);
  
  BOOST_CHECK_NE(input[0],output[0]);
  BOOST_CHECK_NE(ret,42);
  const unsigned written_bytes = sqeazy::lz4_scheme<char,long>::last_num_encoded_bytes;
  BOOST_CHECK_NE(written_bytes,local_size);
  
}

BOOST_AUTO_TEST_CASE( encode_decode_bitswap1 )
{
  typedef sqeazy::bmpl::vector<sqeazy::lz4_scheme<char,long>, sqeazy::bitswap_scheme<char,8> > test_pipe;
  typedef sqeazy::pipeline<char, test_pipe> current_pipe;
  
  const char* input = reinterpret_cast<const char*>(&constant_cube[0]);
  char* output = reinterpret_cast<char*>(&to_play_with[0]);
  
  const unsigned local_size = uint8_cube_of_8::size;
  int enc_ret = current_pipe::encode(input, output, local_size);
  
  const unsigned written_bytes = sqeazy::lz4_scheme<char,long>::last_num_encoded_bytes;
  char* temp  = new char[written_bytes];
  std::copy(output,output + written_bytes, temp);
  
  int dec_ret = current_pipe::decode(temp, output, written_bytes);
  BOOST_CHECK_NE(enc_ret,1);
  BOOST_CHECK_NE(dec_ret,1);
  
  delete [] temp;
  BOOST_CHECK_EQUAL_COLLECTIONS(input,input + local_size,
    output, output + local_size
  );
}
BOOST_AUTO_TEST_SUITE_END()