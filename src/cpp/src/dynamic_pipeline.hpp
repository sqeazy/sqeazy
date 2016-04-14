#ifndef _DYNAMIC_PIPELINE_H_
#define _DYNAMIC_PIPELINE_H_
#include <utility>
#include <cmath>
#include <iostream>
#include <vector>
#include <string>
#include <initializer_list>
#include <cstdint>
#include <typeinfo>
#include <stdexcept>
#include <type_traits>
#include <memory>

#include "string_parsers.hpp"
#include "sqeazy_utils.hpp"

#include "dynamic_stage.hpp"
#include "dynamic_stage_factory.hpp"

#include "sqeazy_header.hpp"

namespace sqeazy
{

  template <typename T>
  using default_filter_factory = stage_factory<blank_filter<T> >;

  template <typename T>
  using default_sink_factory = stage_factory<blank_sink<T> >;
  
  template <typename head_t, typename tail_t, bool flag = true>
  struct binary_select_type{
    typedef head_t type;
  };

  template <typename head_t, typename tail_t>
  struct binary_select_type<head_t,tail_t,false>{
    typedef tail_t type;
  };


  
  template <
    typename raw_t,
    template<typename = raw_t> class filter_factory_t = default_filter_factory,
    typename sink_factory_t = default_sink_factory<raw_t>
    >
  struct dynamic_pipeline : public sink<raw_t>
  {

    typedef sink<raw_t> base_t;
    typedef raw_t incoming_t;
    typedef typename base_t::out_type outgoing_t;

    static_assert(std::is_arithmetic<incoming_t>::value==true, "[dynamic_pipeline.hpp:56] received non-arithmetic type for input");
    static_assert(std::is_arithmetic<outgoing_t>::value==true, "[dynamic_pipeline.hpp:57] received non-arithmetic type for output");
    //static_assert(std::is_same<outgoing_t,incoming_t>::value==false, "[dynamic_pipeline.hpp:58] incoming and outgoing types equal!");
            
    typedef sink<incoming_t> sink_t;
    typedef std::shared_ptr< sink<incoming_t> > sink_ptr_t;
    
    typedef filter<incoming_t> head_filter_t;
    typedef filter<outgoing_t> tail_filter_t;
    
    typedef sqeazy::stage_chain<head_filter_t> head_chain_t;
    typedef sqeazy::stage_chain<tail_filter_t> tail_chain_t;

    
    head_chain_t head_filters_;
    tail_chain_t tail_filters_;
    sink_ptr_t sink_;


    
    /**
       \brief given a buffer that contains a valid header, this static method can fill the filter_holder and set the sink
       
       \param[in] _buffer string to parse
       \param[in] f filter factory that is aware of all possible filters to choose from
       \param[in] s sink factory that is aware of all possible sinks to choose from

       \return 
       \retval 
       
    */
    static dynamic_pipeline bootstrap(const std::string &_config)
    {

      if(_config.empty())
	return dynamic_pipeline();
      else
	return bootstrap(_config.begin(), _config.end());
    }

    /**
       \brief given an iterator range that contains a valid header, this static method can fill the filter_holder and set the sink
       
       \param[in] _buffer string to parse
       \param[in] f filter factory that is aware of all possible filters to choose from
       \param[in] s sink factory that is aware of all possible sinks to choose from

       \return 
       \retval 
       
    */
    template <
      typename iterator_t
      >
    static dynamic_pipeline bootstrap(iterator_t _begin, iterator_t _end)
    {
      sqeazy::image_header hdr(_begin,_end);

      std::string pipeline = hdr.pipeline();
      
      return from_string(pipeline);
    }
    
