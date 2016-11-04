#ifndef _REGEX_HELPERS_H_
#define _REGEX_HELPERS_H_

#include <vector>

#define GCC_VERSION (__GNUC__ * 10000		      \
                               + __GNUC_MINOR__ * 100 \
                               + __GNUC_PATCHLEVEL__)

#if __GNUC__ && GCC_VERSION < 40900

#include "boost/regex.hpp"


namespace sqeazy {

  template <typename string_t>
  int int_extract(const string_t& _data, const std::string& _regex = "W([0-9]+)", int _offset = 0){

    int value = -1;

    const boost::regex regex_present(_regex,boost::regex::egrep);
 
    boost::smatch re_match;
    boost::regex_search(_data, re_match, regex_present);
    if(re_match.size()){
      std::string match = re_match[0];
      value = std::stoi(match.substr(_offset));
    }
    
    return value;
  }

  template <typename string_t>
  std::vector<int> extract_ints(string_t _data, const std::string& _regex = "[0-9]+"){

    std::vector<int> value;
    const boost::regex regex_present(_regex,boost::regex::egrep);
 
    boost::smatch re_match;
    while(boost::regex_search(_data, re_match, regex_present)){
      value.push_back(std::stoi(re_match.str()));
      _data = re_match.suffix();
    }

    
    return value;
  }

  template <typename string_t>
  string_t string_extract(const string_t& _data, const std::string& _regex = "C([0-9]+)", int _offset = 0){

    string_t value = "";

    const boost::regex regex_present(_regex,boost::regex::egrep);
 
    boost::smatch re_match;
    boost::regex_search(_data, re_match, regex_present);
    if(re_match.size()){
      std::string match = re_match[0];
      value = match.substr(_offset);
    }
    
    return value;
  }
  
  template <typename string_it>
  bool matches(string_it _begin,string_it _end, const std::string& _regex){

    const boost::regex present(_regex,boost::regex::egrep);
    
    bool value = boost::regex_match(_begin, _end,present);
    
    return value;
  }
  
};
#else
#include <regex>

namespace sqeazy {
  
  template <typename string_t>
  int int_extract(const string_t& _data, const std::string& _regex = "W([0-9]+)", int _offset = 0){
    
    int value = -1;

    const std::regex regex_present(_regex,std::regex::egrep);
 
    std::smatch re_match;
    std::regex_search(_data, re_match, regex_present);
    if(re_match.size()){
      std::string match = re_match[0];
      value = std::stoi(match.substr(_offset));
    }
    
    return value;
  }

  template <typename string_t>
  std::vector<int> extract_ints(string_t _data, const std::string& _regex = "[0-9]+"){

    std::vector<int> value;

    const std::regex regex_present(_regex,std::regex::egrep);
 
    std::smatch re_match;
    while(std::regex_search(_data, re_match, regex_present)){
      value.push_back(std::stoi(re_match.str()));
      _data = re_match.suffix();
    }

    
    return value;
  }

  
  template <typename string_t>
  string_t string_extract(const string_t& _data, const std::string& _regex = "C([0-9]+)", int _offset = 0){
    
    string_t value = "";

    const std::regex regex_present(_regex,std::regex::egrep);
 
    std::smatch re_match;
    std::regex_search(_data, re_match, regex_present);
    if(re_match.size()){
      std::string match = re_match[0];
      value = match.substr(_offset);
    }
    
    return value;
  }

  

  template <typename string_it>
  bool matches(string_it _begin,string_it _end, const std::string& _regex){

    const std::regex present(_regex,std::regex::egrep);
    
    bool value = std::regex_match(_begin, _end,present);
    
    return value;
  }  
};

#endif 

namespace sqeazy {

  template <typename string_t>
  bool matches(const string_t& _data, const std::string& _regex){
    
    return matches(_data.begin(), _data.end(),_regex);
  }
  
};


#endif /* _REGEX_HELPERS_H_ */
