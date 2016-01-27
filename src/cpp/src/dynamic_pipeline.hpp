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
#include "dynamic_stage.hpp"

namespace sqeazy
{

  struct dynamic_pipeline : public stage
  {

    typedef std::shared_ptr<stage> ptr_t;
    typedef std::shared_ptr<stage> sink_t;
    typedef std::vector<ptr_t> filter_holder_t;

    filter_holder_t filters_;
    sink_t sink_;


    template <typename filter_factory_t, typename sink_factory_t>
    static dynamic_pipeline load(const std::string &_config,
				 filter_factory_t f = stage_factory<blank_stage>(),
				 sink_factory_t s = stage_factory<blank_stage>())
    {

      std::vector<std::string> tags = sqeazy::split(_config, std::string("->"));

      dynamic_pipeline value;

      for(std::string &word : tags)
      {
        if(sink_factory_t::has(word))
          value.sink_ = sink_factory_t::create(word);
      }

      for(std::string &word : tags)
        if(filter_factory_t::has(word))
          value.add(filter_factory_t::create(word));

      return value;
    }

    template <typename filter_factory_t = stage_factory<blank_stage>,
	      typename sink_factory_t = stage_factory<blank_stage>
	      >
    static dynamic_pipeline load(const char *_config,
				 filter_factory_t f = stage_factory<blank_stage>(),
				 sink_factory_t s = stage_factory<blank_stage>())
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
       \brief construct an pipeline from given stages

       \param[in] _size how many stages will the pipeline have

       \return
       \retval

    */
    dynamic_pipeline(std::initializer_list<ptr_t> _stages)
        : filters_()
        , sink_(nullptr)
    {

      for(ptr_t step : _stages)
      {

        if(!step->is_compressor())
          filters_.push_back(step);
        else
        {
          if(sink_ == nullptr)
            sink_ = step;
          else
            std::cerr << "sqeazy::dynamic_pipeline:" << __LINE__ << ":\t"
                      << " skipping second sink in stages " << step->name() << "(known sink: " << sink_->name() << ")\n";
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
    const std::size_t size() const { return filters_.size() + (sink_ ? 1 : 0); }

    const bool empty() const { return filters_.empty() && !sink_; }


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

      if(sink_)
      {
        return sink_->input_type();
      }
      else
      {
        if(filters_.size())
        {
          return filters_.back()->input_type();
        }
        else
          return std::string("");
      }
    };

    bool is_compressor() const { return sink_ != nullptr; }

    /**
       \brief produce string of pipeline, separated by ->

       \param[in]

       \return
       \retval

    */
    std::string name() const
    {

      std::string value = "";

      for(auto &step : filters_)
      {
        value.append(step->name());

        if(step != filters_.back())
          value.append("->");
      }

      if(is_compressor())
      {
        value.append("->");
        value.append(sink_->name());
      }

      return value;
    };

    /**
       \brief add new stage to pipeline (if stage is a sink, the sink will be replaced)

       \param[in] _new_item new stage to add (filter or sink)

       \return
       \retval

    */
    void add(ptr_t _new_item)
    {

      if(_new_item->is_compressor())
      {
        sink_ = _new_item;
        return;
      }

      filters_.push_back(_new_item);
    }
    
    /**
       \brief encode one-dimensional array _in and write results to _out
       
       \param[in] 
       
       \return 
       \retval 
       
    */
    template <typename raw_type, typename compressed_type>
    int encode(const raw_type *_in, compressed_type *_out, std::size_t _len) {

      
      return 0;

    }
  };
}

#endif /* _DYNAMIC_PIPELINE_H_ */
