#ifndef __DYNAMIC_STAGE_FACTORY_HPP__
#define __DYNAMIC_STAGE_FACTORY_HPP__

namespace sqeazy {

  /**
     \brief method to create stage for a type given by a string
     
     \param[in] _name string to match type by
     
     \return 
     \retval 
     
  */
  template <typename pointee_t, typename Head>
  std::shared_ptr<pointee_t> create_by_name(const std::string &_name,
					    const std::string &_payload = "")
  {
    if(Head().name() == _name)
    {
      return std::dynamic_pointer_cast<pointee_t>(std::make_shared<Head>(_payload));
    }
    else
    {
      return nullptr;
    }
  }

  template <typename pointee_t, typename Head, typename Second, typename... Tail>
  std::shared_ptr<pointee_t> create_by_name(const std::string &_name,
					    const std::string &_payload = "")
  {
    if(Head().name() == _name)
    {
      return std::dynamic_pointer_cast<pointee_t>(std::make_shared<Head>(_payload));
    }
    else
    {
      return create_by_name<pointee_t, Second, Tail...>(_name,_payload);
    }
  }

  /**
     \brief method to check if list of types contains type given by runtime string name
     
     \param[in] _name string to match type by
     
     \return 
     \retval 
     
  */
  template <class Head>
  bool type_matches(const std::string &_name)
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


  template <class Head, class Second, class... Tail>
  bool type_matches(const std::string &_name)
  {
    if(Head().name() == _name)
    {
      // got it
      return true;
    }
    else
    {
      return type_matches<Second, Tail...>(_name);
    }
  }

  
  /**
     \brief method to check if list of types contains type given by runtime string name
     
     \param[in] _name string to match type by
     
     \return 
     \retval 
     
  */
  template <class Head>
  void print_name(const std::string & _line_prefix="\n\t")
  {

    Head instance;
    std::cout << _line_prefix << instance.name() << "\n";
    return ;
    
  }


  template <class Head, class Second, class... Tail>
  void print_name(const std::string & _line_prefix="\n\t")
  {
    Head instance;
    std::cout << _line_prefix << instance.name() << "\n";
    return print_name<Second, Tail...>(_line_prefix);
  }
  
  /**
     \brief factory object that will create a stage defined by a runtime string
     
     \param[in] _name runtime string to define the stage
     
     \return 
     \retval 
     
  */
  template <typename... available_types>
  struct stage_factory
  {
    static_assert(sizeof...(available_types) > 0, "[dynamic_stage.hpp::stage_factory] Need at least one type for factory");

    
    
    template <typename pointee_t>
    static const std::shared_ptr<pointee_t> create(const std::string &_name,
					    const std::string &_payload = "")
    {
      // static_assert(sizeof...(available_types) > 0, "Need at least one type for factory");
      auto value = create_by_name<pointee_t,available_types...>(_name,_payload);
      
      return value;
    }

    static const bool has(const std::string &_name) { return type_matches<available_types...>(_name); }

    static const void print_names(const std::string & _line_prefix="\n\t") { return print_name<available_types...>(_line_prefix); }
  };








};


#endif 
