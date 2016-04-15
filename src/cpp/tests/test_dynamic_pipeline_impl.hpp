#ifndef _TEST_DYNAMIC_PIPELINE_IMPL_HPP_
#define _TEST_DYNAMIC_PIPELINE_IMPL_HPP_

//#include "dynamic_pipeline.hpp"
#include "dynamic_stage.hpp"
#include "dynamic_stage_factory.hpp"
#include "string_parsers.hpp"

namespace sqy = sqeazy;

template <typename T>
struct set_to :  public sqy::filter<T> {

  typedef T raw_type;
  typedef T compressed_type;
  
  raw_type value;
  raw_type first_value;
  
  static_assert(std::is_arithmetic<raw_type>::value==true,"[set_to] input type is non-arithmetic");

  template <typename U>
  set_to(const set_to<U>& _rhs):
    sqy::filter<T>::filter(),
    value(_rhs.value),
    first_value(_rhs.first_value)
  { }
  
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

  template <typename U>
  add_one(const add_one<U>& _rhs):
    sqy::filter<T>::filter()
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

  template <typename U>
  square(const square<U>& _rhs):
    sqy::filter<T>::filter()
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

  template <typename U>
  sum_up(const sum_up<U>& _rhs):
    sqy::filter<T>::filter()
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


template <typename T >
struct high_bits :  public sqy::sink<T> {

  typedef T raw_type;
  typedef typename sqy::sink<T>::out_type compressed_type;
  typedef std::uint64_t result_type;

  std::intmax_t max_encoded_size(std::intmax_t _incoming_size_byte) const override final {

    auto scale = sizeof(compressed_type)/float(sizeof(raw_type));
    return scale*_incoming_size_byte;
  }
  
  std::string name() const {

    return std::string("high_bits");

  }

  
  std::string config() const {

    return std::string("");

  }  

  //TODO: could add parameter to select which of high bits
  high_bits(const std::string& _payload="")
  { }

  template <typename U>
  high_bits(const high_bits<U>& _rhs):
    sqy::filter<T>::filter()
  {

  }
  
  ~high_bits(){};
  

  std::string input_type() const {

    return typeid(raw_type).name();
    
  }

  std::string output_type() const {

    return typeid(compressed_type).name();
    
  }

  compressed_type* encode( const raw_type* _in, compressed_type* _out, std::vector<std::size_t> _shape) override final {

    std::size_t size = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());
    const int shift_right_by = (sizeof(raw_type)*CHAR_BIT) - 4;
  
    for(std::size_t i = 0;i < size;++i){
      _out[i] = (_in[i] >> shift_right_by) & 0xf;
    }
    return _out + size;
  }

  int decode( const compressed_type* _in, raw_type* _out, std::vector<std::size_t> _shape) const override final {

    std::size_t size = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());

    const int shift_left_by = (sizeof(raw_type)*CHAR_BIT) - 4;

    for(std::size_t i = 0;i < size;++i){
      _out[i] = _in[i] << shift_left_by;
    }
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
using sink_factory = sqy::stage_factory<sum_up<T>, high_bits<T> >;

template <typename T>
using my_tail_factory = sqy::stage_factory<add_one<T> >;

static auto adder_sptr = std::make_shared<add_one<int>>(add_one<int>());
static auto square_sptr = std::make_shared<square<int>>(square<int>());
static auto summer_sptr = std::make_shared<sum_up<int>>(sum_up<int>());
static auto hibits_sptr = std::make_shared<high_bits<int>>(high_bits<int>());


#endif /* _TEST_DYNAMIC_PIPELINE_IMPL_H_ */
