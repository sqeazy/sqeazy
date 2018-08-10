#define BOOST_TEST_MODULE TEST_DYNAMIC_PIPELINE
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"
#include <numeric>
#include <functional>
#include <cmath>
#include <vector>
#include <cstdint>
#include <sstream>
//#include "array_fixtures.hpp"
#include "encoders/sqeazy_impl.hpp"
#include "dynamic_pipeline.hpp"
#include "test_dynamic_pipeline_impl.hpp"

namespace sqy = sqeazy;

namespace sqeazy_testing {
  template <typename T>
  using dynamic_pipeline = sqy::dynamic_pipeline<T,filter_factory, sink_factory<T> >;
}

BOOST_AUTO_TEST_SUITE( access_test_suite )


BOOST_AUTO_TEST_CASE( without_pipeline )
{
    std::vector<int> test_in(42,1);
    std::vector<int> test_out(42,0);

    add_one<int> functor;
    std::transform(test_in.begin(), test_in.end(), test_out.begin(), functor );

    BOOST_CHECK_EQUAL(std::accumulate(test_out.begin(), test_out.end(),0),42*2);

    std::copy(test_out.begin(), test_out.end(), test_in.begin());
    std::transform(test_in.begin(), test_in.end(), test_out.begin(), square<int>() );
    BOOST_CHECK_EQUAL(std::accumulate(test_out.begin(), test_out.end(),0),42*4);
}

BOOST_AUTO_TEST_CASE (construct) {

  sqy::dynamic_pipeline<int> empty_pipe;
  BOOST_CHECK_EQUAL(empty_pipe.size(), 0u);
  BOOST_CHECK_EQUAL(empty_pipe.size(), 0u);
  BOOST_CHECK_EQUAL(empty_pipe.empty(), true);


}

BOOST_AUTO_TEST_CASE (copy_construct) {


  sqy::dynamic_pipeline<int> init_pipe{adder_sptr};// ,sum_em};

  BOOST_CHECK_EQUAL(init_pipe.size(), 1u);

  sqy::dynamic_pipeline<int> filled_pipe{adder_sptr,square_sptr};
  BOOST_CHECK_EQUAL(filled_pipe.size(), 2u);

  sqy::dynamic_pipeline<int> assigned_pipe(filled_pipe);
  BOOST_CHECK_EQUAL(assigned_pipe.size(), 2u);
  BOOST_CHECK_NE(assigned_pipe.empty(), true);

  sqy::dynamic_pipeline<int> copied_pipe(filled_pipe);
  BOOST_CHECK_EQUAL(copied_pipe.size(), 2u);
  BOOST_CHECK_NE(copied_pipe.empty(), true);

  sqy::dynamic_pipeline<int> sink_pipe(filled_pipe);
  sink_pipe.add(summer_sptr);
  BOOST_CHECK_EQUAL(sink_pipe.size(), 3u);
  BOOST_CHECK_NE(sink_pipe.empty(), true);
  BOOST_CHECK(sink_pipe.is_compressor());

  sqeazy_testing::dynamic_pipeline<int> rhs = {adder_sptr,square_sptr};
  sqy::dynamic_pipeline<int> copied_from_rhs(rhs);
  BOOST_CHECK_EQUAL(copied_from_rhs.size(), 2u);
  BOOST_CHECK_NE(copied_from_rhs.empty(), true);

}

