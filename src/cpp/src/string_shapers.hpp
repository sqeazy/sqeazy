#ifndef _STRING_SHAPERS_H_
#define _STRING_SHAPERS_H_

#include <vector>
#include <string>


namespace sqeazy {

  template <typename char_itr_t>
  std::vector<std::string> break_lines(char_itr_t _begin,
				       char_itr_t _end,
				       int _target_line_size,
				       const std::string& _sep = " "
				       ){

    
    const size_t string_length = _end - _begin;
    
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
  
};

#endif /* _STRING_PARSERS_H_ */
