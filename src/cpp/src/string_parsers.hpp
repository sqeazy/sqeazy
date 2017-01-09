#ifndef _STRING_PARSERS_H_
#define _STRING_PARSERS_H_


#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp>

#include <boost/utility/string_ref.hpp>

#include <vector>
#include <map>
#include <string>


namespace sqeazy {

  namespace qi = boost::spirit::qi;
  namespace ascii = boost::spirit::ascii;
  namespace phoenix = boost::phoenix;

  typedef std::vector<std::string> vec_of_strings_t;
  typedef std::vector<boost::string_ref> vec_of_string_refs_t;
  
  typedef std::vector<std::pair<std::string, std::string> > vec_of_pairs_t;
  typedef std::map<std::string, std::string> parsed_map_t;

  static const std::pair<std::string, std::string> ignore_this_delimiters = std::make_pair("<verbatim>","</verbatim>");
  static const std::pair<boost::string_ref, boost::string_ref> ignore_this_delimiters_ref = std::make_pair(boost::string_ref(ignore_this_delimiters.first),
													   boost::string_ref(ignore_this_delimiters.second)); 
  
  template <typename char_itr_t>
  vec_of_strings_t split_by(char_itr_t _begin, char_itr_t _end, const std::string& _sep = ","
				    ){

    std::string allowed_characters = "a-zA-Z_0-9.=";
    auto f_itr = allowed_characters.find(_sep);

    if(f_itr != std::string::npos)
      allowed_characters.erase(f_itr,f_itr + _sep.size());
    
    vec_of_strings_t v;

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

  
  
  
  template <typename string_t>
  vec_of_string_refs_t split_string_ref_to_ref(const boost::string_ref& _data,
					       const string_t& _sep = ","
					       ){
    vec_of_string_refs_t value;

    if(_sep.empty() || _data.empty())
      return value;

    std::size_t approx_count = std::count(_data.begin(), _data.end(),_sep.front());
    value.reserve(approx_count);
    
    std::size_t first = 0;

    boost::string_ref to_search(_data);
    std::size_t pos_of_ignore_delim = to_search.find(ignore_this_delimiters.first);
    
    while(to_search.size()){
	
      first = to_search.find_first_of(_sep);

      if(first != boost::string_ref::npos &&
	 pos_of_ignore_delim!= boost::string_ref::npos &&
	 pos_of_ignore_delim < first //did we pass a delimiter of ignorance?
	 ) 
	{
	pos_of_ignore_delim = to_search.find(ignore_this_delimiters.second);
	
	boost::string_ref tmp = to_search;
	tmp.remove_prefix(pos_of_ignore_delim+ignore_this_delimiters.second.size());
	first = tmp.find_first_of(_sep);
	if(first != boost::string_ref::npos)
	  first +=  pos_of_ignore_delim+ignore_this_delimiters.second.size();
	}	

      auto end_itr = first != boost::string_ref::npos ? to_search.begin()+first : to_search.end();
      std::size_t len = std::distance(to_search.begin(),end_itr);
      boost::string_ref result(&*to_search.begin(),
			       len);

      value.push_back(result);
      
      if(first==boost::string_ref::npos){
	break;
      }

      to_search.remove_prefix(first + _sep.size());
      pos_of_ignore_delim -= (first + _sep.size());
    }


    value.reserve(value.size());
    
    return value;

  }

  template <typename string_t>
  vec_of_strings_t split_string_ref_by(const boost::string_ref& _data,
				       const string_t& _sep = ","
				       ){
    

    auto tmp = split_string_ref_to_ref(_data,_sep);
    vec_of_strings_t value;
    value.reserve(tmp.size());
    
    for( const boost::string_ref& ref : tmp )
      value.push_back(std::string(ref.begin(), ref.end()));
    

    return value;

  }
  

  
  vec_of_strings_t split_string_by(const std::string& _data,
				   const std::string& _sep = ","
				   ){
    vec_of_strings_t value;

    if(_sep.empty() || _data.empty())
      return value;

    boost::string_ref data_ref = _data;

    value = split_string_ref_by(data_ref, _sep);
    
    return value;

  }

  

  
  
  template <typename Iterator>
    struct ordered_command_sequence 
      : qi::grammar<Iterator, vec_of_pairs_t()>
    {
      ordered_command_sequence()
          : ordered_command_sequence::base_type(query)
        {
	  query =  pair >> *(qi::lit("->") >> pair);
	  pair  =  key >> -('(' >> value >> ')');
	  key   =  qi::char_("a-zA-Z") >> *qi::char_("a-zA-Z_0-9");
	  //value =  +qi::char_("a-zA-Z_0-9:=,\"/");
	  value =  +(qi::print - qi::char_("()"));
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
	std::cerr << "[sqeazy::parse_by] failed to parse " << msg << "\n";
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

    vec_of_strings_t parts = split_by(_begin,_end,_pair_sep);
    vec_of_strings_t key_value;
    
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

  parsed_map_t parse_string_by(const std::string& _payload,
			       const std::string& _pair_sep = ",",
			       const std::string& _kv_sep = "="){

    using return_t = std::map<std::string,std::string>;

    return_t value;

    vec_of_strings_t parts = split_string_by(_payload,_pair_sep);
    vec_of_strings_t key_value;
    
    for( auto & str : parts ){

      key_value = split_string_by(str,
				  _kv_sep);

      if(key_value.size()==1)
	value[key_value.front()] = "";
      
      if(key_value.size()>1 && key_value.size() % 2 == 0)
	value[key_value.front()] = key_value.back();

      key_value.clear();

    }
    
    return value;
    
  }



  
  struct pipeline_parser {

    typedef std::vector<boost::string_ref> vec_of_string_refs_t;
    
    vec_of_strings_t seperators_;

    
    std::map<std::string, parsed_map_t> tree_; 

    pipeline_parser(const vec_of_strings_t& _seps = {"->",",","="}):
      seperators_(_seps)
    {

     	
    }
    

    template <typename iter_t>
    std::map<std::string, parsed_map_t> operator()(iter_t _begin, iter_t _end){

      std::map<std::string, parsed_map_t> value;
      const std::size_t len = std::distance(_begin,_end);
      
      if(!len)
	return value;
      
      if(seperators_.empty())
	return value;

      
      boost::string_ref msg(&*_begin,len);
      
      vec_of_string_refs_t major_keys = split_string_ref_to_ref(msg,
								seperators_.front());

      for(const boost::string_ref& maj_key : major_keys ){

	auto dist = maj_key.find("(");
	if(dist == std::string::npos)
	  dist = maj_key.size();
	
	std::string key(maj_key.begin(),
			maj_key.begin()+dist);
	
	parsed_map_t pmap;
	
	if( key.size() < maj_key.size() ){

	  boost::string_ref in_brackets(maj_key.begin()+dist+1,
					maj_key.size()-1-(dist+1));

	  vec_of_string_refs_t options = split_string_ref_to_ref(in_brackets,
								 seperators_[1]);
	  for( const boost::string_ref& opt : options ){
	    vec_of_strings_t key_value = split_string_ref_by(opt,
							     seperators_.back());

	    pmap[key_value.front()] = key_value.back();
	    
	  }
	  	  
	  
	}
	
	value[key] = pmap;
      }

      return value;
    }

    template <typename iter_t>
    vec_of_pairs_t to_pairs(iter_t _begin, iter_t _end){

      vec_of_pairs_t value;
      const std::size_t len = std::distance(_begin,_end);
      
      if(!len)
	return value;
      
      if(seperators_.empty())
	return value;

      
      boost::string_ref msg(&*_begin,len);
      
      vec_of_string_refs_t major_keys = split_string_ref_to_ref(msg,
								seperators_.front());

      value.reserve(major_keys.size());
	
      for(const boost::string_ref& maj_key : major_keys ){

	auto dist = maj_key.find("(");
	if(dist == std::string::npos)
	  dist = maj_key.size();
	
	std::string key(maj_key.begin(),
			maj_key.begin()+dist);
	
	std::string in_brackets = "";
	if( key.size() < maj_key.size() ){

	  in_brackets = std::string(maj_key.begin()+dist+1,
				    maj_key.end()-1);
	  
	}
	
	value.push_back( std::make_pair(key,in_brackets) );
      }

      return value;
    }

    template <typename iter_t>
    vec_of_strings_t major_keys(iter_t _begin, iter_t _end){

      vec_of_strings_t value;
      const std::size_t len = std::distance(_begin,_end);
      
      if(!len)
	return value;
      
      if(seperators_.empty())
	return value;

      
      boost::string_ref msg(&*_begin,len);
      
      vec_of_string_refs_t major_keys = split_string_ref_to_ref(msg,
								seperators_.front());

      value.reserve(major_keys.size());
	
      for(const boost::string_ref& maj_key : major_keys ){

	auto dist = maj_key.find("(");
	if(dist == std::string::npos)
	  dist = maj_key.size();
	
	std::string key(maj_key.begin(),
			maj_key.begin()+dist);
		
	value.push_back( key );
      }

      return value;
    }

    
    template <typename iter_t>
    parsed_map_t minors(iter_t _begin, iter_t _end){

      parsed_map_t value;
      const std::size_t len = std::distance(_begin,_end);
      
      if(!len)
	return value;
      
      if(seperators_.empty())
	return value;

      
      boost::string_ref msg(&*_begin,len);
      
      vec_of_string_refs_t major_keys = split_string_ref_to_ref(msg,
								seperators_[1]);

      for(const boost::string_ref& maj_key : major_keys ){

	auto dist = maj_key.find(seperators_.back());
	if(dist == std::string::npos)
	  dist = maj_key.size();
	
	std::string key(maj_key.begin(),
			maj_key.begin()+dist);

	std::string key_value(maj_key.begin()+dist+seperators_.back().size(),
			  maj_key.end());

	value[ key ] = key_value;
      }

      return value;
    }

    

  };

  namespace parsing {

        /**
       \brief function to copy the memory that a container occupies to a string

       ATTENTION: this function is not portable between memory ordering, 
       i.e. something verbatim on little-endian yields not the same as on large-endian systems

       ATTENTION: this function is not portable between different alignments or allocators, 
       i.e. if a container uses some element alignement on the src machine that does not match the one on the destination, this will produce corrupted data

       \return 
       \retval 

    */
    template <typename iter_t>
    static std::string range_to_verbatim(iter_t _begin, iter_t _end){

      typedef typename std::iterator_traits<iter_t>::value_type value_t;
      
      const std::size_t len = std::distance(_begin, _end);
      const std::size_t bytes = len*sizeof(value_t);
      std::size_t exp_size = bytes+ignore_this_delimiters.first.size()+ignore_this_delimiters.second.size();

      std::string value;
      
      if(!len)
	return value;

      value.resize(exp_size);
      
      auto iter = std::copy(ignore_this_delimiters.first.begin(), ignore_this_delimiters.first.end(),
			    value.begin());

      const char* src = reinterpret_cast<const char*>(&*_begin);
      iter = std::copy(src,src+bytes,iter);

      iter = std::copy(ignore_this_delimiters.second.begin(), ignore_this_delimiters.second.end(),
		       iter);

      
      return value;
    }

    
    /**
       \brief function to copy the memory of a verbatim string to range given

       ATTENTION: this function is not portable between memory ordering, 
       i.e. something verbatim on little-endian yields not the same as on large-endian systems

       ATTENTION: this function is not portable between different alignments or allocators, 
       e.g. if a container uses some element alignement on the src machine that does not match the one on the destination, this will produce corrupted data

       \return 
       \retval 

    */
    template <typename string_t, typename iter_t>
    static iter_t verbatim_to_range(string_t _verbatim, iter_t _begin, iter_t _end){

      typedef typename std::iterator_traits<iter_t>::value_type value_t;
      
      const std::size_t len = std::distance(_begin, _end);
      const std::size_t bytes = len*sizeof(value_t);

      iter_t value = _begin;
      
      if(!len)
	return value;
      
      if((_verbatim.size() - ignore_this_delimiters.first.size() - ignore_this_delimiters.second.size()) > bytes)
	return value;
      
      auto src = _verbatim.begin() + ignore_this_delimiters.first.size();
      auto src_end_pos = _verbatim.rfind(ignore_this_delimiters.second) - ignore_this_delimiters.first.size();

      char* dst = reinterpret_cast<char*>(&*_begin);
      auto dst_end = std::copy(src,src+src_end_pos,dst);

      value += std::distance(dst,dst_end)/sizeof(value_t);
      
      return value;
    }
    
    /**
       \brief function estimate how many bytes a range may consume as verbatim string 

       \return 
       \retval 

    */
    template <typename iter_t>
    static std::size_t verbatim_bytes(iter_t _begin, iter_t _end){

      typedef typename std::iterator_traits<iter_t>::value_type value_t;
      
      const std::size_t len = std::distance(_begin, _end);
      const std::size_t bytes = len*sizeof(value_t);
      std::size_t exp_size = bytes+ignore_this_delimiters.first.size()+ignore_this_delimiters.second.size();
      return exp_size;
      
    }

    template <typename value_t,typename string_t>
    static std::size_t verbatim_yields_n_items_of(const string_t& _verbatim ){

      std::size_t value = _verbatim.size()+ignore_this_delimiters.first.size()+ignore_this_delimiters.second.size();
      return value;
      
    }
    
  };
  
};

#endif /* _STRING_PARSERS_H_ */