BOOST_AUTO_TEST_CASE (assign_construct) {

  sqy::dynamic_pipeline<int> filled_pipe{adder_sptr,square_sptr};
  BOOST_CHECK_EQUAL(filled_pipe.size(), 2u);
  BOOST_CHECK(!filled_pipe.is_compressor());

  sqy::dynamic_pipeline<int> assigned_pipe = (filled_pipe);
  BOOST_CHECK_EQUAL(assigned_pipe.size(), 2u);
  BOOST_CHECK_NE(assigned_pipe.empty(), true);

  sqy::dynamic_pipeline<int> copied_pipe = (filled_pipe);
  BOOST_CHECK_EQUAL(copied_pipe.size(), 2u);
  BOOST_CHECK_NE(copied_pipe.empty(), true);

  sqy::dynamic_pipeline<int> sink_pipe = (filled_pipe);
  sink_pipe.add(summer_sptr);
  BOOST_CHECK_EQUAL(sink_pipe.size(), 3u);
  BOOST_CHECK_NE(sink_pipe.empty(), true);
  BOOST_CHECK(sink_pipe.is_compressor());

  sqeazy_testing::dynamic_pipeline<int> rhs = {adder_sptr,square_sptr};
  sqy::dynamic_pipeline<int> copied_from_rhs = rhs;
  BOOST_CHECK_EQUAL(copied_from_rhs.size(), 2u);
  BOOST_CHECK_NE(copied_from_rhs.empty(), true);
}



BOOST_AUTO_TEST_CASE (types_match) {

  sqy::dynamic_pipeline<int> empty_step;
  BOOST_CHECK_NE(empty_step.valid_filters(),true);

  sqy::dynamic_pipeline<int> single_step = {adder_sptr};
  BOOST_CHECK(single_step.valid_filters());

  sqy::dynamic_pipeline<int> working_pipe = {adder_sptr,square_sptr};
  BOOST_CHECK(working_pipe.valid_filters());

  //this does not compile
  // sqy::dynamic_pipeline<int> failing_pipe = {add_one<int>(),square<char>()};
  // BOOST_CHECK_NE(failing_pipe.valid_filters(),true);

  //this does not compile
  // sqy::dynamic_pipeline<int> working_pipe_with_sink = {add_one<int>(),
  //                               square<int>(),
  //                               sum_up<int>()};
  // BOOST_CHECK(working_pipe_with_sink.valid_filters());

}

BOOST_AUTO_TEST_CASE (name_given) {

  sqy::dynamic_pipeline<int> empty_step;
  BOOST_CHECK_EQUAL(empty_step.name(),"");


  sqy::dynamic_pipeline<int> working_pipe = {adder_sptr,square_sptr};
  BOOST_CHECK_EQUAL(working_pipe.name(),"add_one->square");

}

BOOST_AUTO_TEST_CASE (input_type) {

  sqy::dynamic_pipeline<int> empty_step;
  BOOST_CHECK_EQUAL(empty_step.input_type(),"");

  sqy::dynamic_pipeline<int> working_pipe = {adder_sptr,square_sptr};
  BOOST_CHECK_EQUAL(working_pipe.input_type(),sqeazy::header_utils::represent<int>::as_string());

}

BOOST_AUTO_TEST_CASE (output_type) {

  sqy::dynamic_pipeline<int> empty_step;
  BOOST_CHECK_EQUAL(empty_step.output_type(),"");

  sqy::dynamic_pipeline<int> working_pipe = {adder_sptr,square_sptr};
  BOOST_CHECK_EQUAL(working_pipe.output_type(),sqeazy::header_utils::represent<int>::as_string());

}

BOOST_AUTO_TEST_CASE (add) {

  sqy::dynamic_pipeline<int> empty_pipe;
  BOOST_CHECK_EQUAL(empty_pipe.output_type(),"");

  empty_pipe.add(adder_sptr);
  empty_pipe.add(summer_sptr);

  BOOST_CHECK_EQUAL(empty_pipe.empty(),false);
  BOOST_CHECK_EQUAL(empty_pipe.size(),2u);

}

BOOST_AUTO_TEST_CASE (clear) {

  sqy::dynamic_pipeline<int> pipe = {adder_sptr,square_sptr};
  pipe.clear();
  BOOST_CHECK_EQUAL(pipe.empty(),true);
  BOOST_CHECK_EQUAL(pipe.size(),0u);

}