    /**
       \brief given a string, this static method can fill the filter_holder and set the sink
       
       \param[in] _config string to parse
       \param[in] head_f filter factory that yields possible filters that filter incoming_type to incoming_type arrays
       \param[in] s sink factory that yields possible sinks to go from incoming_type to outgoing_type
       \param[in] tail_f filter factory that yields possible filters that filter outgoing_type to outgoing_type arrays
       \return 
       \retval 
       
    */
    static dynamic_pipeline from_string(const char* _config_str)
    {

      using head_filter_factory_t = filter_factory_t<incoming_t>;
      using tail_filter_factory_t = filter_factory_t<outgoing_t>;

      std::string _config = _config_str;
      sqeazy::vec_of_pairs_t steps_n_args = sqeazy::parse_by(_config.begin(),
							     _config.end(),
							     "->");

      dynamic_pipeline value;

      for(const auto &pair : steps_n_args)
      {
	if(!value.sink_){
	  if(head_filter_factory_t::has(pair.first)){
	    value.add((head_filter_factory_t::template create<head_filter_t>(pair.first, pair.second)));
	    continue;
	  }
	
	  if(sink_factory_t::has(pair.first)){
	    //auto temp = sink_factory_t::template create< sink_t >(pair.first, pair.second);
	    //value.sink_ = temp;
	    value.add(sink_factory_t::template create< sink_t >(pair.first, pair.second));
	  }
	} else {
	   if(tail_filter_factory_t::has(pair.first)){
	    value.add((tail_filter_factory_t::template create<tail_filter_t>(pair.first, pair.second)));
	    
	  }
	}
	//what if sink is present in _config, but not in factory
      }

      
      return value;
    }

    static bool can_be_built_from(const char* _config_str, std::string _sep = "->")
    {

      using head_filter_factory_t = filter_factory_t<incoming_t>;
      using tail_filter_factory_t = filter_factory_t<outgoing_t>;

      std::string _config = _config_str;
      sqeazy::vec_of_pairs_t steps_n_args = sqeazy::parse_by(_config.begin(),
							     _config.end(),
							     _sep);

      bool value = false;
      std::uint32_t found = 0;
      
      for(const auto &pair : steps_n_args)
      {
	
	  if(head_filter_factory_t::has(pair.first)){
	    found++;
	    continue;
	  }
	
	  if(sink_factory_t::has(pair.first)){
	    found++;
	    continue;
	  }
	  
	  if(tail_filter_factory_t::has(pair.first)){
	    found++;
	  }
	
      }

      value = found == steps_n_args.size();
      int rebuild_size = _sep.size()*(steps_n_args.size()-1);
      for( const auto& item : steps_n_args )
	rebuild_size += item.first.size();

      value = value && rebuild_size == _config.size();
      return value;
    }
    
    /**
       \brief given a char buffer, this static method can fill the filter_holder and set the sink
       
       \param[in] _config string to parse
       \param[in] f filter factory that is aware of all possible filters to choose from
       \param[in] s sink factory that is aware of all possible sinks to choose from

       \return 
       \retval 
       
    */
    static dynamic_pipeline from_string(const std::string& _config)
    {
      return from_string(_config.c_str());
    }


    
    friend void swap(dynamic_pipeline &_lhs, dynamic_pipeline &_rhs)
    {
      std::swap(_lhs.head_filters_, _rhs.head_filters_);
      std::swap(_lhs.tail_filters_, _rhs.tail_filters_);

      std::swap(_lhs.sink_, _rhs.sink_);
    }

    /**
       \brief construct an empty pipeline


       \return
       \retval

    */
    dynamic_pipeline()
        :
      head_filters_(),
      tail_filters_(),
      sink_(nullptr){};

    /**
       //DEPRECATE THIS?//
       \brief construct an pipeline from given stages (mainly for debugging)
       
       \param[in] _size how many stages will the pipeline have

       \return
       \retval

    */
    dynamic_pipeline(std::initializer_list<typename head_chain_t::filter_ptr_t> _stages)
        : 
      head_filters_(_stages),
      tail_filters_(),
      sink_(nullptr)
    {

      // for(const head_filter_ptr_t& step : _stages)
      // {
      // 	head_filters_.push_back(step);
      // }
    };

    /**
       \brief copy-constructor 

       \param[in]

       \return
       \retval

    */
    template <template<typename> class T, typename U>
    dynamic_pipeline(const dynamic_pipeline<incoming_t, T, U>& _rhs):
      head_filters_(_rhs.head_filters_),
      tail_filters_(_rhs.tail_filters_),
      sink_        (_rhs.sink_        )
    {
    }

    /**
       \brief copy-by-assignment using copy&swap idiom

       \param[in]

       \return
       \retval

    */
    template <template<typename> class T, typename U>
    dynamic_pipeline &operator=(dynamic_pipeline<incoming_t, T, U> _rhs)
    {

      swap(*this, _rhs);

      return *this;
    }

    /**
       \brief how many stages are setup in the pipeline

       \return
       \retval

    */
    const std::size_t size() const { return head_filters_.size() + (sink_ ? 1 : 0) + tail_filters_.size(); }

