#ifndef _PIPELINE_HPP_
#define _PIPELINEr_HPP_
#include <string>
#include <sstream>
#include "boost/mpl/vector.hpp"
#include "boost/mpl/for_each.hpp"
#include "boost/mpl/front.hpp"
#include "boost/mpl/back.hpp"

namespace sqeazy {

  namespace bmpl = boost::mpl;
  struct get_name {
    
    std::string* text;
    
    template <typename T>
    void operator()(T any) {

        if(text)
            *text += T::name();

    }
    
  };
  
template <typename TransferT, typename TypeList>
struct pipeline : public bmpl::front<TypeList>::type, 
		 public bmpl::back<TypeList>::type {
  
  
  
  static std::string name() {
    
    static std::string temp;
    get_name extractor;
    extractor.text = &temp;
    bmpl::for_each<TypeList>(extractor);
    
    return temp;
    
  }
  
  
  
};
}//sqeazy

#endif /* _PIPELINEr_HPP_ */
