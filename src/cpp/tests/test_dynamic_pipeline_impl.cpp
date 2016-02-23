#define BOOST_TEST_MODULE TEST_DYNAMIC_PIPELINE
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <functional>
#include <cmath>
#include <vector>
#include <cstdint>
#include <sstream>
#include "dynamic_pipeline.hpp"
#include "array_fixtures.hpp"
#include "sqeazy_impl.hpp"

namespace sqy = sqeazy;

template <typename T>
struct set_to :  public sqy::filter<T> {

  typedef T raw_type;
  typedef T compressed_type;
  
  raw_type value;
  raw_type first_value;
  
  static_assert(std::is_arithmetic<raw_type>::value==true,"[set_to]");
  
  set_to(const std::string& _payload=""):
    value(),
    first_value()
  {

    sqy::parsed_map_t parsed_map = sqy::unordered_parse_by(_payload.begin(), _payload.end());
    if(parsed_map.size()){

      if(parsed_map.find("value")!=parsed_map.end()){
	
	try{
	  value = std::stoi(parsed_map.find("value")->second);
	}
	catch(...){
	  std::cerr << "[set_to::constructor]\t unable to convert ." << parsed_map.find("value")->second << ". to number\n";
	}
      }

      if(parsed_map.find("first_value")!=parsed_map.end()){
	try{
	  first_value = std::stoi(parsed_map.find("first_value")->second);
	}
	catch(...){
	  std::cerr << "[set_to::constructor]\t unable to convert ." << parsed_map.find("first_value")->second << ". to number\n";
	}
      }
      
    }
    

    
  }

  std::string name() const {

    return std::string("set_to");

  }

  /**
     \brief serialize the parameters of this filter
     
     \return 
     \retval string .. that encodes the configuration paramters
     
  */
  std::string config() const {

    std::ostringstream cfg;
    cfg << "value=" << value << ",";
    cfg << "first_value=" << first_value;
    return cfg.str();

  }
  
  T operator()( const T& _in) {
    return value;
  }

  compressed_type* encode( const raw_type* _in, compressed_type* _out, std::vector<std::size_t> _shape) override final {

    std::size_t size = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());
    
    const raw_type* begin = _in;
    const raw_type* end = begin + size;
   
    if(size)
      first_value = *begin;
    
    std::transform(begin, end, _out, [&](raw_type _in){return this->value;});
    
    return _out+size;
  }

  int decode( const raw_type* _in, compressed_type* _out, std::vector<std::size_t> _shape) const override final {

    std::size_t size = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());
    
    const compressed_type* begin = _in;
    const compressed_type* end = begin + size;
   
    std::transform(begin, end, _out, [=](compressed_type _in){return first_value;});

    return 0;
  }
  
  const unsigned long max_encoded_size(unsigned long _size_bytes)  {
    return _size_bytes;
  }

  ~set_to(){};
  

  std::string output_type() const final override {

    return typeid(compressed_type).name();
    
  }

  bool is_compressor() const final override {
    
    return sqy::filter<T>::is_compressor;
    
  }

};

template <typename T>
struct add_one :  public sqy::filter<T> {

  typedef T raw_type;
  typedef T compressed_type;
  
  
  std::string name() const {

    return std::string("add_one");

  }

  std::string config() const {

    return std::string("");

  }  
  
  add_one(const std::string& _payload="")
  { }
  

  T operator()( const T& _in) {
    return _in + 1;
  }

  compressed_type* encode( const raw_type* _in, compressed_type* _out, std::vector<std::size_t> _shape) override final {

    std::size_t size = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());
    
    const raw_type* begin = _in;
    const raw_type* end = begin + size;
   
    
    std::transform(begin, end, _out, [](raw_type _in){return _in+1;});

    return _out+size;
  }

  int decode( const raw_type* _in, compressed_type* _out, std::vector<std::size_t> _shape) const override final {

    std::size_t size = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());
    
    const compressed_type* begin = _in;
    const compressed_type* end = begin + size;
   
    std::transform(begin, end, _out, [](compressed_type _in){return _in-1;});

    return 0;
  }
  
  const unsigned long max_encoded_size(unsigned long _size_bytes)  {
    return _size_bytes;
  }

  ~add_one(){};
  

  std::string input_type() const {

    return typeid(raw_type).name();
    
  }

  std::string output_type() const {

    return typeid(compressed_type).name();
    
  }

  bool is_compressor() const final override {
    
    return sqy::filter<T>::is_compressor;
    
  }

};

