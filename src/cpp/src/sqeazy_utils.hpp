#ifndef _SQEAZY_UTILS_H_
#define _SQEAZY_UTILS_H_

namespace sqeazy {

  template <typename string_t>
  static inline size_t count(const string_t& _buffer, const string_t& _token){
    size_t value = 0;
    typename string_t::size_type start = 0;

    while ((start = _buffer.find(_token, start)) != string_t::npos) {
      ++value;
      start += _token.size(); // see the note
    }
  }
  
  /**
     \brief split string to a vector of strings
     
     \param[in] _buffer string that contains tokenized content, e.g. "a,b,c"
     \param[in] _token symbol to split _buffer by, e.g. ","
     
     \return vector that contains tokenized content of _buffer
     \retval 
     
  */
  template <typename string_t>
  static inline std::vector<string_t> split(const string_t& _buffer, string_t _token) {

    std::vector<string_t> value;

    size_t begin = 0;
    size_t end = _buffer.find(_token);

    while(begin != string_t::npos) {
      
      value.push_back(_buffer.substr(begin,end - begin));

      if(end != string_t::npos)
	begin = end + _token.size();
      else
	begin = string_t::npos;
      
      end = _buffer.find(_token,begin);
      
    }

    return value;
  }

};
#endif /* _SQEAZY_UTILS_H_ */
