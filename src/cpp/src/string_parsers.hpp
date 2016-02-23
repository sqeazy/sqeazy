#ifndef _STRING_PARSERS_H_
#define _STRING_PARSERS_H_


#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp>

#include <vector>
#include <map>
#include <string>


namespace sqeazy {

  namespace qi = boost::spirit::qi;
  namespace ascii = boost::spirit::ascii;
  namespace phoenix = boost::phoenix;

  template <typename char_itr_t>
  std::vector<std::string> split_by(char_itr_t _begin, char_itr_t _end, const std::string& _sep = ","
				    ){

    std::string allowed_characters = "a-zA-Z_0-9.=";
    auto f_itr = allowed_characters.find(_sep);

    if(f_itr != std::string::npos)
      allowed_characters.erase(f_itr,f_itr + _sep.size());
    
    std::vector<std::string> v;

    bool r = qi::parse( _begin,                          /*< start iterator >*/
			_end,                           /*< end iterator >*/
			(
			 +qi::char_(allowed_characters) % +qi::lit(_sep)
			 ),
			v
			);

    if (_begin != _end  && !r) // fail if we did not get a full match
      {
	std::string msg(_begin,_end);
	std::cerr << "[sqeazy::split_by] couldn't split anything in " << msg << "\n";
	v.clear();
	return v;
      }
    
    return v;

  }

  
  typedef std::vector<std::pair<std::string, std::string> > vec_of_pairs_t;
  typedef std::map<std::string, std::string> parsed_map_t;
  
  template <typename Iterator>
    struct ordered_command_sequence 
      : qi::grammar<Iterator, vec_of_pairs_t()>
    {
        ordered_command_sequence()
          : ordered_command_sequence::base_type(query)
        {
	  query =  pair >> *((qi::lit(';') | qi::lit("->")) >> pair);
	  pair  =  key >> -(qi::lit('(') >> -value >> qi::lit(')'));
	  key   =  qi::char_("a-zA-Z_") >> *qi::char_("a-zA-Z_0-9");
	  value =  qi::alnum >> *qi::char_("a-zA-Z_0-9.=,;");
        }

        qi::rule<Iterator, vec_of_pairs_t()> query;
        qi::rule<Iterator, std::pair<std::string, std::string>()> pair;
        qi::rule<Iterator, std::string()> key, value;
    };
  
  template <typename char_itr_t>
  vec_of_pairs_t parse_by(char_itr_t _begin, char_itr_t _end, const std::string& _sep = ","){

    // ordered_command_sequence<std::string::iterator> p;
    ordered_command_sequence<char_itr_t> p;
    vec_of_pairs_t v;

    bool r = qi::parse(_begin, _end, p, v);
    if (_begin != _end  && !r) // fail if we did not get a full match
      {
	std::string msg(_begin,_end);
	std::cerr << "[sqeazy::parse_by] couldn't parse anything in " << msg << "\n";
	v.clear();
	return v;
      }

    return v;
  }
  
  template <typename char_itr_t>
  parsed_map_t unordered_parse_by(char_itr_t _begin, char_itr_t _end,
				  const std::string& _pair_sep = ",",
				  const std::string& _kv_sep = "="){

    using return_t = std::map<std::string,std::string>;

    return_t value;

    std::vector<std::string> parts = split_by(_begin,_end,_pair_sep);
    std::vector<std::string> key_value;
    
    for( auto & str : parts ){

      key_value = split_by(str.begin(),
			   str.end(),
			   _kv_sep);

      if(key_value.size()==1)
	value[key_value.front()] = "";
      
      if(key_value.size()>1 && key_value.size() % 2 == 0)
	value[key_value.front()] = key_value.back();

      key_value.clear();

    }
    
    return value;
    
  }
  
};

#endif /* _STRING_PARSERS_H_ */