template <typename T >
struct square :  public sqy::filter<T> {

  typedef T raw_type;
  typedef T compressed_type;

  bool is_compressor() const final override {
    
    return sqy::filter<T>::is_compressor;
    
  }
  
  square(const std::string& _payload="")
  { }

  std::string name() const {

    return std::string("square");

  }

  
  std::string config() const {

    return std::string("");

  }  
  
  T operator()( const T& _in) {
    return _in * _in;
  }


  const unsigned long max_encoded_size(unsigned long _size_bytes){
    return _size_bytes;
  }

  ~square(){};

  

  std::string input_type() const {

    return typeid(raw_type).name();
    
  }

  std::string output_type() const {

    return typeid(compressed_type).name();
    
  }

   compressed_type*  encode( const raw_type* _in, compressed_type* _out, std::vector<std::size_t> _shape) override final {

    std::size_t size = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());
    
    const raw_type* begin = _in;
    const raw_type* end = _in + size;
   
    square<raw_type> operation;
    std::transform(begin, end, _out, operation);

    return (_out+size);
  }

  int decode( const raw_type* _in, compressed_type* _out, std::vector<std::size_t> _shape) const override final {

    std::size_t size = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());
    
    const compressed_type* begin = _in;
    const compressed_type* end = begin + size;
   
    std::transform(begin, end, _out, [](compressed_type _in){return std::sqrt(_in);});

    return 0;
  }

  
};

template <typename T >
struct sum_up :  public sqy::sink<T> {

  typedef T raw_type;
  typedef typename sqy::sink<T>::out_type compressed_type;
  typedef std::uint64_t result_type;

  std::intmax_t max_encoded_size(std::intmax_t _incoming_size_byte) const override final {
    return sizeof(result_type);
  }
  
  std::string name() const {

    return std::string("sum_up");

  }

  
  std::string config() const {

    return std::string("");

  }  
  
  sum_up(const std::string& _payload="")
  { }

  ~sum_up(){};
  

  std::string input_type() const {

    return typeid(raw_type).name();
    
  }

  std::string output_type() const {

    return typeid(compressed_type).name();
    
  }

  compressed_type* encode( const raw_type* _in, compressed_type* _out, std::vector<std::size_t> _shape) override final {

    std::size_t size = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());
    
    const raw_type* begin = _in;
    const raw_type* end = begin + size;

    result_type value = std::accumulate(begin, end, 0);

    *reinterpret_cast<result_type*>(_out) = value;
    // std::copy(reinterpret_cast<compressed_type*>(&value),
    // 	      reinterpret_cast<compressed_type*>(&value)+sizeof(result_type),
    // 	      _out);
    
    return _out+(sizeof(result_type)/sizeof(compressed_type));
  }

  int decode( const compressed_type* _in, raw_type* _out, std::vector<std::size_t> _shape) const override final {

    std::size_t size = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());
    
    raw_type* begin = _out;
    raw_type* end = begin + size;
    
    raw_type value = (*reinterpret_cast<const result_type*>(_in))/double(size);
    
    std::fill(begin, end, value);

    return 0;
  }

  bool is_compressor() const final override {
    
      return sqy::sink<T>::is_compressor;
    
  }

};

using add_one_to_ints = add_one<int> ;
using int_stage = sqy::stage<int>;
using int_filter = sqy::filter<int>;
using int_sink = sqy::sink<int>;

using int_factory_with_one_entry = sqy::stage_factory<add_one_to_ints> ;
using int_factory = sqy::stage_factory<add_one_to_ints, square<int> > ;
  
template <typename T>
using filter_factory = sqy::stage_factory<add_one<T>, square<T>, set_to<T> >;

template <typename T>
using sink_factory = sqy::stage_factory<sum_up<T> >;

static auto adder_sptr = std::make_shared<add_one<int>>(add_one<int>());
static auto square_sptr = std::make_shared<square<int>>(square<int>());
static auto summer_sptr = std::make_shared<sum_up<int>>(sum_up<int>());

BOOST_AUTO_TEST_SUITE( factory_test_suite )


  
BOOST_AUTO_TEST_CASE( factory_finds_valid )
{

  auto local = int_factory_with_one_entry::create<int_filter>("add_one");
  auto result = sqy::const_stage_view(local);
  BOOST_CHECK_NE(result->name(),"");
  BOOST_CHECK_EQUAL(result->name(),"add_one");
    
}