BOOST_AUTO_TEST_CASE (bootstrap) {

  sqeazy_testing::dynamic_pipeline<int> filter_pipe;
  BOOST_CHECK_EQUAL(filter_pipe.output_type(),"");

  filter_pipe = sqeazy_testing::dynamic_pipeline<int>::from_string("add_one->square");
  BOOST_CHECK_NE(filter_pipe.empty(),true);
  BOOST_CHECK_EQUAL(filter_pipe.size(),2u);


  sqeazy_testing::dynamic_pipeline<int> sink_pipe;
  BOOST_CHECK_EQUAL(sink_pipe.output_type(),"");

  sink_pipe = sqeazy_testing::dynamic_pipeline<int>::from_string("add_one->sum_up");
  BOOST_CHECK_NE(sink_pipe.empty(),true);
  BOOST_CHECK_EQUAL(sink_pipe.size(),2u);
  BOOST_CHECK_EQUAL(sink_pipe.is_compressor(),true);
  BOOST_CHECK_EQUAL(sink_pipe.name(),"add_one->sum_up");

  // sqy::dynamic_pipeline<int> empty_pipe;
  // empty_pipe = sqeazy_testing::dynamic_pipeline<int>::from_string("add_one->square");
  // BOOST_CHECK_EQUAL(empty_pipe.empty(),true);
}

BOOST_AUTO_TEST_CASE (max_encoded_size) {

  const size_t artificial_size_bytes = 1000*sizeof(int);
  auto filter_pipe = sqeazy_testing::dynamic_pipeline<int>::from_string("add_one->square");
  size_t result = filter_pipe.max_encoded_size(artificial_size_bytes);

  BOOST_CHECK_GT(result,artificial_size_bytes);

  auto sink_pipe = sqeazy_testing::dynamic_pipeline<int>::from_string("add_one->sum_up");
  result = sink_pipe.max_encoded_size(artificial_size_bytes);
  BOOST_CHECK_GT(result,artificial_size_bytes);

  sink_pipe = sqeazy_testing::dynamic_pipeline<int>::from_string("sum_up");
  result = sink_pipe.max_encoded_size(artificial_size_bytes);
  BOOST_CHECK_LT(result,artificial_size_bytes);
}

BOOST_AUTO_TEST_CASE (n_threads_defaults_to_1) {
  auto sink_pipe = sqeazy_testing::dynamic_pipeline<int>::from_string("add_one->sum_up");
  BOOST_CHECK_EQUAL(sink_pipe.n_threads(),1u);
}

