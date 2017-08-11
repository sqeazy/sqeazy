#define BOOST_TEST_MODULE TEST_APPLY
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"
#include <numeric>
#include <vector>
#include "pipeline.hpp"
#include "array_fixtures.hpp"
#include "encoders/sqeazy_impl.hpp"

template <typename T>
struct add_one {

  typedef T raw_type;
  typedef T compressed_type;
  static const bool is_sink = false;

  static std::string static_name() {

    return std::string("add_one");

  }


  T operator()( const T& _in) {
    return _in + 1;
  }

  static int static_encode( const T* _in, T* _out, const unsigned long& _size) {
    const T* begin = _in;
    const T* end = begin + _size;
   
    add_one<T> adder;
    std::transform(begin, end, _out, adder);

    return 0;
  }

  static const unsigned long static_max_encoded_size(unsigned long _size_bytes){
    return _size_bytes;
  }

};

template <typename T >
struct square {

  typedef T raw_type;
  typedef T compressed_type;
  static const bool is_sink = false;

  static std::string static_name() {

    return std::string("square");

  }

  T operator()( const T& _in) {
    return _in * _in;
  }

  static int static_encode( const T* _in, T* _out, const unsigned long& _size) {
    const T* begin = _in;
    const T* end = begin + _size;

    square<T> operation;
    std::transform(begin, end, _out, operation);

    return 0;
  }

