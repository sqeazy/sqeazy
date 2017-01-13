#ifndef _STRING_SHAPERS_H_
#define _STRING_SHAPERS_H_

#include <vector>
#include <string>

#include "sqeazy_common.hpp"

namespace sqeazy {

  template <typename char_itr_t>
  std::vector<std::string> break_lines(char_itr_t _begin,
                                       char_itr_t _end,
                                       std::size_t _target_line_size,
                                       const std::string& _sep = " "
                                       ){


    const std::size_t string_length = _end - _begin;

    std::vector<std::string> value;

    if(string_length < _target_line_size)
      value.push_back(std::string(_begin,_end));
    else{
      std::string local(_begin,_end);

      const size_t n_lines = (string_length + _target_line_size -1 )/_target_line_size;
      value.reserve(n_lines);

      auto cbegin = local.cbegin();
      auto cend = local.cend();

      for(size_t i = 0;i<n_lines && cbegin!=cend;++i){

        auto target_end = cbegin + _target_line_size;
        size_t pos = target_end - local.cbegin();
        size_t dist_to_next  = local.find(_sep, pos);
        if(dist_to_next!=std::string::npos)
          target_end += (dist_to_next - pos);

        if(target_end<cend){
          value.push_back(std::string(cbegin,target_end));
          cbegin = target_end + 1;
        }
        else{
          value.push_back(std::string(cbegin,cend));
          cbegin = cend;
        }
      }
      ;

    }
    return value;

  }

  // template <typename string_t>
  // std::string remove_whitespace(const string_t& _buffer){

  //   //stripped.erase(remove_if(stripped.begin(), stripped.end(), isspace),stripped.end());

  //   auto fpos = _buffer.find(sqeazy::ignore_this_delimiters.first);

  //   std::string value = _buffer;

  //   if(fpos == std::string::npos){
  //     auto vitr = remove_copy_if(_buffer.begin(), _buffer.end(),
  //                                value.begin(),
  //                                isspace);
  //     value.erase(vitr,value.end());
  //   }
  //   else{

  //     auto buffer_begin = _buffer.begin();
  //     auto vitr = value.begin();
  //     std::size_t fend = 0;

  //     while(fpos != std::string::npos){
  //       vitr = std::remove_copy_if(buffer_begin,
  //                                  _buffer.begin()+fpos,
  //                                  vitr,
  //                                  isspace);
  //       buffer_begin = _buffer.begin()+fpos;

  //       fend = _buffer.find(sqeazy::ignore_this_delimiters.second, fpos);
  //       if(fend != std::string::npos && (_buffer.size() - fend) >= sqeazy::ignore_this_delimiters.second.size())
  //         fend += sqeazy::ignore_this_delimiters.second.size();

  //       vitr = std::copy(buffer_begin,
  //                        (fend != std::string::npos) ? _buffer.begin()+fend : _buffer.end(),
  //                        vitr);

  //       buffer_begin = (fend != std::string::npos) ? _buffer.begin()+fend : _buffer.end();
  //       fpos = _buffer.find(sqeazy::ignore_this_delimiters.first, fend);
  //     }


  //     vitr = std::remove_copy_if(buffer_begin,
  //                                _buffer.end(),
  //                                vitr,
  //                                isspace);

  //     value.erase(vitr,value.end());
  //   }
  //   return value;

  // }

};

#endif /* _STRING_PARSERS_H_ */
