#ifndef _SQEAZY_HEADER_H_
#define _SQEAZY_HEADER_H_

#include <cctype>
#include <numeric>
#include <algorithm>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <stdexcept>
#include <typeinfo>
#include <cstdint>
#include <iterator>

#include <boost/property_tree/ptree.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/utility/string_ref.hpp>

#include "sqeazy_common.hpp"
#include "string_shapers.hpp"

namespace bpt = boost::property_tree;

namespace sqeazy {

  template <typename iter>
  static inline bool ends_with(iter begin, iter end, const std::string& _match){

    const std::size_t len = std::distance(begin,end);
    boost::string_ref buffer(&*begin,len);
    bool value = false;
    if(!(_match.size()>len || !len || _match.empty())){
      //      value = std::equal(_match.begin(),_match.end(),begin+len-_match.size());
      value = buffer.ends_with(_match);
    }

    return value;
  }

  static unsigned sizeof_typeinfo(const std::string& _lit) {

    static std::map<std::string,int> type_size_map;
    type_size_map[typeid(short).name()] = sizeof(short);
    type_size_map[typeid(unsigned short).name()] = sizeof(unsigned short);
    type_size_map[typeid(char).name()] = sizeof(char);
    type_size_map[typeid(unsigned char).name()] = sizeof(unsigned char);
    type_size_map[typeid(int).name()] = sizeof(int);
    type_size_map[typeid(unsigned int).name()] = sizeof(unsigned int);
    type_size_map[typeid(long).name()] = sizeof(long);
    type_size_map[typeid(unsigned long).name()] = sizeof(unsigned long);

    std::map<std::string,int>::const_iterator found = type_size_map.find(_lit);
    if(found!=type_size_map.end())
      return found->second;
    else
      return 0;
  }


  struct image_header {

    static std::string header_end_delim;
    static const std::string header_end_delimeter() { return header_end_delim; }


    // typedef T value_type;

    std::string header_;

    std::vector<std::size_t> raw_shape_;
    std::string pipeline_;
    std::string raw_type_name_;
    std::intmax_t compressed_size_byte_;

    /**
       \brief swap

       \param[in] _lhs reference to left-hand side
       \param[in] _rhs reference to right-hand side

       \return 
       \retval 

    */
    friend void swap(image_header& _lhs, image_header& _rhs) // nothrow
    {
      std::swap(_lhs.header_, _rhs.header_);
      std::swap(_lhs.raw_shape_, _rhs.raw_shape_);
      std::swap(_lhs.pipeline_, _rhs.pipeline_);
      std::swap(_lhs.raw_type_name_, _rhs.raw_type_name_);
      std::swap(_lhs.compressed_size_byte_, _rhs.compressed_size_byte_); 
    }

    /**
       \brief default constructor

       \param[in] 

       \return 
       \retval 

    */
    image_header():
      header_(""),
      raw_shape_(0),
      pipeline_(""),
      raw_type_name_(typeid(void).name()),
      compressed_size_byte_(0)
    {
    }

    /**
       \brief copy constructor

       \param[in] 

       \return 
       \retval 

    */
    image_header(const image_header& _rhs):
      header_			(_rhs.header_                  ),
      raw_shape_		(_rhs.raw_shape_              ),
      pipeline_		(_rhs.pipeline_               ),
      raw_type_name_		(_rhs.raw_type_name_          ),
      compressed_size_byte_	(_rhs.compressed_size_byte_   )
    {
    }

    /**
       \brief copy-assignment

       \param[in] 

       \return 
       \retval 

    */
    image_header& operator=(image_header _rhs){

      swap(*this, _rhs);

      return *this;
    }
	
