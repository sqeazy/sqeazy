#ifndef _DYNAMIC_STAGE_H_
#define _DYNAMIC_STAGE_H_

#include <memory>

namespace sqeazy{



  struct stage
  {

    virtual ~stage() {}


    virtual std::string input_type() const = 0;
    virtual std::string output_type() const = 0;
    virtual std::string name() const = 0;

    virtual bool is_compressor() const = 0;
  };

  struct blank_stage : public stage
  {

    ~blank_stage() {}

    std::string input_type() const {return "";}
    std::string output_type() const {return "";}
    std::string name() const { return "blank";}

    bool is_compressor() const {return false;}
  };
  
  

  /**
     \brief method to create stage for a type given by a string
     
     \param[in] _name string to match type by
     
     \return 
     \retval 
     
  */
  template <class Head>
  std::shared_ptr<stage> create_by_name(const std::string &_name)
  {
    if(Head().name() == _name)
    {
      return std::make_shared<Head>();
    }
    else
    {
      return nullptr;
    }
  }

  template <class Head, class Second, class... Tail>
  std::shared_ptr<stage> create_by_name(const std::string &_name)
  {
    if(Head().name() == _name)
    {
      return std::make_shared<Head>();
    }
    else
    {
      return create_by_name<Second, Tail...>(_name);
    }
  }

  /**
     \brief method to check if list of types contains type given by runtime string name
     
     \param[in] _name string to match type by
     
     \return 
     \retval 
     
  */
  template <class Head> bool contains(const std::string &_name)
  {
    if(Head().name() == _name)
    {
      return true;
    }
    else
    {
      return false;
    }
  }


  template <class Head, class Second, class... Tail> bool contains(const std::string &_name)
  {
    if(Head().name() == _name)
    {
      // got it
      return true;
    }
    else
    {
      return contains<Second, Tail...>(_name);
    }
  }

  /**
     \brief factory object that will create a stage defined by a runtime string
     
     \param[in] _name runtime string to define the stage
     
     \return 
     \retval 
     
  */
  template <class... available_types>
  struct stage_factory
  {
    // static_assert(sizeof...(available_types) > 0, "Need at least one type for factory");

    static std::shared_ptr<stage> create(const std::string &_name)
    {
      // static_assert(sizeof...(available_types) > 0, "Need at least one type for factory");

      return create_by_name<available_types...>(_name);
    }

    static bool has(const std::string &_name) { return contains<available_types...>(_name); }
  };



};

#endif /* _DYNAMIC_STAGE_H_ */
