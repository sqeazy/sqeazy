#ifndef _DYNAMIC_PIPELINE_H_
#define _DYNAMIC_PIPELINE_H_
#include <utility>
#include <iostream>
#include <vector>
#include <string>
#include <initializer_list>
#include <memory>
#include <cstdint>
#include <typeinfo>
#include <stdexcept>

#include "boost/blank.hpp"
#include "sqeazy_utils.hpp"

namespace sqeazy
{
  
  
  struct stage
  {

    virtual ~stage() {}

    
    virtual std::string input_type() const = 0;
    virtual std::string output_type() const = 0;
    virtual std::string name() const = 0;

    virtual bool is_compressor() const = 0;
  };

    template <class Head> std::shared_ptr<stage> extract(const std::string &_name)
  {
    if(Head().name() == _name)
    {
      //got it
      return std::make_shared<Head>();
    }
    else
    {
      std::ostringstream msg;
      msg << _name << " not available as pipeline step\n";
      throw std::runtime_error(msg.str());
    }
  }

  template <class Head, class Second, class... Tail> std::shared_ptr<stage> extract(const std::string &_name)
  {
    if(Head().name() == _name)
    {
      //got it
      return std::make_shared<Head>();
    }
    else
    {
      return extract<Second, Tail...>(_name);
    }
  }


  template <class... available_types> struct stage_factory
  {

    static std::shared_ptr<stage> create(const std::string &_name) 
    {
      static_assert(sizeof...(available_types) > 0, "Need at least one type for factory");

      return extract<available_types...>(_name);
    }
  };

  

  struct dynamic_pipeline : public stage
  {

    typedef std::shared_ptr<stage> ptr_t;
    typedef std::shared_ptr<stage> sink_t;
    typedef std::vector<ptr_t> filter_holder_t;

    filter_holder_t	filters_;
    sink_t		sink_;


    
    static dynamic_pipeline load(const std::string& _config){

      std::vector<std::string> tags = sqeazy::split(_config,std::string("->"));

      dynamic_pipeline value;
      
      return value;
      
    }
    
    friend void swap(dynamic_pipeline &_lhs, dynamic_pipeline &_rhs) {
      std::swap(_lhs.filters_, _rhs.filters_);
      std::swap(_lhs.sink_, _rhs.sink_);
    }

    /**
       \brief construct an empty pipeline


       \return
       \retval

    */
    dynamic_pipeline()
      : filters_(),
	sink_(nullptr)
    {};

    /**
       \brief construct an pipeline from given stages

       \param[in] _size how many stages will the pipeline have

       \return
       \retval

    */
    dynamic_pipeline(std::initializer_list<ptr_t> _stages)
      : filters_(), sink_(nullptr) {

      for(ptr_t step : _stages){

	if(!step->is_compressor())
	  filters_.push_back(step);
	else{
	  if(sink_==nullptr)
	    sink_ = step;
	  else
	    std::cerr << "sqeazy::dynamic_pipeline:"<< __LINE__ << ":\t"
		      <<" skipping second sink in stages " << step->name()
		      << "(known sink: " << sink_->name() << ")\n";

	}
      }


    };

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
    const std::size_t size() const { return filters_.size()+ (sink_ ? 1 : 0); }

    const bool empty() const { return filters_.empty() && !sink_; }

    // filter_holder_t::iterator begin() { return filters_.begin(); }

    // filter_holder_t::const_iterator begin() const { return filters_.begin(); }


    // filter_holder_t::iterator end() { return filters_.end(); }

    // filter_holder_t::const_iterator end() const { return filters_.end(); }

    void add_filter(ptr_t _new_item)
    {

      if(empty())
      {
        filters_.push_back(_new_item);
        return;
      }

      if(_new_item->input_type() == filters_.back()->output_type())
        filters_.push_back(_new_item);
      else
        throw std::runtime_error("sqeazy::dynamic_pipeline::add_filter failed");
    }

    bool valid_filters() const
    {

      bool value = true;
      if(empty())
        return !value;

      if(filters_.size() == 1)
        return value;


      for(unsigned i = 1; i < filters_.size(); ++i)
      {
        value = value && (filters_[i]->input_type() == filters_[i - 1]->output_type());
      }

      return value;
    }

    std::string input_type() const
    {

      if(filters_.size())
        return filters_.front()->input_type();

      if(sink_)
        return sink_->input_type();
      
      return std::string("");
    };

    std::string output_type() const
    {

      if(sink_){
	return sink_->input_type();
      } else{
	if(filters_.size()){
	  return filters_.back()->input_type();
	}
	else
	  return std::string("");
      }
    };

    bool is_compressor() const {

      return sink_ != nullptr;
      
    }
    
    std::string name() const
    {

      std::string value = "";

      for(auto & step : filters_)
      {
        value.append(step->name());

        if(step != filters_.back())
          value.append("->");
      }

      if(is_compressor()){
	value.append("->");
	value.append(sink_->name());
      }
	 
      return value;
    };
  };
}

#endif /* _DYNAMIC_PIPELINE_H_ */