    /**
       \brief pack takes the parameters of a to-compress/compressed nD data set and packs them into a JSON compatible string, the output string needs to be aligned to raw_type

       \param[in]  raw_type data type of the nD data set to compress       
       \param[in] _dims shape of the nD data set to compress
       \param[in] _pipename sqy pipeline used
       \param[in] _payload_bytes size of the sqy compressed buffer in Byte


       \return std::string that contains the JSON packed header (stripped of any whitespaces)
       \retval 

    */
    template <typename raw_type,typename size_type>
    static const std::string pack(const std::vector<size_type>& _dims,
                                  const std::string& _pipe_name = "no_pipeline",
                                  const unsigned long& _payload_bytes = 0
                                  ) {

      // Create an empty property tree object.
      bpt::ptree tree;
      std::stringstream json_stream("");

      // Put the simple values into the tree. The integer is automatically
      // converted to a string. Note that the "debug" node is automatically
      // created if it doesn't exist.
      tree.put("pipename", _pipe_name);

      std::string raw_type_name = typeid(raw_type).name();
      auto space_pos = raw_type_name.find(" ");
	  if (space_pos != std::string::npos)
        raw_type_name.replace(space_pos, 1, "_");

      tree.put("raw.type", raw_type_name);
      tree.put("raw.rank",_dims.size());

      for(unsigned i = 0;i<_dims.size();++i){
        tree.add("raw.shape.dim", _dims[i]);
      }

      tree.put("encoded.bytes", _payload_bytes);

      // Add all the modules. Unlike put, which overwrites existing nodes, add
      // adds a new node at the lowest level, so the "modules" node will have
      // multiple "module" children.

      // Write property tree to XML file
      bpt::write_json(json_stream, tree);
      std::string stripped = json_stream.str();
      //stripped = remove_whitespace(stripped);

      stripped += image_header::header_end_delim;

      if(stripped.size() % sizeof(raw_type) != 0){
        std::stringstream intermediate("");
        intermediate << std::setw(sizeof(raw_type) - (stripped.size() % sizeof(raw_type))) << " ";
        intermediate << stripped;
        stripped = intermediate.str();
      }

      return std::string(stripped);
    }

    template <typename value_type,
              typename size_type
              >
    image_header(value_type,
                 const std::vector<size_type>& _dims,
                 const std::string& _pipe_name = "no_pipeline",
                 const unsigned long& _payload_bytes = 0):
      header_(""),
      raw_shape_(_dims.begin(), _dims.end()),
      pipeline_(_pipe_name),
      raw_type_name_(typeid(value_type).name()),
      compressed_size_byte_(_payload_bytes)
    {


      if(!_payload_bytes){
        compressed_size_byte_ = std::accumulate(raw_shape_.begin(), raw_shape_.end(),sizeof(value_type),std::multiplies<unsigned long>());
      }

      try{
        header_ = pack<value_type>(raw_shape_,
                                   pipeline_,
                                   compressed_size_byte_);
      }
      catch(...){
        std::cerr << "["<< __FILE__ <<":" << __LINE__ <<"]\t unable to pack pipe!\n";
        header_ = "";
      }
    }

    template <typename value_type>
    image_header(value_type,
                 unsigned long _raw_in_byte,
                 const std::string& _pipe_name = "no_pipeline",
                 const unsigned long& _payload_bytes = 0):
      header_(""),
      raw_shape_(1, _raw_in_byte),
      pipeline_(_pipe_name),
      raw_type_name_(typeid(value_type).name()),
      compressed_size_byte_(_payload_bytes)
    {

      if(!_payload_bytes){
        compressed_size_byte_ = std::accumulate(raw_shape_.begin(), raw_shape_.end(),sizeof(value_type),std::multiplies<unsigned long>());
      }

      try{
        header_ = pack<value_type>(raw_shape_,
                                   pipeline_,
                                   compressed_size_byte_);
      }
      catch(...){
        std::cerr << "["<< __FILE__ <<":" << __LINE__ <<"]\t unable to pack pipe!\n";
      }
    }

    template <typename value_type>
    void set_compressed_size_byte(std::intmax_t _new)
    {
      compressed_size_byte_ = _new;

      try{
        header_ = pack<value_type>(raw_shape_,
                                   pipeline_,
                                   compressed_size_byte_);
      }
      catch(...){
        std::cerr << "["<< __FILE__ <<":" << __LINE__ <<"]\t unable to pack pipe!\n";
      }
    }

    template <typename value_type>
    void set_pipeline(const std::string& _pname)
    {
      pipeline_ = _pname;
      
      try{
        header_ = pack<value_type>(raw_shape_,
                                   pipeline_,
                                   compressed_size_byte_);
      }
      catch(...){
        std::cerr << "["<< __FILE__ <<":" << __LINE__ <<"]\t unable to pack pipe!\n";
      }
    }