BOOST_AUTO_TEST_CASE( factory_finds_valid_from_list )
{

  auto stage = int_factory::create<int_filter>("add_one");
  // std::shared_ptr<int_stage> result = ;
  auto result = sqy::const_stage_view(stage);
  BOOST_CHECK_NE(result->name(),"");
  BOOST_CHECK_EQUAL(result->name(),"add_one");
  BOOST_CHECK(int_factory::has("square"));
  
}


BOOST_AUTO_TEST_CASE( templated_factory_finds_valid )
{

  auto result = filter_factory<int>::create<int_filter>("square");
  BOOST_REQUIRE(result!=nullptr);
  BOOST_CHECK_NE(sqy::const_stage_view(result)->name(),"");
  BOOST_CHECK_EQUAL(sqy::const_stage_view(result)->name(),"square");
    
}


BOOST_AUTO_TEST_CASE( factory_finds_nothing )
{

  BOOST_CHECK(int_factory::create<int_filter>("dope")==nullptr);
    
}

BOOST_AUTO_TEST_SUITE_END()


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
  BOOST_CHECK_EQUAL(empty_pipe.size(), 0);
  BOOST_CHECK_EQUAL(empty_pipe.size(), 0);
  BOOST_CHECK_EQUAL(empty_pipe.empty(), true);

  
}

BOOST_AUTO_TEST_CASE (copy_construct) {


  sqy::dynamic_pipeline<int> init_pipe{adder_sptr};// ,sum_em};
  
  BOOST_CHECK_EQUAL(init_pipe.size(), 1);
  
  sqy::dynamic_pipeline<int> filled_pipe{adder_sptr,square_sptr};
  BOOST_CHECK_EQUAL(filled_pipe.size(), 2);

  sqy::dynamic_pipeline<int> assigned_pipe(filled_pipe);
  BOOST_CHECK_EQUAL(assigned_pipe.size(), 2);
  BOOST_CHECK_NE(assigned_pipe.empty(), true);

  sqy::dynamic_pipeline<int> copied_pipe(filled_pipe);
  BOOST_CHECK_EQUAL(copied_pipe.size(), 2);
  BOOST_CHECK_NE(copied_pipe.empty(), true);

  sqy::dynamic_pipeline<int,std::uint32_t> sink_pipe(filled_pipe);
  sink_pipe.add(summer_sptr);
  BOOST_CHECK_EQUAL(sink_pipe.size(), 3);
  BOOST_CHECK_NE(sink_pipe.empty(), true);
  BOOST_CHECK(sink_pipe.is_compressor());
}

BOOST_AUTO_TEST_CASE (assign_construct) {

  sqy::dynamic_pipeline<int> filled_pipe{adder_sptr,square_sptr};
  BOOST_CHECK_EQUAL(filled_pipe.size(), 2);
  BOOST_CHECK(!filled_pipe.is_compressor());

  sqy::dynamic_pipeline<int> assigned_pipe = (filled_pipe);
  BOOST_CHECK_EQUAL(assigned_pipe.size(), 2);
  BOOST_CHECK_NE(assigned_pipe.empty(), true);

  sqy::dynamic_pipeline<int> copied_pipe = (filled_pipe);
  BOOST_CHECK_EQUAL(copied_pipe.size(), 2);
  BOOST_CHECK_NE(copied_pipe.empty(), true);

  sqy::dynamic_pipeline<int,std::uint32_t> sink_pipe = (filled_pipe);
  sink_pipe.add(summer_sptr);
  BOOST_CHECK_EQUAL(sink_pipe.size(), 3);
  BOOST_CHECK_NE(sink_pipe.empty(), true);
  BOOST_CHECK(sink_pipe.is_compressor());
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
  // 						       square<int>(),
  // 						       sum_up<int>()};
  // BOOST_CHECK(working_pipe_with_sink.valid_filters());
  
}

BOOST_AUTO_TEST_CASE (name_given) {

  sqy::dynamic_pipeline<int> empty_step;
  BOOST_CHECK_EQUAL(empty_step.name(),"");

    
  sqy::dynamic_pipeline<int> working_pipe = {adder_sptr,square_sptr};
  BOOST_CHECK_EQUAL(working_pipe.name(),"add_one->square");

  //this does not compile
  // sqy::dynamic_pipeline<int> compressing_pipe = {add_one<int>(),sum_up<int>()};
  // BOOST_CHECK_EQUAL(compressing_pipe.name(),"add_one->sum_up");

}

