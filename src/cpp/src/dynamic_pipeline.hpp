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

#include "sqeazy_utils.hpp"
#include "dynamic_stage.hpp"

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

    
    typedef sink<incoming_t> sink_t;
    typedef filter<incoming_t> filter_t;
    typedef stage<incoming_t> stage_t;

    typedef std::shared_ptr< filter<incoming_t> > filter_ptr_t;
    typedef std::shared_ptr< sink<incoming_t> > sink_ptr_t;
    typedef std::vector<filter_ptr_t> filter_holder_t;

    static_assert(std::is_arithmetic<incoming_t>::value==true, "[dynamic_pipeline.hpp:31] received non-arithmetic type for input");
    static_assert(std::is_arithmetic<outgoing_t>::value==true, "[dynamic_pipeline.hpp:31] received non-arithmetic type for output");
    
    filter_holder_t filters_;
    sink_ptr_t sink_;


    
    /**
       \brief given a string, this static method can fill the filter_holder and set the sink
       
       \param[in] _config string to parse
       \param[in] f filter factory that is aware of all possible filters to choose from
       \param[in] s sink factory that is aware of all possible sinks to choose from

       \return 
       \retval 
       
    */
    template <typename filter_factory_t = stage_factory<blank_filter>,
	      typename sink_factory_t = stage_factory<blank_sink>
	      >
    static dynamic_pipeline load(const std::string &_config,
				 filter_factory_t f = stage_factory<blank_filter>(),
				 sink_factory_t s = stage_factory<blank_sink>())
    {

      std::vector<std::string> tags = sqeazy::split(_config, std::string("->"));

      dynamic_pipeline value;

      for(const std::string &word : tags)
      {
        if(sink_factory_t::has(word)){
	  auto temp = sink_factory_t::template create< sink_t >(word);
          value.sink_ = temp;
	}
	//what if sink is present in _config, but not in factory
      }

      for(const std::string &word : tags){
        if(filter_factory_t::has(word))
          value.add((filter_factory_t::template create<filter_t>(word)));
      	//what if filter is present in _config, but not in factory
      }
      
      return value;
    }

    template <typename filter_factory_t = stage_factory<blank_filter>,
	      typename sink_factory_t = stage_factory<blank_sink>
	      >
    static dynamic_pipeline load(const char *_config,
				 filter_factory_t f = stage_factory<blank_filter>(),
				 sink_factory_t s = stage_factory<blank_sink>())
    {
      std::string config = _config;

      return load(config, f, s);
    }

    friend void swap(dynamic_pipeline &_lhs, dynamic_pipeline &_rhs)
    {
      std::swap(_lhs.filters_, _rhs.filters_);
      std::swap(_lhs.sink_, _rhs.sink_);
    }

    /**
       \brief construct an empty pipeline


       \return
       \retval

    */
    dynamic_pipeline()
        : filters_()
        , sink_(nullptr){};

    /**
       //DEPRECATE THIS?//
       \brief construct an pipeline from given stages (mainly for debugging)
       
       \param[in] _size how many stages will the pipeline have

       \return
       \retval

    */
    dynamic_pipeline(std::initializer_list<filter_ptr_t> _stages)
        : filters_()
        , sink_(nullptr)
    {

      for(const filter_ptr_t& step : _stages)
      {
	filters_.push_back(step);
      }
    };

    /**
       \brief copy-constructor using copy&swap idiom

       \param[in]

       \return
       \retval

    */
    template <typename T>
    dynamic_pipeline(const T& _rhs):
      filters_(_rhs.filters_),
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
    const std::size_t size() const { return filters_.size() + (sink_ ? 1 : 0); }

    const bool empty() const { return (filters_.empty() && !sink_); }


    void add(const filter_ptr_t& _new_filter)
    {

      if(empty())
      {
        filters_.push_back(_new_filter);
        return;
      }

      auto view = const_stage_view(_new_filter);
      if(view->input_type() == typeid(incoming_t).name())
        filters_.push_back(_new_filter);
      else
        throw std::runtime_error("sqeazy::dynamic_pipeline::add_filter failed");
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
      if(filters_.empty())
        return !value;

      if(filters_.size() == 1)
        return value;

      
      for(unsigned i = 1; i < filters_.size(); ++i)
      {
	auto this_filter =  const_stage_view(filters_[i]);
	auto last_filter =  const_stage_view(filters_[i-1]);

	if(!this_filter || !last_filter){
	  value = false ;
	  break;
	}
        value = value && (this_filter->input_type() == last_filter->output_type());
      }

      return value;
    }

    std::string input_type() const
    {

      if(filters_.size())
        return const_stage_view(filters_.front())->input_type();

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
        if(filters_.size())
        {
          return const_stage_view(filters_.front())->output_type();
        }
        else
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

      std::string value = "";

      for(const auto& step : filters_)
      {
        value.append(const_stage_view(step)->name());

        if(step != filters_.back())
          value.append("->");
      }

      if(is_compressor())
      {
        value.append("->");
        value.append(const_stage_view(sink_)->name());
      }

      return value;
    };


    /**
       \brief encode one-dimensional array _in and write results to _out
       
       \param[in] 
       
       \return 
       \retval 
       
    */
    
    int encode(const raw_t *_in, compressed_t *_out, std::size_t _len) const override final {

      std::vector<std::size_t> shape(1,_len);
      return encode(_in,_out,shape);

    }

        /**
       \brief encode one-dimensional array _in and write results to _out
       
       \param[in] _in input buffer
       \param[out] _out output buffer must be of size max_encoded_size or larger // FIXME!!
       
       \return 
       \retval 
       
    */
    
    int encode(const raw_t *_in, compressed_t *_out, std::vector<std::size_t> _shape) const override final {

      int value = 0;
      std::size_t len = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());

      //prepare temp data
      std::vector<raw_t> temp_in(_in, _in+len);
      std::vector<raw_t> temp_out;
      raw_t* out = nullptr;
      if(is_compressor()){
	temp_out.resize(len);
	out = &temp_out[0];
      }
      else {
	out = reinterpret_cast<raw_t*>(_out);
      }

      //loop filters
      int err_code = 1;
      for( std::size_t fidx = 0;fidx<filters_.size();++fidx )
	{

	  err_code = filters_[fidx]->encode(&temp_in[0],
					     out,
					     _shape);
	  value += err_code ? (10*fidx)+err_code : 0;
	  std::copy(out, out+len,temp_in.begin());
	}

      
      if(is_compressor()){
	err_code = sink_->encode(&temp_in[0],
				 (typename sink_t::out_type*)_out,
				 _shape);
	value += err_code ? 10*err_code : 0;
      }
      else {
	std::copy(out, out+len,_out);
      }

      
      
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
       \param[in] _out output buffer //FIXME might be larger than _in for sink type pipelines
       
       \return 
       \retval 
       
    */
    
    int decode(const compressed_t *_in, raw_t *_out, std::vector<std::size_t> _shape) const override final {

      int value = 0;
      int err_code = 0;
      std::size_t len = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());

      const raw_t* begin = reinterpret_cast<const raw_t*>(_in);
      std::vector<raw_t> temp(begin, begin+len);
      
      if(is_compressor()){
	err_code = sink_->decode((const typename sink_t::out_type*)_in,
				 &temp[0],
				 _shape);
	value += err_code ;
      }       

      for( std::size_t fidx = 0;fidx<filters_.size();++fidx )
	{
	  
	  err_code = filters_[fidx]->decode(&temp[0],
					    _out,
					    _shape);
	  value += err_code ? (10*(fidx+1))+err_code : 0;
	  std::copy(_out, _out+len,temp.begin());
	}

      
      return value;

    }

    std::intmax_t max_encoded_size(std::intmax_t _incoming_size_byte){
      if(!is_compressor())
	return _incoming_size_byte;
      else
	return sink_->max_encoded_size(_incoming_size_byte);
    }
    
  };
}

#endif /* _DYNAMIC_PIPELINE_H_ */
