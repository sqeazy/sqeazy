#ifndef _COMPASS_DETAIL_H_
#define _COMPASS_DETAIL_H_

#include "cpuid.h"

namespace compass {
  namespace compiletime {

    struct gnu_tag  {};
    struct llvm_tag {};
    struct msvc_tag {};

    struct platform {

#ifdef __GNUC__
      typedef gnu_tag type;
#endif

#ifdef __clang__
      typedef llvm_tag type;
#endif
      
#ifdef __MSC_BUILD__
      typedef msvc_tag type;
#endif
      
    };
  };

  namespace ct = compiletime;
  
  namespace runtime {

    namespace detail {
      static bool works(compiletime::gnu_tag) {

	std::uint32_t regs[4] = {0,0,0,0};
	int cpuid_rvalue = __get_cpuid(0,&regs[0],&regs[1],&regs[2],&regs[3]);
      
	if(cpuid_rvalue > 0)
	  return true;
	else
	  return false;
      
      }
    
      static bool works(compiletime::llvm_tag) {

	return false;//not implemented yet
      
      }

      static bool works(compiletime::msvc_tag) {

	return false;//not implemented yet
      
      }

    };
  };
  
  namespace rt = runtime;
  
};
#endif /* _COMPASS_DETAIL_H_ */