BOOST_AUTO_TEST_CASE (input_type) {

  sqy::dynamic_pipeline<int> empty_step;
  BOOST_CHECK_EQUAL(empty_step.input_type(),"");

  sqy::dynamic_pipeline<int> working_pipe = {adder_sptr,square_sptr};
  BOOST_CHECK_EQUAL(working_pipe.input_type(),typeid(int).name());

}

BOOST_AUTO_TEST_CASE (output_type) {

  sqy::dynamic_pipeline<int> empty_step;
  BOOST_CHECK_EQUAL(empty_step.output_type(),"");

  sqy::dynamic_pipeline<int> working_pipe = {adder_sptr,square_sptr};
  BOOST_CHECK_EQUAL(working_pipe.output_type(),typeid(int).name());

}

BOOST_AUTO_TEST_CASE (add) {

  sqy::dynamic_pipeline<int> empty_step;
  BOOST_CHECK_EQUAL(empty_step.output_type(),"");

  sqy::dynamic_pipeline<int>::filter_holder_t input = {adder_sptr,square_sptr};
  for(const auto& step : input)
    empty_step.add(step);
  
  BOOST_CHECK_EQUAL(empty_step.empty(),false);
  BOOST_CHECK_EQUAL(empty_step.size(),2);

}


BOOST_AUTO_TEST_CASE (bootstrap) {

  sqy::dynamic_pipeline<int> filter_pipe;
  BOOST_CHECK_EQUAL(filter_pipe.output_type(),"");

  filter_pipe = sqy::dynamic_pipeline<int>::from_string("add_one->square",
						 filter_factory<int>(),
						 sink_factory<int>());
  BOOST_CHECK_NE(filter_pipe.empty(),true);
  BOOST_CHECK_EQUAL(filter_pipe.size(),2);
  

  sqy::dynamic_pipeline<int,std::uint32_t> sink_pipe;
  BOOST_CHECK_EQUAL(sink_pipe.output_type(),"");

  sink_pipe = sqy::dynamic_pipeline<int, std::uint32_t>::from_string("add_one->sum_up",
							      filter_factory<int>(),
							      sink_factory<int>());
  BOOST_CHECK_NE(sink_pipe.empty(),true);
  BOOST_CHECK_EQUAL(sink_pipe.size(),2);
  BOOST_CHECK_EQUAL(sink_pipe.is_compressor(),true);
  BOOST_CHECK_EQUAL(sink_pipe.name(),"add_one->sum_up");

  sqy::dynamic_pipeline<int> empty_pipe;
  empty_pipe = sqy::dynamic_pipeline<int>::from_string("add_one->square");
  BOOST_CHECK_EQUAL(empty_pipe.empty(),true);
}

BOOST_AUTO_TEST_CASE (encode_with_filters) {

  std::vector<int> input(10);
  std::iota(input.begin(), input.end(),0);
  BOOST_CHECK_EQUAL(input.front(),0);
  BOOST_CHECK_EQUAL(input.back(),9);

  auto filters_pipe = sqy::dynamic_pipeline<int>::from_string("square",filter_factory<int>(), sink_factory<int>());

  std::size_t max_encoded_size_byte = filters_pipe.max_encoded_size(input.size()*sizeof(int));
  std::vector<int> intermediate(max_encoded_size_byte/sizeof(int),0);

  int* encoded_end = filters_pipe.encode(&input[0],&intermediate[0],input.size());

  BOOST_CHECK_EQUAL(intermediate.back(),std::pow(9,2));
  BOOST_CHECK_EQUAL(*(encoded_end - input.size()),std::pow(0,2));
  BOOST_CHECK_EQUAL(*(encoded_end - input.size()+4),std::pow(4,2));
  BOOST_CHECK(encoded_end!=nullptr);
  BOOST_CHECK_EQUAL(encoded_end-&intermediate[0],intermediate.size());
}

