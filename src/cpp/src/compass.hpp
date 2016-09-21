#ifndef _COMPASS_HPP_
#define _COMPASS_HPP_

#include <cstdint>
#include "cpuid.h"

namespace compass {

  namespace compiletime {

    struct gnu_tag  {};
    struct llvm_tag {};
    struct msvc_tag {};
    
    static constexpr bool is_gnu(){

#ifdef __GNUC__
      return true;
#else
      return false;
#endif
      
    }

    static constexpr bool is_llvm(){

#ifdef __clang__
      return true;
#else
      return false;
#endif
      
    }

    static constexpr bool is_msvc(){

#ifdef _MSC_BUILD
      return true;
#else
      return false;
#endif
      
    }

  };

  namespace ct = compiletime;
  
  namespace runtime {


    static bool detail_works(compiletime::gnu_tag) {

      std::uint32_t regs[4] = {0,0,0,0};
      int cpuid_rvalue = __get_cpuid(0,&regs[0],&regs[1],&regs[2],&regs[3]);
      
      if(cpuid_rvalue > 0)
	return true;
      else
	return false;
      
    }
    
    
    static bool works() {

      //TODO: infer platform at compile-time here
      #ifdef __GNUC__
      return detail_works(ct::gnu_tag());
      #else
      return false;
      #endif
      
    }
    
  };

  namespace rt = compiletime;

};

#endif /* _COMPASS_H_ */