  static const unsigned long static_max_encoded_size(unsigned long _size_bytes){
    return _size_bytes;
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

BOOST_AUTO_TEST_CASE (with_pipeline_apply) {

    std::vector<int> test_in(42,1);
    std::vector<int> test_out;

    typedef sqeazy::bmpl::vector<add_one<int>, square<int> > test_pipe;
    typedef sqeazy::pipeline<test_pipe> current_pipe;

    unsigned hdr_size_bytes = current_pipe::header_size(test_in.size());
    long expected_size_bytes = current_pipe::static_max_bytes_encoded(test_in.size()*sizeof(int), hdr_size_bytes);
    test_out.resize(std::ceil(expected_size_bytes/float(sizeof(int))));
    std::fill(test_out.begin(), test_out.end(),0);

    std::string pipe_name = current_pipe::static_name();

    BOOST_CHECK_NE(pipe_name.size(),0u);
    BOOST_CHECK_NE(pipe_name.find("square"),std::string::npos);
    BOOST_CHECK_NE(pipe_name.find("add_one"),std::string::npos);

    unsigned long encoded_bytes = test_in.size()*sizeof(int);
    size_t in_size = test_in.size();
    current_pipe::compress((const int*)&test_in[0], &test_out[0],in_size,encoded_bytes);

    BOOST_CHECK_EQUAL(encoded_bytes,test_in.size()*sizeof(int) + hdr_size_bytes);
    test_out.resize(encoded_bytes/sizeof(int));
    
    std::vector<int> expected(test_in.size(),4);

    unsigned shift = std::ceil(float(hdr_size_bytes)/sizeof(int));
    BOOST_CHECK_EQUAL_COLLECTIONS(test_out.begin() + shift,test_out.end(),
                                  expected.begin(),expected.end()
                                 );

}
BOOST_AUTO_TEST_SUITE_END()

#include "encoders/external_encoders.hpp"

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;
typedef sqeazy::array_fixture<unsigned char> uint8_cube_of_8;

BOOST_FIXTURE_TEST_SUITE( pipeline_on_chars, uint8_cube_of_8 )

BOOST_AUTO_TEST_CASE( encode_bitswap1 )
{
    typedef sqeazy::bmpl::vector<sqeazy::bitswap_scheme<value_type>, sqeazy::lz4_scheme<value_type> > test_pipe;
    typedef sqeazy::pipeline<test_pipe> current_pipe;

    std::string pipe_name = current_pipe::static_name();

    

    BOOST_CHECK_NE(pipe_name.size(),0u);
    BOOST_CHECK_NE(pipe_name.find("lz4"),std::string::npos);
    BOOST_CHECK_NE(pipe_name.find("bswap1"),std::string::npos);

    const value_type* input = reinterpret_cast<const value_type*>(&constant_cube[0]);

    long expected_size = current_pipe::static_max_bytes_encoded(size_in_byte, current_pipe::header_size(size));
    if(expected_size!=size_in_byte)
      to_play_with.resize(expected_size);

    char* output = reinterpret_cast<char*>(&to_play_with[0]);

    unsigned local_size = uint8_cube_of_8::size;
    unsigned written_bytes = 0;
    int ret = current_pipe::compress(input, output, local_size, written_bytes);

    BOOST_CHECK_NE(input[0],output[0]);
    BOOST_CHECK_NE(ret,42);

    BOOST_CHECK_NE(written_bytes,local_size);

}


BOOST_AUTO_TEST_CASE( plain_encode_decode_chars )
{
    typedef sqeazy::bmpl::vector<sqeazy::lz4_scheme<value_type> > test_pipe;
    typedef sqeazy::pipeline<test_pipe> current_pipe;

    to_play_with.resize(current_pipe::static_max_bytes_encoded(to_play_with.size()));
    char* output = reinterpret_cast<char*>(&to_play_with[0]);
    
    unsigned local_size = size;
    unsigned written_bytes=0;

    const value_type* input = &constant_cube[0];
    int enc_ret = current_pipe::compress(input, output, local_size, written_bytes);


    std::vector<char> temp(written_bytes);
    std::copy(output,output + written_bytes, temp.begin());

    to_play_with.resize(size);
    int dec_ret = current_pipe::decompress(&temp[0], &to_play_with[0], written_bytes);
    BOOST_CHECK_EQUAL(enc_ret,0);
    BOOST_CHECK_EQUAL(dec_ret,0);

    
    BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(), constant_cube.end(),
                                  &to_play_with[0], &to_play_with[0] + size
                                 );
}

BOOST_AUTO_TEST_CASE( encode_decode_bitswap_chars )
{
    typedef sqeazy::bmpl::vector<sqeazy::bitswap_scheme<value_type>, sqeazy::lz4_scheme<value_type> > test_pipe;
    typedef sqeazy::pipeline<test_pipe> current_pipe;

    long expected_size = current_pipe::static_max_bytes_encoded(size_in_byte, current_pipe::header_size(size));
    if(expected_size!=size_in_byte)
      to_play_with.resize(expected_size);

    char* output = reinterpret_cast<char*>(&to_play_with[0]);

    unsigned local_size = size;
    unsigned written_bytes=0;

    int enc_ret = current_pipe::compress(&constant_cube[0], output, local_size, written_bytes);

    std::vector<char> temp(output,output + written_bytes);

    int dec_ret = current_pipe::decompress(&temp[0], &to_play_with[0], written_bytes);
    BOOST_REQUIRE_EQUAL(enc_ret,0);
    BOOST_REQUIRE_EQUAL(dec_ret,0);


    BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(), constant_cube.end(),
                                  &to_play_with[0], &to_play_with[0] + size
                                 );
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( pipeline_on_shorts, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( plain_encode_decode_shorts )
{
    typedef sqeazy::bmpl::vector<sqeazy::lz4_scheme<value_type> > test_pipe;
    typedef sqeazy::pipeline<test_pipe> current_pipe;

    long expected_size = current_pipe::static_max_bytes_encoded(size_in_byte, current_pipe::header_size(size));
    if(expected_size!=size_in_byte)
      to_play_with.resize(expected_size);

    char* output = reinterpret_cast<char*>(&to_play_with[0]);

    unsigned local_size = size;
    unsigned written_bytes=0;
    int enc_ret = current_pipe::compress(&constant_cube[0], output, local_size,written_bytes);
    BOOST_CHECK_EQUAL(enc_ret,0);



    std::vector<char> temp(output,output + (2*written_bytes));


    int dec_ret = current_pipe::decompress(&temp[0], &to_play_with[0], written_bytes);

    BOOST_CHECK_EQUAL(dec_ret,0);


    BOOST_CHECK_EQUAL_COLLECTIONS(&constant_cube[0],&constant_cube[0] + local_size,
                                  &to_play_with[0], &to_play_with[0] + local_size
                                 );
}

BOOST_AUTO_TEST_CASE( encode_decode_bitswap_shorts )
{

    typedef sqeazy::bmpl::vector<sqeazy::bitswap_scheme<value_type>, sqeazy::lz4_scheme<value_type> > test_pipe;
    typedef sqeazy::pipeline<test_pipe> current_pipe;

    long hdr_size = current_pipe::header_size(size);
    long expected_size_byte = current_pipe::static_max_bytes_encoded(size_in_byte, hdr_size);
    
    std::vector<char> output(expected_size_byte,'z');

    const unsigned local_size = size;
    unsigned written_bytes = 0;
    int enc_ret = current_pipe::compress(&constant_cube[0], &output[0], local_size, written_bytes);

    BOOST_CHECK_GT(written_bytes,0u);

    std::vector<char> temp(output.begin(),output.begin() + written_bytes);
    unsigned temp_size = temp.size();
    int dec_ret = current_pipe::decompress(&temp[0], &to_play_with[0], temp_size);
    BOOST_REQUIRE_EQUAL(enc_ret,0);
    BOOST_REQUIRE_EQUAL(dec_ret,0);

    BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(), constant_cube.end(),
                                  to_play_with.begin(), to_play_with.begin() + size
                                 );
}

BOOST_AUTO_TEST_CASE( encode_decode_bitswap_shorts_dims_input )
{

    typedef sqeazy::bmpl::vector<sqeazy::bitswap_scheme<value_type>, sqeazy::lz4_scheme<value_type> > test_pipe;
    typedef sqeazy::pipeline<test_pipe> current_pipe;

    long hdr_size = current_pipe::header_size(size);
    long expected_size_byte = current_pipe::static_max_bytes_encoded(size_in_byte, hdr_size);
    std::vector<char> output(expected_size_byte,'z');


    std::vector<int> dims(3);
    const unsigned local_axis_length = axis_length;
    std::fill(dims.begin(), dims.end(), local_axis_length);

    //COMPRESS
    unsigned written_bytes = 0;
    int enc_ret = current_pipe::compress(&constant_cube[0], &output[0], dims,written_bytes);
    
    std::vector<char> temp(output.begin(),output.begin() + written_bytes);
    std::fill(to_play_with.begin(), to_play_with.end(),0);

    //DECOMPRESS
    int dec_ret = current_pipe::decompress(&temp[0], &to_play_with[0], written_bytes);
    BOOST_REQUIRE_EQUAL(enc_ret,0);
    BOOST_REQUIRE_EQUAL(dec_ret,0);

    BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(), constant_cube.end(),
                                  &to_play_with[0], &to_play_with[0] + size
                                 );
}

