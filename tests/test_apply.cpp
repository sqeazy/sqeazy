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

    typedef T raw_type;
    typedef T compressed_type;
    static const bool is_compressor = false;

    static std::string name() {

        return std::string("add_one");

    }


    T operator()( const T& _in) {
        return _in + 1;
    }

    static int encode( const T* _in, T* _out, const unsigned long& _size) {
        const T* begin = _in;
        const T* end = begin + _size;

//     for(;begin!=end;++begin, ++_out){
//       *_out = *begin + 1;
//     }
        add_one<T> adder;
        std::transform(begin, end, _out, adder);

        return 0;
    }


};

template <typename T >
struct square {

    typedef T raw_type;
    typedef T compressed_type;
    static const bool is_compressor = false;

    static std::string name() {

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
    std::vector<int> test_out(42,0);

    typedef sqeazy::bmpl::vector<add_one<int>, square<int> > test_pipe;
    typedef sqeazy::pipeline<test_pipe> current_pipe;

    BOOST_CHECK_NE(current_pipe::name().size(),0);

    BOOST_CHECK_NE(current_pipe::name().find("square"),std::string::npos);

    BOOST_CHECK_NE(current_pipe::name().find("add_one"),std::string::npos);

    unsigned long in_size = test_in.size();
    current_pipe::compress((const int*)&test_in[0], &test_out[0],in_size);

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
    typedef sqeazy::bmpl::vector<sqeazy::bitswap_scheme<char>, sqeazy::lz4_scheme<char,long> > test_pipe;
    typedef sqeazy::pipeline<test_pipe> current_pipe;

    std::string pipe_name = current_pipe::name();

    BOOST_CHECK_NE(pipe_name.size(),0);

    BOOST_CHECK_NE(pipe_name.find("lz4"),std::string::npos);

    BOOST_CHECK_NE(current_pipe::name().find("bitswap1"),std::string::npos);

    const char* input = reinterpret_cast<const char*>(&constant_cube[0]);
    char* output = reinterpret_cast<char*>(&to_play_with[0]);

    const unsigned local_size = uint8_cube_of_8::size;
    int ret = current_pipe::compress(input, output, local_size);

    BOOST_CHECK_NE(input[0],output[0]);
    BOOST_CHECK_NE(ret,42);
    const unsigned written_bytes = sqeazy::lz4_scheme<char,long>::last_num_encoded_bytes;
    BOOST_CHECK_NE(written_bytes,local_size);

}


BOOST_AUTO_TEST_CASE( plain_encode_decode_chars )
{
    typedef sqeazy::bmpl::vector<sqeazy::lz4_scheme<value_type> > test_pipe;
    typedef sqeazy::pipeline<test_pipe> current_pipe;


    char* output = reinterpret_cast<char*>(&to_play_with[0]);

    const unsigned local_size = size;
    int enc_ret = current_pipe::compress(&constant_cube[0], output, local_size);

    const unsigned written_bytes = current_pipe::last_num_encoded_bytes;
    char* temp  = new char[written_bytes];
    std::copy(output,output + written_bytes, temp);

    int dec_ret = current_pipe::decompress(temp, &to_play_with[0], written_bytes);
    BOOST_CHECK_EQUAL(enc_ret,0);
    BOOST_CHECK_EQUAL(dec_ret,0);

    delete [] temp;

    BOOST_CHECK_EQUAL_COLLECTIONS(&constant_cube[0],&constant_cube[0] + size,
                                  &to_play_with[0], &to_play_with[0] + size
                                 );
}

BOOST_AUTO_TEST_CASE( encode_decode_bitswap_chars )
{
    typedef sqeazy::bmpl::vector<sqeazy::bitswap_scheme<value_type>, sqeazy::lz4_scheme<value_type> > test_pipe;
    typedef sqeazy::pipeline<test_pipe> current_pipe;


    char* output = reinterpret_cast<char*>(&to_play_with[0]);

    const unsigned local_size = uint8_cube_of_8::size;
    int enc_ret = current_pipe::compress(&constant_cube[0], output, local_size);

    const unsigned written_bytes = current_pipe::last_num_encoded_bytes;
    char* temp  = new char[written_bytes];
    std::copy(output,output + written_bytes, temp);

    int dec_ret = current_pipe::decompress(temp, &to_play_with[0], written_bytes);
    BOOST_CHECK_EQUAL(enc_ret,0);
    BOOST_CHECK_EQUAL(dec_ret,0);

    delete [] temp;

    BOOST_CHECK_EQUAL_COLLECTIONS(&constant_cube[0],&constant_cube[0] + size,
                                  &to_play_with[0], &to_play_with[0] + size
                                 );
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( pipeline_on_shorts, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( plain_encode_decode_shorts )
{
    typedef sqeazy::bmpl::vector<sqeazy::lz4_scheme<value_type> > test_pipe;
    typedef sqeazy::pipeline<test_pipe> current_pipe;
    const unsigned local_size = size;

    char* output = reinterpret_cast<char*>(&to_play_with[0]);

    
    int enc_ret = current_pipe::compress(&constant_cube[0], output, local_size);
    BOOST_CHECK_EQUAL(enc_ret,0);
    
    const unsigned written_bytes = current_pipe::last_num_encoded_bytes;
    
    std::vector<char> temp(output,output + (2*written_bytes));
    

    int dec_ret = current_pipe::decompress(&temp[0], &to_play_with[0], written_bytes);
    
    BOOST_CHECK_EQUAL(dec_ret,0);
    

//     BOOST_CHECK_EQUAL_COLLECTIONS(&constant_cube[0],&constant_cube[0] + local_size,
//                                   &to_play_with[0], &to_play_with[0] + local_size
//                                  );
}

BOOST_AUTO_TEST_CASE( encode_decode_bitswap_shorts )
{

    typedef sqeazy::bmpl::vector<sqeazy::bitswap_scheme<value_type>, sqeazy::lz4_scheme<value_type> > test_pipe;
    typedef sqeazy::pipeline<test_pipe> current_pipe;

    char* output = reinterpret_cast<char*>(&to_play_with[0]);

    const unsigned local_size = size;
    int enc_ret = current_pipe::compress(&constant_cube[0], output, local_size);

    
    const unsigned written_bytes = current_pipe::last_num_encoded_bytes;
    std::vector<char> temp(output,output + written_bytes);
    
    int dec_ret = current_pipe::decompress(&temp[0], &to_play_with[0], written_bytes);
    BOOST_REQUIRE_EQUAL(enc_ret,0);
    BOOST_REQUIRE_EQUAL(dec_ret,0);

    BOOST_CHECK_EQUAL_COLLECTIONS(&constant_cube[0],&constant_cube[0] + size,
                                  &to_play_with[0], &to_play_with[0] + size
                                 );
}

BOOST_AUTO_TEST_CASE( encode_decode_bitswap_shorts_dims_input )
{

    typedef sqeazy::bmpl::vector<sqeazy::bitswap_scheme<value_type>, sqeazy::lz4_scheme<value_type> > test_pipe;
    typedef sqeazy::pipeline<test_pipe> current_pipe;

    char* output = reinterpret_cast<char*>(&to_play_with[0]);

    std::vector<int> dims(3);
    const unsigned local_axis_length = axis_length;
    std::fill(dims.begin(), dims.end(), local_axis_length);
//     const unsigned local_size = size;
    int enc_ret = current_pipe::compress(&constant_cube[0], output, dims);

    
    const unsigned written_bytes = current_pipe::last_num_encoded_bytes;
    std::vector<char> temp(output,output + written_bytes);
    
    int dec_ret = current_pipe::decompress(&temp[0], &to_play_with[0], written_bytes);
    BOOST_REQUIRE_EQUAL(enc_ret,0);
    BOOST_REQUIRE_EQUAL(dec_ret,0);

    BOOST_CHECK_EQUAL_COLLECTIONS(&constant_cube[0],&constant_cube[0] + size,
                                  &to_play_with[0], &to_play_with[0] + size
                                 );
}
BOOST_AUTO_TEST_SUITE_END()