    const bool empty() const { return (head_filters_.empty() && !sink_ && tail_filters_.empty()); }

    template <typename pointee_t>
    typename std::enable_if<!std::is_base_of<sink_t,pointee_t>::value>::type
    add(const std::shared_ptr<pointee_t>& _new_filter)
    {

      auto view = const_stage_view(_new_filter);
      
      if(std::is_base_of<typename head_chain_t::filter_base_t,pointee_t>::value){
        head_filters_.push_back(_new_filter);
	return;
      }

      if(std::is_base_of<typename tail_chain_t::filter_base_t,pointee_t>::value){
        tail_filters_.push_back(_new_filter);
	return;
      }
      
      std::ostringstream msg;
      msg << "sqeazy::dynamic_pipeline::add failed to push_back filter called " << _new_filter.get()->name() << "\n";
      throw std::runtime_error(msg.str());
    }

    
    
    // void add_sink(const sink_ptr_t& _new_sink)
    void add(const sink_ptr_t& _new_sink)
    {
      if(sink_)
    	sink_.reset();

      sink_ = _new_sink;
    }

    
    bool valid_filters() const
    {

      bool value = true;
      if(head_filters_.empty() && tail_filters_.empty())
        return !value;

      if(!head_filters_.empty()){
	value = value && head_filters_.valid();

	if(sink_){
	  auto back_view = const_stage_view(head_filters_.back());
	  auto sink_view = const_stage_view(sink_);
	  value = value && back_view->output_type() == sink_view->input_type();
	}
      }

      if(!tail_filters_.empty()){
	auto front_view = const_stage_view(tail_filters_.front());

	if(sink_){
	  auto sink_view = const_stage_view(sink_);
	  value = value && front_view->input_type() == sink_view->output_type();
      	}

	value = value && tail_filters_.valid();
      }
      return value;
    }

    std::string input_type() const
    {

      if(head_filters_.size())
        return const_stage_view(head_filters_.front())->input_type();

      if(sink_)
        return const_stage_view(sink_)->input_type();

      return std::string("");
    };

    std::string output_type() const
    {

      if(sink_)
      {
        return const_stage_view(sink_)->output_type();
      }
      else
      {
        if(tail_filters_.size())
        {
          return const_stage_view(tail_filters_.front())->output_type();
        }
	
	if(sink_)
        {
          return const_stage_view(sink_)->output_type();
        }

	if(head_filters_.size())
        {
          return const_stage_view(head_filters_.front())->output_type();
        }

	return std::string("");
      }
    };

    bool is_compressor() const {
      
      return sink_ ? true : false ;
      
    }

    /**
       \brief produce string of pipeline, separated by ->

       \param[in]

       \return
       \retval

    */
    std::string name() const
    {

      std::ostringstream value;

      if(head_filters_.size())
	value << head_filters_.name();

      if(is_compressor())
      {
        value << "->";
        value << const_stage_view(sink_)->name();
	std::string cfg = const_stage_view(sink_)->config();

	if(!cfg.empty())
	  value << "(" << cfg << ")";

      }

      if(tail_filters_.size())
	value << tail_filters_.name();

      return value.str();
    };

    std::string config() const
    {
      return name();
    }

    /**
       \brief encode one-dimensional array _in and write results to _out
       
       \param[in] 
       
       \return 
       \retval 
       
    */
    
    outgoing_t * encode(const incoming_t *_in, outgoing_t *_out, std::size_t _len) override final {

      std::vector<std::size_t> shape(1,_len);
      return encode(_in,_out,shape);

    }

        /**
       \brief encode one-dimensional array _in and write results to _out
       
       \param[in] _in input buffer
       \param[out] _out output buffer must be at least of size max_encoded_size
       \param[in] _shape shape in input buffer in nDim

       \return 
       \retval 
       
    */
    
    outgoing_t* encode(const incoming_t *_in, outgoing_t *_out, std::vector<std::size_t> _shape) override final {

      outgoing_t* value = nullptr;
      std::size_t len = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());
      // std::size_t max_len_byte = len*sizeof(incoming_t);

      ////////////////////// HEADER RELATED //////////////////
      //insert header
      sqeazy::image_header hdr(incoming_t(),
			       _shape,
			       name(),
			       len*sizeof(incoming_t));