BOOST_AUTO_TEST_CASE( encode_decode_diff_shorts_dims_input )
{

    typedef sqeazy::bmpl::vector<sqeazy::diff_scheme<value_type>, sqeazy::lz4_scheme<value_type> > test_pipe;
    typedef sqeazy::pipeline<test_pipe> current_pipe;

    long expected_size = current_pipe::static_max_bytes_encoded(size_in_byte, current_pipe::header_size(size));
    if(expected_size!=size_in_byte)
      to_play_with.resize(expected_size);

    char* output = reinterpret_cast<char*>(&to_play_with[0]);

    std::vector<int> dims(3);
    unsigned local_axis_length = axis_length;
    std::fill(dims.begin(), dims.end(), local_axis_length);
//     const unsigned local_size = size;
    unsigned written_bytes = 0;
    int enc_ret = current_pipe::compress(&constant_cube[0], output, dims, written_bytes);



    std::vector<char> temp(output,output + written_bytes);

    int dec_ret = current_pipe::decompress(&temp[0], &to_play_with[0], written_bytes);
    BOOST_REQUIRE_EQUAL(enc_ret,0);
    BOOST_REQUIRE_EQUAL(dec_ret,0);

    BOOST_CHECK_EQUAL_COLLECTIONS(constant_cube.begin(), constant_cube.end(),
                                  &to_play_with[0], &to_play_with[0] + size
                                 );
}


