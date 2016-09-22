#ifndef _COMPASS_HPP_
#define _COMPASS_HPP_

#include <type_traits>
#include <cstdint>
#include <string>
#include "compass_detail.hpp"

namespace compass {

  namespace compiletime {
    
    static const bool is_gnu(){

      using current_platform_t = platform::type;
      
      bool value = std::is_same<current_platform_t,gnu_tag>::value;
      return value;
      
    }

    static const bool is_llvm(){

      using current_platform_t = platform::type;
      bool value = std::is_same<current_platform_t,llvm_tag>::value;
      return value;

    }

    static const bool is_msvc(){

      using current_platform_t = platform::type;
      bool value = std::is_same<current_platform_t,msvc_tag>::value;
      return value;

    }

  };

  
  
  namespace runtime {


    static bool works() {

      using current_platform_t = ct::platform::type;
      return detail::works(current_platform_t());
      
    }

    static std::string vendor() {

      using current_platform_t = ct::platform::type;
      return detail::vendor(current_platform_t());
      
    }

    
  };


};

#endif /* _COMPASS_H_ */
