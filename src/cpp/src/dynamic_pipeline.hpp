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
       \brief copy-constructor using copy&swap idiom

       \param[in]

       \return
       \retval

    */
    dynamic_pipeline(const dynamic_pipeline& _rhs):
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

    template <typename pointee_t>
    void add(const std::shared_ptr<pointee_t>& _new_filter)
    {

      auto view = const_stage_view(_new_filter);
      //if(view->input_type() == typeid(incoming_t).name()){
      if(std::is_base_of<typename head_chain_t::filter_base_t,pointee_t>::value){
        head_filters_.push_back(_new_filter);
	return;
      }

      //      if(view->input_type() == typeid(outgoing_t).name()){
      if(std::is_base_of<typename tail_chain_t::filter_base_t,pointee_t>::value){
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
      // std::size_t max_len_byte = len*sizeof(raw_t);

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
      if(std::is_same<raw_t,compressed_t>::value)
	value = head_filters_.encode(_in,first_output,_shape);
      else
	value = detail_encode(_in,first_output,_shape);
      
      ////////////////////// HEADER RELATED //////////////////
      //update header
      std::size_t compressed_size = value-first_output;
      hdr.set_compressed_size_byte<raw_t>(compressed_size*sizeof(compressed_t));
      hdr.set_pipeline<raw_t>(name());

      if(hdr.size()!=hdr_shift){
	std::copy(first_output, first_output + compressed_size,
		  output_buffer+hdr.size()
		  );
	first_output = reinterpret_cast<compressed_t*>(output_buffer+hdr.size());
      }
      
      std::copy(hdr.begin(), hdr.end(), output_buffer);
            
      value = (compressed_t*)(output_buffer+hdr.size()+(compressed_size*sizeof(compressed_t)));
            
      return value;

    }

    compressed_t* detail_encode(const raw_t *_in,
				compressed_t *_out,
				std::vector<std::size_t> _shape)  {
      
     
      std::size_t len = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());
      // std::vector<raw_t> temp_in(_in, _in+len);
      std::vector<raw_t> temp;

      std::size_t max_len_byte = sink_->max_encoded_size(len*sizeof(raw_t));
      temp.resize(std::max(
			   std::ceil(max_len_byte/sizeof(raw_t)),
			   double(len)
			   )
		  );

      compressed_t* encoded_end = nullptr;
      
      if(head_filters_.size())
	encoded_end = head_filters_.encode(_in,&temp[0],_shape);
	
      
      if(sink_){
	encoded_end = (compressed_t*)sink_->encode(temp.data(),
						   _out,
						   _shape);
    

	std::intmax_t compressed_size = encoded_end-_out;
	if(tail_filters_.size()){

	  compressed_t* casted_temp = reinterpret_cast<compressed_t*>(temp.data());
	  
	  std::copy(_out, _out+compressed_size,casted_temp);


	  encoded_end = tail_filters_.encode(casted_temp,
					     _out,
					     _shape);
	}
      }
      else{
	std::copy(temp.data(), encoded_end - temp.data(),_out);
	encoded_end = _out + (encoded_end - temp.data());
      }

      return encoded_end;
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
       
       \return error code encoded as 3-digit decimal number
		0   .. filter error code before sink
		10  .. sink error code
		100 .. filter error code after sink
       \retval 
       
    */
    
    int decode(const compressed_t *_in, raw_t *_out, std::vector<std::size_t> _shape) const override final {
      

      //FIXME: strange, we receive an n-dim array with _in that spans across the payload AND the header
      //       this would imply that almost always, _shape is 1D?
      std::size_t len = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());

      ////////////////////// HEADER RELATED //////////////////
      //load header
      const char* _in_char_begin = (const char*)_in;

      //FIXME: works only if len is greater than hdr.size()
      const char* _in_char_end = _in_char_begin + (len*sizeof(compressed_t));
      
      image_header hdr(_in_char_begin,_in_char_end);
      std::vector<std::size_t> output_shape(hdr.shape()->begin(),
					    hdr.shape()->end());
      // std::intmax_t output_len = std::accumulate(output_shape.begin(),
      // 						 output_shape.end(),
      // 						 1,
      // 						 std::multiplies<std::intmax_t>());
      auto payload_begin = reinterpret_cast<const compressed_t*>(_in_char_begin + hdr.size());
      size_t in_size_bytes = (len*sizeof(compressed_t)) - hdr.size();

      int value = 0;
	    
      if(std::is_same<compressed_t,raw_t>::value)
	value = head_filters_.decode(payload_begin,_out,_shape);
      else
	value = detail_decode(payload_begin, _out,
			      in_size_bytes,
			      output_shape);

      return value;
    }



    int detail_decode(const compressed_t *_in, raw_t *_out,
		      std::size_t in_size_bytes,
		      std::vector<std::size_t> out_shape) const {
      int value = 0;
      int err_code = 0;

      // //FIXME: strange, we receive an n-dim array with _in that spans across the payload AND the header
      // //       this would imply that almost always, _shape is 1D?
      // std::size_t len = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());

      // ////////////////////// HEADER RELATED //////////////////
      // //load header
      // const char* _in_char_begin = (const char*)_in;

      // //FIXME: works only if len is greater than hdr.size()
      // const char* _in_char_end = _in_char_begin + (len*sizeof(compressed_t));
      
      // image_header hdr(_in_char_begin,_in_char_end);
      // std::vector<std::size_t> output_shape(hdr.shape()->begin(),
      // 					    hdr.shape()->end());
      // std::intmax_t output_len = std::accumulate(output_shape.begin(),
      // 						 output_shape.end(),
      // 						 1,
      // 						 std::multiplies<std::intmax_t>());
      // auto payload_begin = reinterpret_cast<const compressed_t*>(_in_char_begin + hdr.size());
      
      ////////////////////// DECODING //////////////////
      std::size_t output_len = std::accumulate(out_shape.begin(), out_shape.end(),
					       1,
					       std::multiplies<std::size_t>());
      std::size_t len = in_size_bytes/sizeof(compressed_t);
      std::vector<raw_t> temp(output_len,0);

      
      if(is_compressor()){
	typedef typename sink_t::out_type sink_out_t;

	const sink_out_t* compressor_begin = reinterpret_cast<const sink_out_t*>(_in);
	std::vector<compressed_t> sink_in;
	  
	if(tail_filters_.size()){
	  const sink_out_t* tail_in = compressor_begin;
	  sink_in.resize(in_size_bytes/sizeof(compressed_t));
	  const sink_out_t* tail_out = reinterpret_cast<sink_out_t*>(sink_in.data());
	  
	  err_code = tail_filters_.decode(tail_in,
					  tail_out,
					  in_size_bytes);
	  value += err_code ;
	  
	  compressor_begin = sink_in.data();
	}
		

	err_code = sink_->decode(compressor_begin,
				 &temp[0],
				 in_size_bytes
				 );
	value += err_code+10 ;
      }
      else{
	std::copy(_in,_in+len,temp.begin());
      }

      if(head_filters_.empty()){
	std::copy(temp.begin(),temp.begin()+output_len,_out);
      }
      else{
	
	err_code = head_filters_.decode(&temp[0],
					_out,
					out_shape);
	value += err_code + 100;
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