BOOST_AUTO_TEST_CASE (n_threads_can_be_set) {
  auto sink_pipe = sqeazy_testing::dynamic_pipeline<int>::from_string("add_one->sum_up");
  sink_pipe.set_n_threads(2);
  BOOST_CHECK_EQUAL(sink_pipe.n_threads(),2u);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( compression_related )


BOOST_AUTO_TEST_CASE (encode_with_filters) {

  std::vector<int> input(10);
  std::iota(input.begin(), input.end(),0);
  BOOST_CHECK_EQUAL(input.front(),0);
  BOOST_CHECK_EQUAL(input.back(),9);

  auto filters_pipe = sqeazy_testing::dynamic_pipeline<int>::from_string("square");

  std::size_t max_encoded_size_byte = filters_pipe.max_encoded_size(input.size()*sizeof(int));
  std::vector<int> intermediate(max_encoded_size_byte/sizeof(int),0);

  char* char_end = filters_pipe.encode(input.data(),
                       (char*)intermediate.data(),
                       input.size());

  BOOST_REQUIRE(char_end!=nullptr);
  size_t bytes_encoded = char_end - (char*)intermediate.data();
  intermediate.resize(bytes_encoded/sizeof(int));

  int* encoded_end = reinterpret_cast<int*>(char_end);

  BOOST_CHECK_EQUAL(intermediate.back(),std::pow(9,2));
  BOOST_CHECK_EQUAL(*(encoded_end - input.size()),std::pow(0,2));
  BOOST_CHECK_EQUAL(*(encoded_end - input.size()+4),std::pow(4,2));
  BOOST_CHECK(encoded_end!=nullptr);
  BOOST_CHECK_EQUAL(std::distance(&intermediate[0],encoded_end),std::ptrdiff_t(intermediate.size()));
}

BOOST_AUTO_TEST_CASE (decode_with_filters) {

  std::vector<int> input(10,16);
  std::vector<int> output(input.size(),0);

  auto filters_pipe = sqeazy_testing::dynamic_pipeline<int>::from_string("square");

  std::size_t max_encoded_size_byte = filters_pipe.max_encoded_size(input.size()*sizeof(int));
  std::vector<char> intermediate(max_encoded_size_byte);

  auto encoded_end = filters_pipe.encode(input.data(),
                     intermediate.data(),
                     input.size());
  BOOST_CHECK(encoded_end!=nullptr);

  std::size_t encoded_size = encoded_end - intermediate.data();

  int err_code = filters_pipe.decode(intermediate.data(),
                     output.data(),
                     encoded_size);
  BOOST_CHECK_EQUAL(err_code,0);

  for(std::size_t i = 0;i<input.size();++i)
    BOOST_CHECK_EQUAL(output[i],input[i]);
}

BOOST_AUTO_TEST_CASE (encode_with_sink) {

  std::vector<int> input(10);
  std::iota(input.begin(), input.end(),0);
  BOOST_CHECK_EQUAL(input.front(),0);
  BOOST_CHECK_EQUAL(input.back(),9);

  auto sink_pipe = sqeazy_testing::dynamic_pipeline<int>::from_string("square->sum_up");

  int max_encoded_size = sink_pipe.max_encoded_size(input.size()*sizeof(int));
  BOOST_CHECK_NE(max_encoded_size,0u);
  BOOST_CHECK_GT(max_encoded_size,8u);

  std::vector<char> intermediate(max_encoded_size,0);

  char* encoded_end = sink_pipe.encode(input.data(),
                       intermediate.data(),
                       input.size());

  BOOST_CHECK_NE(intermediate.front(),0);
  BOOST_CHECK(encoded_end!=nullptr);
}

BOOST_AUTO_TEST_CASE (encode_with_hibit_sink) {

  std::vector<int> input(10);
  int start_value = (0xfff)+1;
  std::iota(input.begin(), input.end(),start_value);
  BOOST_CHECK_EQUAL(input.front(),start_value);
  BOOST_CHECK_EQUAL(input.back(),start_value+9);

  auto sink_pipe = sqeazy_testing::dynamic_pipeline<int>::from_string("add_one->high_bits");

  int max_encoded_size = sink_pipe.max_encoded_size(input.size()*sizeof(int));
  BOOST_CHECK_NE(max_encoded_size,0u);
  BOOST_CHECK_GT(max_encoded_size,8u);

  std::vector<char> intermediate(max_encoded_size,0);

  auto encoded_end = sink_pipe.encode(&input[0],&intermediate[0],input.size());

  BOOST_CHECK_NE(intermediate.front(),0);
  BOOST_CHECK(encoded_end!=nullptr);
}

BOOST_AUTO_TEST_CASE (encode_central_hibit_sink) {

  std::vector<int> input(10);
  int start_value = 0;
  std::iota(input.begin(), input.end(),start_value);
  for( int & i : input )
    i <<= 28;
  BOOST_CHECK_EQUAL(input.front(),start_value);
  BOOST_CHECK_EQUAL(input.back(),(start_value+9)<<28);

  auto sink_pipe = sqeazy_testing::dynamic_pipeline<int>::from_string("high_bits->square");

  BOOST_REQUIRE_EQUAL(sink_pipe.size(),2u);
  BOOST_REQUIRE_EQUAL(sink_pipe.valid_filters(),true);

  int max_encoded_size = sink_pipe.max_encoded_size(input.size()*sizeof(int));
  BOOST_CHECK_NE(max_encoded_size,0u);
  BOOST_CHECK_GT(max_encoded_size,8u);

  std::vector<char> intermediate(max_encoded_size,0);

  auto encoded_end = sink_pipe.encode(input.data(),
                      intermediate.data(),
                      input.size());

  BOOST_CHECK_NE(intermediate.front(),0);
  BOOST_CHECK(encoded_end!=nullptr);

  int to_square = start_value + 9;
  int expected = to_square*to_square;
  int received = *(encoded_end-1);

  BOOST_CHECK_NE(received,0);//happens if square is called before high_bits
  BOOST_CHECK_NE(received,to_square);//happens if square is skipped
  BOOST_CHECK_EQUAL(received,expected);
}


BOOST_AUTO_TEST_CASE (encode_with_sink_header) {

  std::vector<int> input(10);
  std::iota(input.begin(), input.end(),0);
  BOOST_CHECK_EQUAL(input.front(),0);
  BOOST_CHECK_EQUAL(input.back(),9);

  auto sink_pipe = sqeazy_testing::dynamic_pipeline<int>::from_string("square->sum_up");

  int max_encoded_size = sink_pipe.max_encoded_size(input.size()*sizeof(int));
  BOOST_CHECK_NE(max_encoded_size,0u);
  BOOST_CHECK_GT(max_encoded_size,8u);

  std::vector<char> intermediate(max_encoded_size,0);

  auto encoded_end = sink_pipe.encode(input.data(),intermediate.data(),input.size());
  BOOST_CHECK(encoded_end!=nullptr);
  std::vector<std::size_t> decoded_shape = sink_pipe.decoded_shape(intermediate.data(),
                                   encoded_end);

  BOOST_CHECK_NE(decoded_shape.empty(),true);
  BOOST_CHECK_EQUAL(decoded_shape.size(),1u);
  BOOST_CHECK_EQUAL(decoded_shape.front(),input.size());


}


BOOST_AUTO_TEST_CASE (decode_with_sink) {

  std::vector<int> input(8,4);
  std::vector<int> output(input.size(),0);

  auto sink_pipe = sqeazy_testing::dynamic_pipeline<int>::from_string("square->sum_up");
  std::size_t max_encoded_size = sink_pipe.max_encoded_size(input.size()*sizeof(int));

  std::vector<char> intermediate(max_encoded_size,0);
  auto encoded_end = sink_pipe.encode(input.data(),intermediate.data(),input.size());
  BOOST_REQUIRE(encoded_end!=nullptr);

  std::size_t intermediate_size = encoded_end - intermediate.data();
  BOOST_CHECK_LE(intermediate_size,max_encoded_size);

  int err_code = sink_pipe.decode(intermediate.data(),output.data(),intermediate_size);

  BOOST_CHECK_EQUAL(err_code,0);
  BOOST_CHECK_EQUAL(output.back(),input.back());
}

BOOST_AUTO_TEST_CASE (decode_with_hibit_sink) {

  std::vector<int> input(10);
  const int start_value = 0;
  std::iota(input.begin(), input.end(),start_value);
  for( int & i : input )
    i <<= 28;

  std::vector<int> output(input.size(),0);

  auto sink_pipe = sqeazy_testing::dynamic_pipeline<int>::from_string("high_bits");
  std::size_t max_encoded_size = sink_pipe.max_encoded_size(input.size()*sizeof(int));

  std::vector<char> intermediate(max_encoded_size,0);
  auto encoded_end = sink_pipe.encode(input.data(),intermediate.data(),input.size());
  BOOST_REQUIRE(encoded_end!=nullptr);

  std::size_t intermediate_size = encoded_end - intermediate.data();
  BOOST_CHECK_LE(intermediate_size,max_encoded_size);

  int err_code = sink_pipe.decode(intermediate.data(),output.data(),intermediate_size);

  BOOST_CHECK_EQUAL(err_code,0);
  BOOST_CHECK_EQUAL(output.back(),input.back());
}



BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( parameter_test_suite )
BOOST_AUTO_TEST_CASE (set_all_to_42) {

  std::vector<int> input(8,4);
  std::vector<int> output(input.size(),1);

  auto filters_pipe = sqeazy_testing::dynamic_pipeline<int>::from_string("set_to(value=42)");

  BOOST_CHECK_EQUAL(filters_pipe.empty(),false);

  std::size_t max_encoded_size_byte = filters_pipe.max_encoded_size(input.size()*sizeof(int));
  std::vector<char> intermediate(max_encoded_size_byte);

  char* encoded_end = filters_pipe.encode(input.data(),intermediate.data(),input.size());
  BOOST_CHECK(encoded_end!=nullptr);
  int* encoded_end_as_int = reinterpret_cast<int*>(encoded_end);
  BOOST_CHECK_EQUAL(*(encoded_end_as_int-1),42);

  // std::size_t intermediate_size = encoded_end - intermediate.data();
  // int err_code = filters_pipe.decode(intermediate.data(),output.data(),intermediate_size);
  // BOOST_CHECK_EQUAL(err_code,0);
  // BOOST_CHECK_EQUAL(output.back(),0);
  // BOOST_CHECK_EQUAL(output.front(),0);

}

BOOST_AUTO_TEST_CASE (set_all_to_42_and_sum) {

  std::vector<int> input(8,4);
  std::vector<int> output(input.size(),0);

  auto sink_pipe = sqeazy_testing::dynamic_pipeline<int>::from_string("set_to(value=42)->sum_up");

  BOOST_CHECK_EQUAL(sink_pipe.empty(),false);

  std::size_t max_encoded_size_byte = sink_pipe.max_encoded_size(input.size()*sizeof(int));
  std::vector<char> intermediate(max_encoded_size_byte);

  auto encoded_end = sink_pipe.encode(input.data(),intermediate.data(),input.size());
  BOOST_CHECK(encoded_end!=nullptr);

  std::string buffer(intermediate.data(),encoded_end);
  size_t pos = buffer.rfind(sqeazy::header::header_end_delimeter());;

  BOOST_CHECK_NE(pos,std::string::npos);
  BOOST_CHECK_NE(pos,buffer.size()-sqeazy::header::header_end_delimeter().size());

  pos+=sqeazy::header::header_end_delimeter().size() ;

  std::uint64_t result = *reinterpret_cast<std::uint64_t*>(&buffer[pos]);
  BOOST_CHECK_NE(result,0);
  BOOST_CHECK_EQUAL(result,42*input.size());

}

BOOST_AUTO_TEST_CASE (set_all_to_42_and_decode_to_first_value) {

  std::vector<int> input(8,4);
  std::vector<int> output(8,0);

  auto filters_pipe = sqeazy_testing::dynamic_pipeline<int>::from_string("set_to(value=42)");
  BOOST_CHECK_EQUAL(filters_pipe.empty(),false);

  std::size_t max_encoded_size_byte = filters_pipe.max_encoded_size(input.size()*sizeof(int));
  std::vector<char> intermediate(max_encoded_size_byte);

  char* encoded_end = filters_pipe.encode(input.data(),intermediate.data(),input.size());
  BOOST_CHECK(encoded_end!=nullptr);

  std::size_t intermediate_size = encoded_end - intermediate.data();
  int err_code = filters_pipe.decode(intermediate.data(),output.data(),intermediate_size);
  BOOST_CHECK_EQUAL(err_code,0);
  BOOST_CHECK_EQUAL(output.back(),4);
  BOOST_CHECK_EQUAL(output.front(),4);

}


BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( bootstrap_pipeline )
BOOST_AUTO_TEST_CASE (filters_only_same_name) {

  std::vector<int> input(8,4);
  std::vector<int> output(8,0);

  auto filters_pipe = sqeazy_testing::dynamic_pipeline<int>::from_string("set_to(value=42)");
  BOOST_CHECK_EQUAL(filters_pipe.empty(),false);

  std::size_t max_encoded_size_byte = filters_pipe.max_encoded_size(input.size()*sizeof(int));
  std::vector<char> intermediate(max_encoded_size_byte);

  char* encoded_end = filters_pipe.encode(input.data(),intermediate.data(),input.size());
  BOOST_CHECK(encoded_end!=nullptr);

  //extract header
  auto bootstrapped = sqeazy_testing::dynamic_pipeline<int>::bootstrap(intermediate.data(),
                                       encoded_end);

  BOOST_REQUIRE_EQUAL(bootstrapped.empty(),false);
  BOOST_CHECK_EQUAL(filters_pipe.name(),bootstrapped.name());
}

BOOST_AUTO_TEST_CASE (decode_from_bootstrap) {

  std::vector<int> input(8,4);
  std::vector<int> output(8,0);

  auto filters_pipe = sqeazy_testing::dynamic_pipeline<int>::from_string("set_to(value=42)");
  BOOST_CHECK_EQUAL(filters_pipe.empty(),false);

  std::size_t max_encoded_size_byte = filters_pipe.max_encoded_size(input.size()*sizeof(int));
  std::vector<char> intermediate(max_encoded_size_byte);

  char* encoded_end = filters_pipe.encode(input.data(),intermediate.data(),input.size());
  BOOST_CHECK(encoded_end!=nullptr);

  //extract header
  std::string buffer(intermediate.data(),
             encoded_end);
  auto bootstrapped = sqeazy_testing::dynamic_pipeline<int>::bootstrap(buffer);

  std::size_t intermediate_size = encoded_end - intermediate.data();
  int err_code = bootstrapped.decode(intermediate.data(),output.data(),intermediate_size);
  BOOST_CHECK_EQUAL(err_code,0);
  BOOST_CHECK_EQUAL(output.back(),4);
  BOOST_CHECK_EQUAL(output.front(),4);
}

BOOST_AUTO_TEST_CASE (sink_only_same_name) {

  std::vector<int> input(8,4);
  std::vector<int> output(8,0);

  auto sink_pipe = sqeazy_testing::dynamic_pipeline<int>::from_string("set_to(value=42)->sum_up");
  BOOST_CHECK_EQUAL(sink_pipe.empty(),false);

  std::size_t max_encoded_size_byte = sink_pipe.max_encoded_size(input.size()*sizeof(int));
  std::vector<char> intermediate(max_encoded_size_byte);

  auto encoded_end = sink_pipe.encode(input.data(),intermediate.data(),input.size());
  BOOST_CHECK(encoded_end!=nullptr);
  BOOST_CHECK_EQUAL(*(std::uint64_t*)(encoded_end-8),8*42);

  //extract header
  std::string buffer(intermediate.data(),
             encoded_end);
  auto bootstrapped = sqeazy_testing::dynamic_pipeline<int>::bootstrap(buffer);

  BOOST_CHECK_EQUAL(bootstrapped.empty(),false);
  BOOST_CHECK_EQUAL(sink_pipe.name(),bootstrapped.name());
}

BOOST_AUTO_TEST_CASE (sink_decode_from_bootstrap) {

  std::vector<int> input(8,4);
  std::vector<int> output(8,0);

  auto sink_pipe = sqeazy_testing::dynamic_pipeline<int>::from_string("set_to(value=42)->sum_up");
  BOOST_CHECK_EQUAL(sink_pipe.empty(),false);

  std::size_t max_encoded_size_byte = sink_pipe.max_encoded_size(input.size()*sizeof(int));
  std::vector<char> intermediate(max_encoded_size_byte);

  auto encoded_end = sink_pipe.encode(input.data(),intermediate.data(),input.size());
  BOOST_CHECK(encoded_end!=nullptr);
  BOOST_CHECK_EQUAL(*(std::uint64_t*)(encoded_end-8),8*42);

  //extract header
  std::string buffer(intermediate.data(),
             encoded_end);
  auto bootstrapped = sqeazy_testing::dynamic_pipeline<int>::bootstrap(buffer);

  std::size_t intermediate_size = encoded_end - intermediate.data();
  int err_code = bootstrapped.decode(intermediate.data(),output.data(),intermediate_size);
  BOOST_CHECK_EQUAL(err_code,0);
  BOOST_CHECK_EQUAL(output.back(),4);
  BOOST_CHECK_EQUAL(output.front(),4);
}

BOOST_AUTO_TEST_CASE (roundtrip_central_hibit_sink) {

  std::vector<int> input(10,0);
  //41073741824 = 4 << 28
  auto sink_pipe = sqeazy_testing::dynamic_pipeline<int>::from_string("set_to(value=1073741824)->high_bits->square");

  BOOST_REQUIRE_EQUAL(sink_pipe.size(),3u);
  BOOST_REQUIRE_EQUAL(sink_pipe.valid_filters(),true);

  int max_encoded_size = sink_pipe.max_encoded_size(input.size()*sizeof(int));
  BOOST_CHECK_NE(max_encoded_size,0u);
  BOOST_CHECK_GT(max_encoded_size,8u);

  std::vector<char> intermediate(max_encoded_size,0);

  auto encoded_end = sink_pipe.encode(input.data(),
                      intermediate.data(),
                      input.size());

  BOOST_CHECK_NE(intermediate.front(),0);
  BOOST_CHECK(encoded_end!=nullptr);

  std::uint32_t compressed_size = encoded_end - intermediate.data();
  const int expected = 4*4;
  int received = *(encoded_end-1);

  BOOST_REQUIRE_EQUAL(received,expected);

  std::vector<int> output(10,42);

  int err_code = sink_pipe.decode(intermediate.data(),
                  output.data(),
                  compressed_size);

  BOOST_CHECK_EQUAL(err_code,0);
  BOOST_CHECK_EQUAL(output.front(),input.front());
  BOOST_CHECK_EQUAL(output.back(),input.back());

}

BOOST_AUTO_TEST_CASE (string_validates) {

  std::string pipe = "high_bits";
  bool string_passes = sqeazy_testing::dynamic_pipeline<int>::can_be_built_from(pipe);
  BOOST_CHECK(string_passes);

  pipe = "add_one->high_bits";
  string_passes = sqeazy_testing::dynamic_pipeline<int>::can_be_built_from(pipe);
  BOOST_CHECK(string_passes);

  pipe = "add_one->high_bits->sum_up";
  string_passes = sqeazy_testing::dynamic_pipeline<int>::can_be_built_from(pipe);
  BOOST_CHECK(string_passes);

  pipe = "add_one->high_bits->sum_upper";
  string_passes = sqeazy_testing::dynamic_pipeline<int>::can_be_built_from(pipe);
  BOOST_CHECK(!string_passes);

}

BOOST_AUTO_TEST_CASE (string_validates_fixed_bugs) {

  std::string pipe = "add_one->high_bits!!sum_upper";
  bool string_passes = sqeazy_testing::dynamic_pipeline<int>::can_be_built_from(pipe);
  BOOST_CHECK(!string_passes);

  pipe = "add_one!!high_bits";
  string_passes = sqeazy_testing::dynamic_pipeline<int>::can_be_built_from(pipe);
  BOOST_CHECK(!string_passes);

}

BOOST_AUTO_TEST_CASE (test_factory_template_args) {

  std::string pipe = "add_one->high_bits->square";
  bool string_passes = sqy::dynamic_pipeline<int,
                         filter_factory,
                         sink_factory<int>,
                         my_tail_factory<char>
                         >::can_be_built_from(pipe);
  BOOST_CHECK(!string_passes);

  //FIXME: should not compile or not pass
  // string_passes = sqy::dynamic_pipeline<int,
  //                    filter_factory,
  //                    sink_factory<int>,
  //                    filter_factory<int>
  //                    >::can_be_built_from("add_one->high_bits->square");
  // BOOST_CHECK(!string_passes);

  pipe = "add_one->high_bits->square";
  string_passes = sqy::dynamic_pipeline<int,
                    filter_factory,
                    sink_factory<int>>::can_be_built_from(pipe);
  BOOST_CHECK(string_passes);

}
BOOST_AUTO_TEST_SUITE_END()