      const std::intmax_t hdr_shift = hdr.size();
      char* output_buffer = reinterpret_cast<char*>(_out);
      std::copy(hdr.begin(), hdr.end(), output_buffer);
      outgoing_t* first_output = reinterpret_cast<outgoing_t*>(output_buffer+hdr_shift);

      ////////////////////// ENCODING //////////////////
      value = detail_encode(_in,first_output,_shape);
      
      ////////////////////// HEADER RELATED //////////////////
      //update header
      std::size_t compressed_size = value-first_output;
      hdr.set_compressed_size_byte<incoming_t>(compressed_size*sizeof(outgoing_t));
      hdr.set_pipeline<incoming_t>(name());

      if(hdr.size()!=hdr_shift){
	std::copy(first_output, first_output + compressed_size,
		  output_buffer+hdr.size()
		  );
	first_output = reinterpret_cast<outgoing_t*>(output_buffer+hdr.size());
      }
      
      std::copy(hdr.begin(), hdr.end(), output_buffer);
            
      value = (outgoing_t*)(output_buffer+hdr.size()+(compressed_size*sizeof(outgoing_t)));
            
      return value;

    }

    outgoing_t* detail_encode(const incoming_t *_in,
			      outgoing_t *_out,
			      std::vector<std::size_t> _shape)  {
      
      std::size_t len = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());
      // std::vector<incoming_t> temp_in(_in, _in+len);
      std::vector<incoming_t> temp;

      std::size_t max_len_byte = sink_ ? sink_->max_encoded_size(len*sizeof(incoming_t)) : 0;
      temp.resize(std::max(
			   std::ceil(max_len_byte/sizeof(incoming_t)),
			   double(len)
			   )
		  );

      incoming_t* head_filters_end = nullptr;
      if(head_filters_.size()){
	head_filters_end = head_filters_.encode(_in,
						     temp.data(),
						     _shape);

	if(!head_filters_end){
	  std::cerr << "[dynamic_pipeline::detail_encode] unable to process data with head_filters\n";
	  return nullptr;
	}
	
      } else {
	std::copy(_in,
		  _in + len,
		  temp.data());

      }
      
      outgoing_t* encoded_end = nullptr;
      if(sink_){
	encoded_end = sink_->encode(temp.data(),
				    _out,
				    _shape);
	

	std::intmax_t compressed_size = encoded_end-_out;
	if(tail_filters_.size()){

	  outgoing_t* casted_temp = reinterpret_cast<outgoing_t*>(temp.data());
	  
	  std::copy(_out, _out+compressed_size,casted_temp);

	  std::vector<std::size_t> casted_shape(1,compressed_size);
	  
	  encoded_end = tail_filters_.encode(casted_temp,
					     _out,
					     casted_shape);
	}
      }
      else{
	incoming_t* out_as_incoming = reinterpret_cast<incoming_t*>(_out);
	incoming_t* temp_end = nullptr;
	if(head_filters_.size())
	  temp_end = head_filters_end;
	else
	  temp_end = temp.data() + temp.size();
	
	std::copy(temp.data(), temp_end ,
		  out_as_incoming);
	encoded_end = reinterpret_cast<outgoing_t*>(out_as_incoming+(temp_end-temp.data()));
      }