BOOST_AUTO_TEST_CASE (decode_with_filters) {

  std::vector<int> input(10,16);
  std::vector<int> output(input.size(),0);

  sqy::dynamic_pipeline<int> filters_pipe = sqy::dynamic_pipeline<int>::from_string("square",filter_factory<int>(), sink_factory<int>());

  std::size_t max_encoded_size_byte = filters_pipe.max_encoded_size(input.size()*sizeof(int));
  std::vector<int> intermediate(max_encoded_size_byte/sizeof(int));

  int* encoded_end = filters_pipe.encode(&input[0],&intermediate[0],input.size());
  BOOST_CHECK(encoded_end!=nullptr);
  
  std::size_t encoded_size = encoded_end - &intermediate[0];
  
  int err_code = filters_pipe.decode(&intermediate[0],&output[0],encoded_size);
  BOOST_CHECK_EQUAL(err_code,0);

  for(std::size_t i = 0;i<input.size();++i)
    BOOST_CHECK_EQUAL(output[i],input[i]);
}

BOOST_AUTO_TEST_CASE (encode_with_sink) {

  std::vector<int> input(10);
  std::iota(input.begin(), input.end(),0);
  BOOST_CHECK_EQUAL(input.front(),0);
  BOOST_CHECK_EQUAL(input.back(),9);

  auto sink_pipe = sqy::dynamic_pipeline<int,std::int8_t>::from_string("square->sum_up",filter_factory<int>(), sink_factory<int>());

  int max_encoded_size = sink_pipe.max_encoded_size(input.size()*sizeof(int));
  BOOST_CHECK_NE(max_encoded_size,0);
  BOOST_CHECK_GT(max_encoded_size,8);

  //FIXME
  std::vector<std::int8_t> intermediate(max_encoded_size,0);
  // std::vector<std::int8_t> intermediate(input.size()*sizeof(int),0);

  std::int8_t* encoded_end = sink_pipe.encode(&input[0],&intermediate[0],input.size());

  BOOST_CHECK_NE(intermediate.front(),0);
  BOOST_CHECK(encoded_end!=nullptr);
}

BOOST_AUTO_TEST_CASE (encode_with_sink_header) {

  std::vector<int> input(10);
  std::iota(input.begin(), input.end(),0);
  BOOST_CHECK_EQUAL(input.front(),0);
  BOOST_CHECK_EQUAL(input.back(),9);

  auto sink_pipe = sqy::dynamic_pipeline<int,std::int8_t>::from_string("square->sum_up",filter_factory<int>(), sink_factory<int>());

  int max_encoded_size = sink_pipe.max_encoded_size(input.size()*sizeof(int));
  BOOST_CHECK_NE(max_encoded_size,0);
  BOOST_CHECK_GT(max_encoded_size,8);

  std::vector<std::int8_t> intermediate(max_encoded_size,0);

  std::int8_t* encoded_end = sink_pipe.encode(&input[0],&intermediate[0],input.size());
  BOOST_CHECK(encoded_end!=nullptr);
  std::vector<std::size_t> decoded_shape = sink_pipe.decoded_shape(&intermediate[0],
								   &intermediate[0]+intermediate.size());

  BOOST_CHECK_NE(decoded_shape.empty(),true);
  BOOST_CHECK_EQUAL(decoded_shape.size(),1);
  BOOST_CHECK_EQUAL(decoded_shape[0],input.size());
  
  
}


BOOST_AUTO_TEST_CASE (decode_with_sink) {

  std::vector<int> input(8,4);
  std::vector<int> output(input.size(),0);
  
  auto sink_pipe = sqy::dynamic_pipeline<int,std::int8_t>::from_string("square->sum_up",filter_factory<int>(), sink_factory<int>());
  int max_encoded_size = sink_pipe.max_encoded_size(input.size()*sizeof(int));

  std::vector<std::int8_t> intermediate(max_encoded_size,0);
  std::int8_t* encoded_end = sink_pipe.encode(&input[0],&intermediate[0],input.size());
  BOOST_REQUIRE(encoded_end!=nullptr);

  std::size_t intermediate_size = encoded_end - &intermediate[0];
  BOOST_CHECK_LE(intermediate_size,max_encoded_size);
  
  int err_code = sink_pipe.decode(&intermediate[0],&output[0],intermediate_size);
  
  BOOST_CHECK_EQUAL(err_code,0);
  BOOST_CHECK_EQUAL(output.back(),input.back());
}


BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( parameter_test_suite )
BOOST_AUTO_TEST_CASE (set_all_to_42) {

  std::vector<int> input(8,4);
  std::vector<int> output(input.size(),1);
  
  auto filters_pipe = sqy::dynamic_pipeline<int>::from_string("set_to(value=42)",filter_factory<int>(), sink_factory<int>());

  BOOST_CHECK_EQUAL(filters_pipe.empty(),false);
  
  std::size_t max_encoded_size_byte = filters_pipe.max_encoded_size(input.size()*sizeof(int));
  std::vector<int> intermediate(max_encoded_size_byte/sizeof(int));

  int* encoded_end = filters_pipe.encode(&input[0],&intermediate[0],input.size());
  BOOST_CHECK(encoded_end!=nullptr);
  BOOST_CHECK_EQUAL(*(encoded_end-1),42);
  
  // std::size_t intermediate_size = encoded_end - &intermediate[0];
  // int err_code = filters_pipe.decode(&intermediate[0],&output[0],intermediate_size);
  // BOOST_CHECK_EQUAL(err_code,0);
  // BOOST_CHECK_EQUAL(output.back(),0);
  // BOOST_CHECK_EQUAL(output.front(),0);
  
}

BOOST_AUTO_TEST_CASE (set_all_to_42_and_sum) {

  std::vector<int> input(8,4);
  std::vector<int> output(input.size(),0);
  
  auto sink_pipe = sqy::dynamic_pipeline<int,std::int8_t>::from_string("set_to(value=42)->sum_up",filter_factory<int>(), sink_factory<int>());

  BOOST_CHECK_EQUAL(sink_pipe.empty(),false);
  
  std::size_t max_encoded_size_byte = sink_pipe.max_encoded_size(input.size()*sizeof(int));
  std::vector<std::int8_t> intermediate(max_encoded_size_byte);

  std::int8_t* encoded_end = sink_pipe.encode(&input[0],&intermediate[0],input.size());
  BOOST_CHECK(encoded_end!=nullptr);

  std::string buffer(&intermediate[0],encoded_end);
  size_t pos = buffer.rfind("|")+1;

  BOOST_CHECK_NE(pos,std::string::npos);
  
  std::uint64_t result = *reinterpret_cast<std::uint64_t*>(&buffer[pos]);
  BOOST_CHECK_NE(result,0);
  BOOST_CHECK_EQUAL(result,42*input.size());
  
}

BOOST_AUTO_TEST_CASE (set_all_to_42_and_decode_to_first_value) {

  std::vector<int> input(8,4);
  std::vector<int> output(8,0);
  
  auto filters_pipe = sqy::dynamic_pipeline<int>::from_string("set_to(value=42)",filter_factory<int>(), sink_factory<int>());
  BOOST_CHECK_EQUAL(filters_pipe.empty(),false);
  
  std::size_t max_encoded_size_byte = filters_pipe.max_encoded_size(input.size()*sizeof(int));
  std::vector<int> intermediate(max_encoded_size_byte/sizeof(int));

  int* encoded_end = filters_pipe.encode(&input[0],&intermediate[0],input.size());
  BOOST_CHECK(encoded_end!=nullptr);

  std::size_t intermediate_size = encoded_end - &intermediate[0];
  int err_code = filters_pipe.decode(&intermediate[0],&output[0],intermediate_size);
  BOOST_CHECK_EQUAL(err_code,0);
  BOOST_CHECK_EQUAL(output.back(),4);
  BOOST_CHECK_EQUAL(output.front(),4);

}


BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( bootstrap_pipeline )
BOOST_AUTO_TEST_CASE (filters_only_same_name) {

  std::vector<int> input(8,4);
  std::vector<int> output(8,0);
  
  auto filters_pipe = sqy::dynamic_pipeline<int>::from_string("set_to(value=42)",filter_factory<int>(), sink_factory<int>());
  BOOST_CHECK_EQUAL(filters_pipe.empty(),false);
  
  std::size_t max_encoded_size_byte = filters_pipe.max_encoded_size(input.size()*sizeof(int));
  std::vector<int> intermediate(max_encoded_size_byte/sizeof(int));

  int* encoded_end = filters_pipe.encode(&input[0],&intermediate[0],input.size());
  BOOST_CHECK(encoded_end!=nullptr);

  //extract header
  std::string buffer((char*)&intermediate[0],(char*)&intermediate[intermediate.size()-1]);
  auto bootstrapped = sqy::dynamic_pipeline<int>::bootstrap(buffer,filter_factory<int>(), sink_factory<int>());

  BOOST_CHECK_EQUAL(bootstrapped.empty(),false);
  BOOST_CHECK_EQUAL(filters_pipe.name(),bootstrapped.name());
}

