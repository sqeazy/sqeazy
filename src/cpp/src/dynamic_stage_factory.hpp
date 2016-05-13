#ifndef __DYNAMIC_STAGE_FACTORY_HPP__
#define __DYNAMIC_STAGE_FACTORY_HPP__

#include <iostream>
#include <string>
#include <memory>
#include <vector>


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
  void fill_max_encoded_size(std::intmax_t _size_byte, std::vector<std::intmax_t>& _output)
  {

    Head instance;
    _output.push_back(instance.max_encoded_size(_size_byte));
    return ;
    
  }


  template <class Head, class Second, class... Tail>
  void fill_max_encoded_size(std::intmax_t _size_byte, std::vector<std::intmax_t>& _output)
  {
    Head instance;
    _output.push_back(instance.max_encoded_size(_size_byte));
    return fill_max_encoded_size<Second, Tail...>(_size_byte,_output);
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
     \brief method to add template parameter name to vector
     
     \return string vector with the return values of T::name of each member of the template parameter pack
     \retval 
     
  */
  template <class Head>
  void add_name(std::vector<std::string>& _names)
  {
    Head instance;
    _names.push_back(instance.name());
    return ;
    
  }


  template <class Head, class Second, class... Tail>
  void add_name(std::vector<std::string>& _names)
  {

    Head instance;
    _names.push_back(instance.name());
    return add_name<Second, Tail...>(_names);
  }

  /**
     \brief method to add template parameter name and description to inbound map
     
     \return string vector with the return values of T::name and the return values of T::description of each member of the template parameter pack
     \retval 
     
  */
  template <class Head>
  void add_name_and_description(std::vector<std::string>& _vector)
  {
   
    _vector.push_back(Head::description());
    return ;
    
  }


  template <class Head, class Second, class... Tail>
  void add_name_and_description(std::vector<std::string>& _vector)
  {

    _vector.push_back(Head::description());
    return add_name_and_description<Second, Tail...>(_vector);
  }
  
  /**
     \brief method to count the number of templates in the template parameter pack
     
     \return number of template parameters
     \retval 
     
  */
  template <class Head>
  size_t count_templates(size_t& _count)
  {

    return _count+1;
    
  }


  template <class Head, class Second, class... Tail>
  size_t count_templates(size_t& _count)
  {
    _count+=1;
    return count_templates<Second, Tail...>(_count);
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

    static const size_t size() {

      size_t value = 0;
      return count_templates<available_types...>(value);

    }

    static const std::intmax_t max_encoded_size(std::intmax_t _size_bytes) {

      std::vector<std::intmax_t> values;
      fill_max_encoded_size<available_types...>(_size_bytes, values);
      
      return *std::max_element(values.begin(), values.end());

    }
    
    static const std::vector<std::string> name_list() {

      std::vector<std::string> value;
      value.reserve(size());
      add_name<available_types...>(value);
      return value;
    }

    static const std::vector<std::string> descriptions() {

      std::vector<std::string> value;
      value.reserve(size());
      add_name_and_description<available_types...>(value);
      return value;
    }
    
  };








};


#endif 
