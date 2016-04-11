#ifndef _DYNAMIC_PIPELINE_H_
#define _DYNAMIC_PIPELINE_H_
#include <utility>
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
#include "stage_factory.hpp"

#include "sqeazy_header.hpp"

namespace sqeazy
{

  
  
  template <typename head_t, typename tail_t, bool flag = true>
  struct binary_select_type{
    typedef head_t type;
  };

  template <typename head_t, typename tail_t>
  struct binary_select_type<head_t,tail_t,false>{
    typedef tail_t type;
  };

  
  template <typename raw_t, typename compressed_t = raw_t>
  struct dynamic_pipeline : public binary_select_type<filter<raw_t>,//true
						      sink<raw_t,compressed_t>,//false
						      std::is_same<raw_t,compressed_t>::value
						      >::type
  {


    typedef typename binary_select_type<filter<raw_t>,//true
					sink<raw_t, compressed_t>,//false
					std::is_same<raw_t,compressed_t>::value
					>::type
    base_t;

    typedef raw_t incoming_t;
    typedef typename base_t::out_type outgoing_t;

    static_assert(std::is_arithmetic<incoming_t>::value==true, "[dynamic_pipeline.hpp:56] received non-arithmetic type for input");
    static_assert(std::is_arithmetic<outgoing_t>::value==true, "[dynamic_pipeline.hpp:57] received non-arithmetic type for output");
        
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
    template <typename filter_factory_t = stage_factory<blank_filter>,
	      typename sink_factory_t = stage_factory<blank_sink>
	      >
    static dynamic_pipeline bootstrap(const std::string &_config,
				      filter_factory_t f = stage_factory<blank_filter>(),
				      sink_factory_t s = stage_factory<blank_sink>())
    {

      if(_config.empty())
	return dynamic_pipeline();
      else
	return bootstrap(_config.begin(), _config.end(),f,s);
    }

    /**
       \brief given an iterator range that contains a valid header, this static method can fill the filter_holder and set the sink
       
       \param[in] _buffer string to parse
       \param[in] f filter factory that is aware of all possible filters to choose from
       \param[in] s sink factory that is aware of all possible sinks to choose from

       \return 
       \retval 
       
    */
    template <typename iterator_t,
	      typename filter_factory_t = stage_factory<blank_filter>,
	      typename sink_factory_t = stage_factory<blank_sink>
	      >
    static dynamic_pipeline bootstrap(iterator_t _begin, iterator_t _end,
				      filter_factory_t f = stage_factory<blank_filter>(),
				      sink_factory_t s = stage_factory<blank_sink>())
    {
      sqeazy::image_header hdr(_begin,_end);

      std::string pipeline = hdr.pipeline();
      
      return from_string(pipeline,f,s);
    }
    
    /**
       \brief given a string, this static method can fill the filter_holder and set the sink
       
       \param[in] _config string to parse
       \param[in] head_f filter factory that yields possible filters that filter raw_type to raw_type arrays
       \param[in] s sink factory that yields possible sinks to go from raw_type to compressed_type
       \param[in] tail_f filter factory that yields possible filters that filter compressed_type to compressed_type arrays
       \return 
       \retval 
       
    */
    template <typename head_filter_factory_t = stage_factory<blank_filter>,
	      typename sink_factory_t = stage_factory<blank_sink>,
	      typename tail_filter_factory_t = stage_factory<blank_filter>
	      >
    static dynamic_pipeline from_string(const std::string &_config,
					head_filter_factory_t head_f = stage_factory<blank_filter>(),
					sink_factory_t s = stage_factory<blank_sink>(),
					tail_filter_factory_t tail_f = stage_factory<blank_filter>())
    {

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
	    auto temp = sink_factory_t::template create< sink_t >(pair.first, pair.second);
	    value.sink_ = temp;
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

    /**
       \brief given a char buffer, this static method can fill the filter_holder and set the sink
       
       \param[in] _config string to parse
       \param[in] f filter factory that is aware of all possible filters to choose from
       \param[in] s sink factory that is aware of all possible sinks to choose from

       \return 
       \retval 
       
    */
    template <typename filter_factory_t = stage_factory<blank_filter>,
	      typename sink_factory_t = stage_factory<blank_sink>
	      >
    static dynamic_pipeline from_string(const char *_config,
				 filter_factory_t f = stage_factory<blank_filter>(),
				 sink_factory_t s = stage_factory<blank_sink>())
    {
      std::string config = _config;

      return from_string(config, f, s);
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
    dynamic_pipeline(std::initializer_list<head_filter_ptr_t> _stages)
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
       \brief copy-constructor using copy&swap idiom

       \param[in]

       \return
       \retval

    */
    template <typename T>
    dynamic_pipeline(const T& _rhs):
      head_filters_(_rhs.head_filters_),
      tail_filters_(_rhs.tail_filters_),
      sink_(_rhs.sink_)
    {

    }

    /**
       \brief copy-by-assignment using copy&swap idiom

       \param[in]

       \return
       \retval

    */
    dynamic_pipeline &operator=(dynamic_pipeline _rhs)
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

    void add(const head_filter_ptr_t& _new_filter)
    {

      auto view = const_stage_view(_new_filter);
      if(view->input_type() == typeid(incoming_t).name()){
        head_filters_.push_back(_new_filter);
	return;
      }

      if(view->input_type() == typeid(outgoing_t).name()){
        tail_filters_.push_back(_new_filter);
	return;
      }
      
      throw std::runtime_error("sqeazy::dynamic_pipeline::add failed to push_back head_filter");
    }

    
    
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

      value = value && head_filters_.valid();
      value = value && tail_filters_.valid();
      
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
      
      return base_t::is_compressor ;
      
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
    
    compressed_t * encode(const raw_t *_in, compressed_t *_out, std::size_t _len) override final {

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
    
    compressed_t* encode(const raw_t *_in, compressed_t *_out, std::vector<std::size_t> _shape) override final {

      compressed_t* value = nullptr;
      std::size_t len = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());
      std::size_t max_len_byte = len*sizeof(raw_t);

      ////////////////////// HEADER RELATED //////////////////
      //insert header
      sqeazy::image_header hdr(raw_t(),
			       _shape,
			       name(),
			       len*sizeof(raw_t));

      const std::intmax_t hdr_shift = hdr.size();
      std::int8_t* output_buffer = reinterpret_cast<std::int8_t*>(_out);
      std::copy(hdr.begin(), hdr.end(), output_buffer);
      compressed_t* first_output = reinterpret_cast<compressed_t*>(output_buffer+hdr_shift);

      ////////////////////// ENCODING //////////////////
      //prepare temp data for encoding
      std::vector<raw_t> temp_in(_in, _in+len);
      std::vector<raw_t> temp_out;

      if(is_compressor()){
	max_len_byte = sink_->max_encoded_size(len*sizeof(raw_t));
	temp_out.resize(max_len_byte/sizeof(raw_t) > len ? max_len_byte/sizeof(raw_t) : len );
      }
      else {
	temp_out.resize(temp_in.size());
      }

      raw_t* out = &temp_out[0];

      if(head_filters_.size()){
	// for( std::size_t fidx = 0;fidx<head_filters_.size();++fidx )
	//   {
	  
	//     auto encoded_end = head_filters_[fidx]->encode(&temp_in[0],
	// 						   out,
	// 						   _shape);
	//     value = reinterpret_cast<decltype(value)>(encoded_end);
	//     std::copy(out, out+len,temp_in.begin());
	//   }
	auto encoded_end = head_filters_.encode(&temp_in[0],out,_shape);
	value = reinterpret_cast<decltype(value)>(encoded_end);
	std::copy(out, out+len,temp_in.begin());
      }
      
      if(is_compressor()){
	value = (compressed_t*)sink_->encode(&temp_in[0],
					     (typename sink_t::out_type*)out,
					     _shape);

      }

      std::intmax_t compressed_size = value-((decltype(value))&temp_out[0]);
      if(tail_filters_.size()){

	std::copy(out, out+compressed_size,temp_in.begin());
	// for( std::size_t fidx = 0;fidx<tail_filters_.size();++fidx )
	//   {
	  
	//     auto encoded_end = tail_filters_[fidx]->encode(&temp_in[0],
	// 						   out,
	// 						   _shape);
	//     value = reinterpret_cast<decltype(value)>(encoded_end);
	//     std::copy(out, out+len,temp_in.begin());
	//   }
	auto encoded_end = tail_filters_.encode(&temp_in[0],out,_shape);
	value = reinterpret_cast<decltype(value)>(encoded_end);
	std::copy(out, out+len,temp_in.begin());
      }
      
      
      ////////////////////// HEADER RELATED //////////////////
      //update header
      compressed_size = value-((decltype(value))&temp_out[0]);
      hdr.set_compressed_size_byte<raw_t>(compressed_size*sizeof(compressed_t));
      hdr.set_pipeline<raw_t>(name());
	
      if(hdr.size()!=hdr_shift)
	first_output = reinterpret_cast<compressed_t*>(output_buffer+hdr.size());
	
      std::copy(hdr.begin(), hdr.end(), output_buffer);

      compressed_t* temp_out_begin = reinterpret_cast<compressed_t*>(&temp_out[0]);
      compressed_t* temp_out_end = temp_out_begin+compressed_size;
      std::copy(temp_out_begin, temp_out_end, first_output);
            
      value = (compressed_t*)(output_buffer+hdr.size()+(compressed_size*sizeof(compressed_t)));
            
      return value;

    }

    
      /**
       \brief decode one-dimensional array _in and write results to _out
       
       \param[in] 
       
       \return 
       \retval 
       
    */
    
    int decode(const compressed_t *_in, raw_t *_out, std::size_t _len) const override final {

      std::vector<std::size_t> shape = {_len};
      
      return decode(_in,_out,shape);

    }
    
    /**
       \brief decode one-dimensional array _in and write results to _out
       
       \param[in] _in input buffer
       \param[out] _out output buffer //NOTE: might be larger than _in for sink type pipelines
       \param[in] _shape of input buffer size in units of its type, aka compressed_t
       
       \return 
       \retval 
       
    */
    
    int decode(const compressed_t *_in, raw_t *_out, std::vector<std::size_t> _shape) const override final {

      int value = 0;
      int err_code = 0;
      std::size_t len = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());

      //load header
      const char* _in_char_begin = (const char*)_in;
      const char* _in_char_end = _in_char_begin + (len*sizeof(compressed_t));
      image_header hdr(_in_char_begin,_in_char_end);
      std::vector<std::size_t> output_shape(hdr.shape()->begin(),
					    hdr.shape()->end());
      std::intmax_t output_len = std::accumulate(output_shape.begin(),
						 output_shape.end(),
						 1,
						 std::multiplies<std::intmax_t>());

      std::vector<raw_t> temp(output_len,0);
      
      

      
      if(is_compressor()){
	auto payload_begin = reinterpret_cast<const compressed_t*>(_in_char_begin + hdr.size());
	const typename sink_t::out_type* compressor_begin = payload_begin;
	
	if(tail_filters_.size()){
	  err_code = head_filters_.decode((const typename sink_t::out_type*)payload_begin,
					  &temp[0],
					  len-hdr.size());
	  value += err_code ? (10*(fidx+1))+err_code : 0;
	  ????????????????
	}
	

	err_code = sink_->decode(compressor_begin,
				 &temp[0],
				 len - hdr.size()
				 );
	value += err_code ;
      }
      else{
	auto payload_begin = reinterpret_cast<const raw_t*>(_in_char_begin + hdr.size());
	std::copy(payload_begin,payload_begin+output_len,temp.begin());
      }

      if(head_filters_.empty()){
	std::copy(temp.begin(),temp.begin()+output_len,_out);
      }
      else{
	
	// for( std::size_t fidx = 0;fidx<head_filters_.size();++fidx )
	//   {
	    
	//     err_code = head_filters_[fidx]->decode(&temp[0],
	// 				      _out,
	// 				      output_shape);
	//     value += err_code ? (10*(fidx+1))+err_code : 0;
	//     std::copy(_out, _out+output_len,temp.begin());
	//   }
	
	err_code = head_filters_.decode(&temp[0],
					_out,
					output_shape);
	value += err_code ? (10*(fidx+1))+err_code : 0;
	std::copy(_out, _out+output_len,temp.begin());

      }
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

    std::intmax_t decoded_size(const std::int8_t* _begin, const std::int8_t* _end){

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
    std::vector<std::size_t> decoded_shape(const std::int8_t* _begin, const std::int8_t* _end){

      image_header found_header(_begin, _end);
      std::vector<std::size_t> value(found_header.shape()->begin(),
				     found_header.shape()->end());

      return value;
    }
    
  };
}

#endif /* _DYNAMIC_PIPELINE_H_ */
