#ifndef _STRING_PARSERS_H_
#define _STRING_PARSERS_H_

#include <vector>
#include <map>
#include <string>

#include <boost/utility/string_ref.hpp>

#include <boost/algorithm/string.hpp>

#include "sqeazy_common.hpp"

#include "base64.hpp"

namespace sqeazy {

  typedef std::vector<std::string> vec_of_strings_t;
  typedef std::vector<boost::string_ref> vec_of_string_refs_t;

  typedef std::vector<std::pair<std::string, std::string> > vec_of_pairs_t;
  typedef std::map<std::string, std::string> parsed_map_t;

  // typedef boost::algorithm::split_iterator<boost::string::iterator> b_split_iter_t;
  // typedef boost::algorithm::find_iterator<boost::string::iterator> b_find_iter_t;

  typedef boost::algorithm::split_iterator<boost::string_ref::iterator> b_split_refiter_t;
  typedef boost::algorithm::find_iterator<boost::string_ref::iterator> b_find_refiter_t;


  static const std::pair<boost::string_ref, boost::string_ref> ignore_this_delimiters_ref = std::make_pair(boost::string_ref(ignore_this_delimiters.first),
                                                                                                           boost::string_ref(ignore_this_delimiters.second));

  template <typename string_t>
  vec_of_string_refs_t simple_split(const boost::string_ref &_data, const string_t &_sep = ",")
  {
    vec_of_string_refs_t value;

    if(_sep.empty() || _data.empty())
      return value;


    auto it = make_split_iterator(_data, boost::first_finder(_sep, boost::is_iequal()));
    auto end = b_split_refiter_t();

    for(;it!=end;++it)
    {
      value.emplace_back(boost::string_ref{it->begin(),(std::size_t)std::distance(it->begin(),it->end())});
    }

    return value;
  }

  template <typename string_t>
  vec_of_string_refs_t informed_split_impl(const boost::string_ref &_data, const string_t &_sep = ",")
  {
    vec_of_string_refs_t value;

    if(_sep.empty() || _data.empty())
      return value;

    auto start_verbatim_itr = make_find_iterator(_data, boost::first_finder(ignore_this_delimiters.first, boost::is_iequal()));
    auto stop_verbatim_itr = make_find_iterator(_data, boost::first_finder(ignore_this_delimiters.second, boost::is_iequal()));
    auto fend = b_find_refiter_t();

    auto last_begin = _data.begin();

    boost::string_ref search_string{last_begin,(std::size_t)std::distance(last_begin, start_verbatim_itr->begin())};
    auto sep_finder = boost::first_finder(_sep, boost::is_iequal());
    vec_of_string_refs_t temp;

    for(;stop_verbatim_itr!=fend;
        ++start_verbatim_itr,++stop_verbatim_itr)
    {

      search_string = boost::string_ref{last_begin,(std::size_t)std::distance(last_begin, start_verbatim_itr->begin())};
      temp = simple_split(search_string,_sep);

      for( std::size_t i = 0;i<temp.size()-1;++i)
        value.emplace_back(temp[i]);

      auto end_of_stop = stop_verbatim_itr->begin()+ignore_this_delimiters.second.size();
      boost::string_ref remainder{end_of_stop,(std::size_t)std::distance(end_of_stop,_data.end())};

      auto last_begin_itr = make_find_iterator(remainder,sep_finder);
      while(last_begin_itr!=fend){
        auto next_start = make_find_iterator(remainder,boost::first_finder(ignore_this_delimiters.first, boost::is_iequal()));
        auto next_stop = make_find_iterator(remainder,boost::first_finder(ignore_this_delimiters.second, boost::is_iequal()));


        if(last_begin_itr->begin() < next_start->begin() && last_begin_itr->begin() < next_stop->begin())
          break;
        end_of_stop = next_stop->begin()+ignore_this_delimiters.second.size();
        remainder = boost::string_ref{end_of_stop,(std::size_t)std::distance(end_of_stop,_data.end())};
        last_begin_itr = make_find_iterator(remainder,sep_finder);
      }

      if(last_begin_itr==fend)
        last_begin = _data.end();
      else
        last_begin = last_begin_itr->begin();

      value.emplace_back(
        boost::string_ref{temp.back().begin(),
            (std::size_t)std::distance(temp.back().begin(),last_begin)
            }
        );
      last_begin += _sep.size();
    }

    if(last_begin < _data.end()){
      search_string = boost::string_ref{last_begin,(std::size_t)std::distance(last_begin, _data.end())};
      temp = simple_split(search_string,_sep);
      for( std::size_t i = 0;i<temp.size();++i)
        value.emplace_back(temp[i]);

    }

    return value;

  }