    static const image_header unpack(const std::string& _buffer) {

      return unpack(_buffer.begin(), _buffer.end());
    }


    static const image_header unpack(const char* _buffer,
                                     unsigned long _buffer_size) {

      return unpack(_buffer, _buffer + _buffer_size);
    }


    template <typename iter_type>
    static const image_header unpack(iter_type _begin, iter_type _end) {

      iter_type header_end_ptr = header_end(_begin, _end);

      if(!valid_header(_begin,
                       header_end_ptr + (header_end_ptr!=_end ? header_end_delimeter().size() : 0)
                       )
         ){
        std::ostringstream msg;
        msg << "[image_header::unpack]\t received header: \n\t";
        std::copy(_begin,header_end_ptr,std::ostreambuf_iterator<char>(msg));
        msg <<"\n does not comply expected format\n";
        throw std::runtime_error(msg.str().c_str());
      }



      //let's omit the header_end_delim for the JSON to be parsable
      std::string hdr(_begin, header_end_ptr);


      image_header value;
      std::stringstream incoming;
      incoming << hdr;

      bpt::ptree tree;
      try{
        bpt::read_json(incoming, tree);
      }
      catch (std::exception &e){
        std::stringstream msg;
        std::cerr << "[image_header::unpack]\t received header: \n\t>>"<< hdr << "<<\n"
                  << "cannot create property tree from JSON\nreason: " <<  e.what() << "\n";
        return image_header(value);
      }

      value.pipeline_ = tree.get("pipename", "");
      value.raw_type_name_ = tree.get("raw.type", typeid(void).name());

	  if (value.raw_type_name_.find("_") != std::string::npos)
        value.raw_type_name_.replace(value.raw_type_name_.find("_"), 1, " ");

      unsigned rank = tree.get("raw.rank", (unsigned)0);//TODO: could be replaced by (tend - tbegin)
      value.raw_shape_.reserve(rank);

      bpt::ptree::const_iterator tbegin = tree.get_child("raw.shape").begin();
      bpt::ptree::const_iterator tend = tree.get_child("raw.shape").end();

      for(;tbegin!=tend;++tbegin)
        value.raw_shape_.push_back(boost::lexical_cast<unsigned long>(tbegin->second.data()));

      value.compressed_size_byte_ = tree.get("encoded.bytes", (unsigned long)0);
      value.header_ = hdr;
      if(!ends_with(value.header_.begin(),value.header_.end(),header_end_delim))
        value.header_ += header_end_delim;

      return image_header(value);//unnamed return-type optimisation
    }


    /**
       \brief constructor that creates the header as much as possible from a string (the header will remain at size 0 if any error occurs)

       \param[in] _str that contains the header

       \return 
       \retval 

    */
    image_header(const std::string& _str):
      header_(""),
      raw_shape_(0),
      pipeline_(""),
      raw_type_name_(typeid(void).name()),
      compressed_size_byte_(0){


      image_header rhs;
	  
      try{
        rhs = unpack(_str.begin(), _str.end());
      }
      catch(...){
        std::cerr << "["<< __FILE__ <<":" << __LINE__ <<"]\t unable to unpack header (" << _str <<")!\n";
        header_ = "";
      }

      swap(*this,rhs);

    }

    template <typename Iter>
    image_header(Iter _begin, Iter _end):
      header_(""),
      raw_shape_(0),
      pipeline_(""),
      raw_type_name_(typeid(void).name()),
      compressed_size_byte_(0){


      image_header rhs;
	  	  

      try{
        rhs = unpack(_begin,_end);
      }
      catch(...){
        std::cerr << "["<< __FILE__ <<":" << __LINE__ <<"]\t unable to unpack header (" << header_ <<")!\n";
      }

      swap(*this,rhs);

    }

    ~image_header(){}



    std::size_t size() const {

      return header_.size();
    }

    std::vector<std::size_t> const * shape() const {
      return &raw_shape_;
    }

    template <typename U>
    void shape(std::vector<U>& _dims) const {
      _dims = raw_shape_;
    }

