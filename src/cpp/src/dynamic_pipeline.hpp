#ifndef _DYNAMIC_PIPELINE_H_
#define _DYNAMIC_PIPELINE_H_
#include <utility>
#include <vector>
#include <string>
#include <initializer_list>
#include <memory>
#include <cstdint>
#include <typeinfo>
#include <stdexcept>

#include "boost/blank.hpp"

namespace sqeazy
{

  struct stage
  {

    virtual ~stage() {}

    virtual std::string input_type() const = 0;
    virtual std::string output_type() const = 0;
    virtual std::string name() const = 0;
  };


  struct dynamic_pipeline : public stage
  {

    typedef std::shared_ptr<stage> ptr_t;
    typedef std::shared_ptr<stage> sink_t;
    typedef std::vector<ptr_t> holder_t;

    holder_t filters_;
    sink_t	sink_;

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
	sink_()
    {};

    /**
       \brief construct an pipeline from given stages

       \param[in] _size how many stages will the pipeline have

       \return
       \retval

    */
    dynamic_pipeline(std::initializer_list<ptr_t> _stages)
      : filters_(_stages), sink_() {};

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
    const std::size_t size() const { return filters_.size(); }

    const bool empty() const { return filters_.empty(); }

    holder_t::iterator begin() { return filters_.begin(); }

    holder_t::const_iterator begin() const { return filters_.begin(); }


    holder_t::iterator end() { return filters_.end(); }

    holder_t::const_iterator end() const { return filters_.end(); }

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

    bool valid_types() const
    {

      bool value = true;
      if(empty())
        return !value;

      if(size() == 1)
        return value;


      for(unsigned i = 1; i < size(); ++i)
      {
        value = value && (filters_[i]->input_type() == filters_[i - 1]->output_type());
      }

      return value;
    }

    std::string input_type() const
    {

      if(size())
        return filters_.front()->input_type();
      else
        return std::string("");
    };

    std::string output_type() const
    {

      if(size())
        return filters_.back()->input_type();
      else
        return std::string("");
    };

    std::string name() const
    {

      std::string value = "";
      // for(std::size_t i = 0;i<size();++i){
      for(auto step : filters_)
      {
        value.append(step->name());

        if(step != (filters_.back()))
          value.append("->");
      }

      return value;
    };
  };
}

#endif /* _DYNAMIC_PIPELINE_H_ */