BOOST_AUTO_TEST_CASE( encode_decode_diff_shorts_incrementing_input )
{

    typedef sqeazy::bmpl::vector<sqeazy::diff_scheme<value_type>, sqeazy::lz4_scheme<value_type> > test_pipe;
    typedef sqeazy::pipeline<test_pipe> current_pipe;

    long expected_size = current_pipe::static_max_bytes_encoded(size_in_byte, current_pipe::header_size(size));
    if(expected_size!=size_in_byte)
      to_play_with.resize(expected_size);

    char* output = reinterpret_cast<char*>(&to_play_with[0]);

    std::vector<int> dims(3);
    const unsigned local_axis_length = axis_length;
    std::fill(dims.begin(), dims.end(), local_axis_length);

    unsigned written_bytes = 0;
    int enc_ret = current_pipe::compress(&incrementing_cube[0], output, dims, written_bytes);

    std::vector<char> temp(output,output + written_bytes);

    int dec_ret = current_pipe::decompress(&temp[0], &to_play_with[0], written_bytes);
    BOOST_REQUIRE_EQUAL(enc_ret,0);
    BOOST_REQUIRE_EQUAL(dec_ret,0);

    BOOST_CHECK_EQUAL_COLLECTIONS(&incrementing_cube[0],&incrementing_cube[0] + size,
                                  &to_play_with[0], &to_play_with[0] + size
                                 );
}

BOOST_AUTO_TEST_CASE( encode_decode_diff_shorts_incrementing_input_last_pixels_on_line )
{

    typedef sqeazy::bmpl::vector<sqeazy::diff_scheme<value_type, sqeazy::last_pixels_on_line_neighborhood<> >,
            sqeazy::lz4_scheme<value_type> > test_pipe;
    typedef sqeazy::pipeline<test_pipe> current_pipe;

    long expected_size = current_pipe::static_max_bytes_encoded(size_in_byte, current_pipe::header_size(size));
    if(expected_size!=size_in_byte)
      to_play_with.resize(expected_size);
    char* output = reinterpret_cast<char*>(&to_play_with[0]);

    std::vector<int> dims(3);
    const unsigned local_axis_length = axis_length;
    std::fill(dims.begin(), dims.end(), local_axis_length);

    unsigned written_bytes = 0;
    int enc_ret = current_pipe::compress(&incrementing_cube[0], output, dims, written_bytes);

    std::vector<char> temp(output,output + written_bytes);

    int dec_ret = current_pipe::decompress(&temp[0], &to_play_with[0], written_bytes);
    BOOST_REQUIRE_EQUAL(enc_ret,0);
    BOOST_REQUIRE_EQUAL(dec_ret,0);

    BOOST_CHECK_EQUAL_COLLECTIONS(&incrementing_cube[0],&incrementing_cube[0] + size,
                                  &to_play_with[0], &to_play_with[0] + size
                                 );
}

BOOST_AUTO_TEST_CASE( encode_decode_diff_shorts_incrementing_onrow_types )
{

  typedef typename sqeazy::remove_unsigned<value_type>::type signed_value_type;
    typedef sqeazy::bmpl::vector<sqeazy::diff_scheme<value_type, sqeazy::last_pixels_on_line_neighborhood<> >,
				 sqeazy::bitswap_scheme<value_type,1> > const_raw_type;
    typedef sqeazy::pipeline<const_raw_type> const_raw_type_pipe;

    typedef sqeazy::bmpl::vector<sqeazy::diff_scheme<value_type, sqeazy::last_pixels_on_line_neighborhood<> >,
				 sqeazy::bitswap_scheme<signed_value_type,1> > var_raw_type_;
    typedef sqeazy::pipeline<var_raw_type_> var_type_pipe;

    long expected_size = const_raw_type_pipe::static_max_bytes_encoded(size_in_byte, const_raw_type_pipe::header_size(size));
    if(expected_size!=size_in_byte)
      to_play_with.resize(expected_size);

    std::vector<value_type> const_result = to_play_with;
    std::vector<signed_value_type> var_result(to_play_with.begin(),to_play_with.end());

    int const_ret = const_raw_type_pipe::compress(&incrementing_cube[0], &const_result[0], dims);
    int var_ret = var_type_pipe::compress(&incrementing_cube[0], &var_result[0], dims);

    std::copy(var_result.begin(), var_result.end(), to_play_with.begin());
    BOOST_CHECK_EQUAL_COLLECTIONS(const_result.begin(), const_result.end(),
				  to_play_with.begin(), to_play_with.end());
    
    BOOST_CHECK_EQUAL(const_ret, var_ret);
}
BOOST_AUTO_TEST_SUITE_END()