  template <typename string_t>
  vec_of_string_refs_t informed_split(const boost::string_ref &_data, const string_t &_sep = ",")
  {

    vec_of_string_refs_t value;

    if(_sep.empty() || _data.empty())
      return value;

    auto ignore_fitr = make_find_iterator(_data, boost::first_finder(ignore_this_delimiters.first, boost::is_iequal()));
    auto ignore_sitr = make_find_iterator(_data, boost::first_finder(ignore_this_delimiters.second, boost::is_iequal()));
    auto fend = b_find_refiter_t();

    if(ignore_fitr==fend && ignore_sitr==fend)
      return simple_split(_data,_sep);

    if(ignore_fitr!=fend && ignore_sitr!=fend)
      return informed_split_impl(_data,_sep);

    std::cerr << "[sqy::string_parsers::informed_split] unable to split malformed string " << _data
              << " (found only 1 verbatim delimeter, expected 2)\n";

    return value;
  }


  template <typename string_t>
  vec_of_string_refs_t manual_split_string_ref_to_ref(const boost::string_ref &_data, const string_t &_sep = ",")
  {
    vec_of_string_refs_t value;

    if(_sep.empty() || _data.empty())
      return value;

    std::size_t approx_count = std::count(_data.begin(), _data.end(), _sep.front());
    value.reserve(approx_count);

    std::size_t first = 0;

    boost::string_ref to_search(_data);
    std::size_t pos_of_ignore_delim = to_search.find(ignore_this_delimiters.first);

    while(to_search.size())
    {
      first = to_search.find_first_of(_sep);

      if(first != boost::string_ref::npos &&
         pos_of_ignore_delim != boost::string_ref::npos &&
         pos_of_ignore_delim < first  // did we pass a delimiter of ignorance?
         ){
        pos_of_ignore_delim = to_search.find(ignore_this_delimiters.second);

        boost::string_ref tmp = to_search;
        tmp.remove_prefix(pos_of_ignore_delim + ignore_this_delimiters.second.size());
        first = tmp.find_first_of(_sep);
        if(first != boost::string_ref::npos)
          first += pos_of_ignore_delim + ignore_this_delimiters.second.size();
      }

      auto end_itr = first != boost::string_ref::npos ? to_search.begin() + first : to_search.end();
      std::size_t len = (std::size_t)std::distance(to_search.begin(), end_itr);
      boost::string_ref result(&*to_search.begin(), len);

      value.push_back(result);

      if(first == boost::string_ref::npos)
      {
        break;
      }

      to_search.remove_prefix(first + _sep.size());
      pos_of_ignore_delim -= (first + _sep.size());
    }


    value.reserve(value.size());

    return value;
  }


  template <typename iter_t, typename string_t>
  vec_of_string_refs_t split_char_range(iter_t _begin, iter_t _end,
					const string_t& _sep = ","
					){

    const std::size_t len = (std::size_t)std::distance(_begin,_end);
    boost::string_ref ref(&*_begin,len);
    return informed_split(ref,_sep);

  }