BOOST_AUTO_TEST_CASE (decode_from_bootstrap) {

  std::vector<int> input(8,4);
  std::vector<int> output(8,0);
  
  auto filters_pipe = sqy::dynamic_pipeline<int>::from_string("set_to(value=42)",filter_factory<int>(), sink_factory<int>());
  BOOST_CHECK_EQUAL(filters_pipe.empty(),false);
  
  std::size_t max_encoded_size_byte = filters_pipe.max_encoded_size(input.size()*sizeof(int));
  std::vector<int> intermediate(max_encoded_size_byte/sizeof(int));

  int* encoded_end = filters_pipe.encode(&input[0],&intermediate[0],input.size());
  BOOST_CHECK(encoded_end!=nullptr);

  //extract header
  std::string buffer((char*)&intermediate[0],(char*)&intermediate[intermediate.size()-1]);
  auto bootstrapped = sqy::dynamic_pipeline<int>::bootstrap(buffer,filter_factory<int>(), sink_factory<int>());

  std::size_t intermediate_size = encoded_end - &intermediate[0];
  int err_code = bootstrapped.decode(&intermediate[0],&output[0],intermediate_size);
  BOOST_CHECK_EQUAL(err_code,0);
  BOOST_CHECK_EQUAL(output.back(),4);
  BOOST_CHECK_EQUAL(output.front(),4);
}

BOOST_AUTO_TEST_CASE (sink_only_same_name) {

  std::vector<int> input(8,4);
  std::vector<int> output(8,0);
  
  auto sink_pipe = sqy::dynamic_pipeline<int,std::int8_t>::from_string("set_to(value=42)->sum_up",
								       filter_factory<int>(),
								       sink_factory<int>());
  BOOST_CHECK_EQUAL(sink_pipe.empty(),false);
  
  std::size_t max_encoded_size_byte = sink_pipe.max_encoded_size(input.size()*sizeof(int));
  std::vector<std::int8_t> intermediate(max_encoded_size_byte);

  std::int8_t* encoded_end = sink_pipe.encode(&input[0],&intermediate[0],input.size());
  BOOST_CHECK(encoded_end!=nullptr);
  BOOST_CHECK_EQUAL(*(std::uint64_t*)(encoded_end-8),8*42);

  //extract header
  std::string buffer((char*)&intermediate[0],(char*)&intermediate[intermediate.size()-1]);
  auto bootstrapped = sqy::dynamic_pipeline<int,std::int8_t>::bootstrap(buffer,filter_factory<int>(), sink_factory<int>());

  BOOST_CHECK_EQUAL(bootstrapped.empty(),false);
  BOOST_CHECK_EQUAL(sink_pipe.name(),bootstrapped.name());
}

BOOST_AUTO_TEST_CASE (sink_decode_from_bootstrap) {

  std::vector<int> input(8,4);
  std::vector<int> output(8,0);
  
  auto sink_pipe = sqy::dynamic_pipeline<int,std::int8_t>::from_string("set_to(value=42)->sum_up",
								       filter_factory<int>(),
								       sink_factory<int>());
  BOOST_CHECK_EQUAL(sink_pipe.empty(),false);
  
  std::size_t max_encoded_size_byte = sink_pipe.max_encoded_size(input.size()*sizeof(int));
  std::vector<std::int8_t> intermediate(max_encoded_size_byte);

  std::int8_t* encoded_end = sink_pipe.encode(&input[0],&intermediate[0],input.size());
  BOOST_CHECK(encoded_end!=nullptr);
  BOOST_CHECK_EQUAL(*(std::uint64_t*)(encoded_end-8),8*42);

  //extract header
  std::string buffer((char*)&intermediate[0],(char*)&intermediate[intermediate.size()-1]);
  auto bootstrapped = sqy::dynamic_pipeline<int,std::int8_t>::bootstrap(buffer,filter_factory<int>(), sink_factory<int>());

  std::size_t intermediate_size = encoded_end - &intermediate[0];
  int err_code = bootstrapped.decode(&intermediate[0],&output[0],intermediate_size);
  BOOST_CHECK_EQUAL(err_code,0);
  BOOST_CHECK_EQUAL(output.back(),4);
  BOOST_CHECK_EQUAL(output.front(),4);
}


BOOST_AUTO_TEST_SUITE_END()