    std::string str() const {

      return header_;

    }



    bool empty() const {
      return header_.empty();
    }

    std::string::iterator begin(){
      return header_.begin();
    }

    std::string::const_iterator begin() const {
      return header_.begin();
    }

    std::string::iterator end(){
      return header_.end();
    }

    std::string::const_iterator end() const {
      return header_.end();
    }

    std::string pipeline() const {

      return this->pipeline_;

    }

	std::intmax_t compressed_size_byte() const {

      return this->compressed_size_byte_;

    }


    std::string raw_type() const {
      return raw_type_name_;
    }

    unsigned long raw_size() const {

      unsigned long value = std::accumulate(raw_shape_.begin(), raw_shape_.end(), 1, std::multiplies<unsigned long>());
      return value;
    }

    unsigned long raw_size_byte() const {

      unsigned long size_of_type = sizeof_typeinfo(raw_type_name_);
      if(!header_.empty()) {
        unsigned found_size_byte = sizeof_header_type();
        if(found_size_byte && found_size_byte!=size_of_type)
          return found_size_byte*raw_size();
      }
      return raw_size()*size_of_type;
    }

    template <typename T>
    unsigned long raw_size_byte_as() const {

      unsigned long size_of_type = sizeof(T);
      if(!header_.empty()) {
        unsigned found_size_byte = sizeof_header_type();
        if(found_size_byte && found_size_byte!=size_of_type)
          return found_size_byte*raw_size();
      }
      return raw_size()*size_of_type;
    }

    unsigned sizeof_header_type() const {
      unsigned value = 0;
      if(header_.empty())
        return value;

      value = sizeof_typeinfo(raw_type_name_);
      return value;
    }





    template <typename Iter>
    static const bool valid_header(Iter _begin, Iter _end){

      bool value = std::count(_begin,_end, '}');
      value = value && std::count(_begin,_end, '}') ==  std::count(_begin,_end, '{');
      value = value && std::count(_begin,_end, ':')>1;

      value = value && ends_with(_begin,_end,header_end_delimeter());
      return value;


    }


    static const bool valid_header(const std::string& _hdr){
      return valid_header(_hdr.begin(), _hdr.end());
    }


    template <typename iter_type>
    static const iter_type header_end(iter_type _begin, iter_type _end){

      const std::size_t len = std::distance(_begin,_end);
      boost::string_ref buffer(&*_begin,len);
      iter_type value = _end;
      auto fpos = buffer.find(header_end_delimeter());

      if(fpos!=boost::string_ref::npos)
        value = _begin+fpos;

      return value;

    }


    template <typename iter_type>
    static const bool contained(iter_type _begin, iter_type _end){
      iter_type header_end_ptr = header_end(_begin,_end);

      return valid_header(_begin, header_end_ptr);

    }

    static const std::vector<std::size_t> unpack_shape(const char* _buffer, const unsigned& _size) {

      image_header unpacked = unpack(_buffer,_buffer + _size);
      return unpacked.raw_shape_;

    }


    static const int unpack_num_dims(const char* _buffer, const unsigned& _size) {



      image_header unpacked = unpack(_buffer, _buffer+_size);
      return static_cast<int>(unpacked.raw_shape_.size());
    }

    static const std::string unpack_type(const char* _buffer, const unsigned& _size) {

      image_header unpacked = unpack(_buffer, _buffer+_size);
      return unpacked.raw_type_name_;
    }

    friend inline bool operator==(const image_header& _left, const image_header& _right)
    {
      bool value = true;
      value = value && _left.header_ ==  _right.header_;
      value = value && _left.raw_shape_ ==  _right.raw_shape_;
      value = value && _left.pipeline_ ==  _right.pipeline_;
      value = value && _left.raw_type_name_ ==  _right.raw_type_name_;
      value = value && _left.compressed_size_byte_ ==  _right.compressed_size_byte_;
      return value;
    }

    friend inline bool operator!=(const image_header& lhs, const image_header& rhs){return !(lhs == rhs);}
  };


  std::string image_header::header_end_delim = "|01307#!";

};

  
#endif /* _SQEAZY_HEADER_H_ */