  template <typename string_t>
  vec_of_strings_t split_string_ref_by(const boost::string_ref& _data,
				       const string_t& _sep = ","
				       ){


    auto tmp = informed_split(_data,_sep);
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
      const std::size_t len = (std::size_t)std::distance(_begin,_end);

      if(!len)
        return value;

      if(seperators_.empty())
        return value;


      boost::string_ref msg(&*_begin,len);

      vec_of_string_refs_t major_keys = informed_split(msg,
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

          vec_of_string_refs_t options = informed_split(in_brackets,
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
      const std::size_t len = (std::size_t)std::distance(_begin,_end);

      if(!len)
        return value;

      if(seperators_.empty())
        return value;


      boost::string_ref msg(&*_begin,len);

      vec_of_string_refs_t major_keys = informed_split(msg,
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
      const std::size_t len = (std::size_t)std::distance(_begin,_end);

      if(!len)
        return value;

      if(seperators_.empty())
        return value;


      boost::string_ref msg(&*_begin,len);

      vec_of_string_refs_t major_keys = informed_split(msg,
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
      const std::size_t len = (std::size_t)std::distance(_begin,_end);

      if(!len)
        return value;

      if(seperators_.empty())
        return value;


      boost::string_ref msg(&*_begin,len);

      vec_of_string_refs_t major_keys = informed_split(msg,seperators_[1]);

      for(const boost::string_ref& maj_key : major_keys ){

        auto dist = maj_key.find(seperators_.back());
        if(dist == std::string::npos)
          dist = maj_key.size();

        std::string key(maj_key.begin(),
                        maj_key.begin()+dist);

        auto kv_begin = (dist+seperators_.back().size()) < maj_key.size() ? (maj_key.begin()+dist+seperators_.back().size()) : maj_key.begin();
        std::string key_value(kv_begin,
                              maj_key.end());

        value[ key ] = key_value;
      }

      return value;
    }



  };

  namespace parsing {
    /**
       \brief function estimate how many bytes a range may consume as verbatim string

       \return
       \retval

    */
    template <typename iter_t>
    static std::size_t verbatim_bytes(iter_t _begin, iter_t _end){

      typedef typename std::iterator_traits<iter_t>::value_type value_t;

      const std::size_t len = (std::size_t)std::distance(_begin, _end);
      const std::size_t bytes = len*sizeof(value_t);

      const std::size_t exp_size = base64::encoded_bytes(bytes)+ignore_this_delimiters.first.size()+ignore_this_delimiters.second.size();
      return exp_size;

    }

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

      const std::size_t len = (std::size_t)std::distance(_begin, _end);
      const std::size_t bytes = len*sizeof(value_t);
      std::size_t exp_size = verbatim_bytes(_begin,_end);

      std::string value;

      if(!len)
        return value;

      value.resize(exp_size);

      char* iter = std::copy(ignore_this_delimiters.first.begin(), ignore_this_delimiters.first.end(),
                             (char*)value.data());

      const char* src = reinterpret_cast<const char*>(&*_begin);
      iter = base64::fast_encode_impl(src,src+bytes,(char*)iter);

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

      const std::size_t len = (std::size_t)std::distance(_begin, _end);
      const std::size_t bytes = len*sizeof(value_t);


      iter_t value = _begin;

      if(!len)
        return value;

      auto src = _verbatim.data() + ignore_this_delimiters.first.size();
      auto src_end_pos = _verbatim.rfind(ignore_this_delimiters.second);
      if(src_end_pos!=std::string::npos)
        src_end_pos -= ignore_this_delimiters.first.size();
      else
        src_end_pos = _verbatim.size() - ignore_this_delimiters.first.size();

      const std::size_t decoded_bytes = base64::decoded_bytes(src,src+src_end_pos);
      if(decoded_bytes > bytes)
        return value;

      char* dst = reinterpret_cast<char*>(&*_begin);
      auto dst_end = base64::decode_impl(src,src+src_end_pos,dst);//std::copy(src,src+src_end_pos,dst);

      std::size_t decoded_elements = (std::size_t)std::distance(dst,dst_end);
      value += (decoded_elements/sizeof(value_t));

      return value;
    }



    template <typename value_t,typename string_t>
    static std::size_t verbatim_yields_n_items_of(const string_t& _verbatim ){

      std::size_t decoded_nbytes = base64::decoded_bytes(_verbatim.data()+ignore_this_delimiters.first.size(),
                                                         _verbatim.data() + _verbatim.size()-ignore_this_delimiters.second.size());

      return decoded_nbytes/sizeof(value_t);

    }

  };

};

#endif /* _STRING_PARSERS_H_ */