      return encoded_end;
    }



    int detail_decode(const outgoing_t *_in, incoming_t *_out,
		      std::size_t in_size_bytes,
		      std::vector<std::size_t> out_shape) const {
      int value = 0;
      int err_code = 0;

      ////////////////////// DECODING //////////////////
      std::size_t output_len = std::accumulate(out_shape.begin(), out_shape.end(),
					       1,
					       std::multiplies<std::size_t>());
      std::size_t len = in_size_bytes/sizeof(outgoing_t);
      std::vector<incoming_t> temp(output_len,0);

      
      if(is_compressor()){
	typedef typename sink_t::out_type sink_out_t;

	const sink_out_t* compressor_begin = reinterpret_cast<const sink_out_t*>(_in);
	std::vector<outgoing_t> sink_in;
	  
	if(tail_filters_.size()){
	  const outgoing_t* tail_in = reinterpret_cast<const outgoing_t*>(_in);
	  sink_in.resize(in_size_bytes/sizeof(outgoing_t));
	  outgoing_t* tail_out = reinterpret_cast<outgoing_t*>(sink_in.data());
	  
	  err_code = tail_filters_.decode(tail_in,
					  tail_out,
					  in_size_bytes);
	  value += err_code ;
	  
	  compressor_begin = reinterpret_cast<const outgoing_t*>(sink_in.data());
	}
		

	err_code = sink_->decode(compressor_begin,
				 &temp[0],
				 in_size_bytes
				 );
	value += err_code ? err_code+10 : 0 ;
      }
      else{
	std::copy(_in,
		  _in+len,
		  reinterpret_cast<outgoing_t*>(temp.data()));
      }

      if(head_filters_.empty()){
	std::copy(temp.begin(),temp.begin()+output_len,_out);
      }
      else{
	
	err_code = head_filters_.decode(reinterpret_cast<const incoming_t*>(temp.data()),
					_out,
					out_shape);
	value += err_code ? err_code+100 : 0 ;
	
      }
      return value;

    }




    
      /**
       \brief decode one-dimensional array _in and write results to _out
       
       \param[in] 
       
       \return 
       \retval 
       
    */
    
    int decode(const outgoing_t *_in, incoming_t *_out, std::size_t _len) const override final {

      std::vector<std::size_t> shape = {_len};
      
      return decode(_in,_out,shape);

    }
    
     /**
       \brief decode one-dimensional array _in and write results to _out
       
       \param[in] _in input buffer
       \param[out] _out output buffer //NOTE: might be larger than _in for sink type pipelines
       \param[in] _shape of input buffer size in units of its type, aka outgoing_t
       
       \return error code encoded as 3-digit decimal number
		0   .. filter error code before sink
		10  .. sink error code
		100 .. filter error code after sink
       \retval 
       
    */
    
    int decode(const outgoing_t *_in, incoming_t *_out, std::vector<std::size_t> _shape) const override final {
      

      //FIXME: strange, we receive an n-dim array with _in that spans across the payload AND the header
      //       this would imply that almost always, _shape is 1D?
      std::size_t len = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());

      ////////////////////// HEADER RELATED //////////////////
      //load header
      const char* _in_char_begin = (const char*)_in;

      //FIXME: works only if len is greater than hdr.size()
      const char* _in_char_end = _in_char_begin + (len*sizeof(outgoing_t));
      
      image_header hdr(_in_char_begin,_in_char_end);
      std::vector<std::size_t> output_shape(hdr.shape()->begin(),
					    hdr.shape()->end());
      // std::intmax_t output_len = std::accumulate(output_shape.begin(),
      // 						 output_shape.end(),
      // 						 1,
      // 						 std::multiplies<std::intmax_t>());
      const outgoing_t* payload_begin = reinterpret_cast<const outgoing_t*>(_in_char_begin + hdr.size());
      size_t in_size_bytes = (len*sizeof(outgoing_t)) - hdr.size();
      
      int value = detail_decode(payload_begin, _out,
				in_size_bytes,
				output_shape);
      
      return value;
    }


    std::intmax_t max_encoded_size(std::intmax_t _incoming_size_byte){
      
      image_header hdr(incoming_t(),
		       _incoming_size_byte,
		       name()
		       );

      std::intmax_t value = hdr.size()+2;
      
      if(!is_compressor())
	return value+_incoming_size_byte;
      else
	return value+sink_->max_encoded_size(_incoming_size_byte);
    }

    template <typename T>
    std::intmax_t decoded_size(const T* _begin, const T* _end){

      static_assert(sizeof(T)==1, "[dynamic_pipeline.hpp::decoded_size] recived non-byte-size iterator");
      
      image_header found_header(_begin, _end);
      std::intmax_t value = found_header.raw_size_byte_as<incoming_t>();

      return value;
    }

    /**
       \brief obtain shape of decoded buffer that contains header from _begin to _end
       
       \param[in] 
       
       \return 
       \retval 
       
    */
    template <typename T>
    std::vector<std::size_t> decoded_shape(const T* _begin, const T* _end){

      static_assert(sizeof(T)==1, "[dynamic_pipeline.hpp::decoded_shape] recived non-byte-size iterator");

      image_header found_header(_begin, _end);
      std::vector<std::size_t> value(found_header.shape()->begin(),
				     found_header.shape()->end());

      return value;
    }
    
  };
}

#endif /* _DYNAMIC_PIPELINE_H_ */
