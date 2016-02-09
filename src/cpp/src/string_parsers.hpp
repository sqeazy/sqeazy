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

      
    std::vector<std::string> v;

    bool r = qi::parse( _begin,                          /*< start iterator >*/
			_end,                           /*< end iterator >*/
			(
			 +qi::alnum % +qi::char_(_sep)
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

  typedef std::vector<std::pair<std::string, std::string> > pairs_type;
  
  template <typename Iterator>
    struct ordered_command_sequence 
      : qi::grammar<Iterator, pairs_type()>
    {
        ordered_command_sequence()
          : ordered_command_sequence::base_type(query)
        {
	  query =  pair >> *((qi::lit(';') | qi::lit("->")) >> pair);
	  pair  =  key >> -(qi::lit('(') >> -value >> qi::lit(')'));
	  key   =  qi::char_("a-zA-Z_") >> *qi::char_("a-zA-Z_0-9");
	  value =  qi::alnum >> *qi::char_("a-zA-Z_0-9.=,;");
        }

        qi::rule<Iterator, pairs_type()> query;
        qi::rule<Iterator, std::pair<std::string, std::string>()> pair;
        qi::rule<Iterator, std::string()> key, value;
    };


  template <typename char_itr_t>
  pairs_type parse_by(char_itr_t _begin, char_itr_t _end, const std::string& _sep = ","){
    ordered_command_sequence<std::string::iterator> p;
    pairs_type v;

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
  std::map<std::string,std::string> unordered_parse_by(char_itr_t _begin, char_itr_t _end, const std::string& _sep = ","){

    using return_t = std::map<std::string,std::string>;

    return_t value;

    pairs_type v = parse_by(_begin,_end,_sep);

    for( auto & pair : v )
      value[pair.first] = pair.second;
    
    return value;
    
  }
  
};

#endif /* _STRING_PARSERS_H_ */
