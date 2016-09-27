#ifndef _COMPASS_MSVC_IMPL_H_
#define _COMPASS_MSVC_IMPL_H_

#include <string>
#include "intrin.h"
#include "compass_tags.hpp"

namespace compass {

  namespace runtime {

    namespace detail {
      
      static bool works(compiletime::msvc_tag) {
		  std::int32_t regs[4] = { 0,0,0,0 };
		  __cpuid(&regs[0],0);

		  if (regs[0] != 0)
			  return true;
		  else
			  return false;
      
      }

	  static std::string vendor(compiletime::msvc_tag) {

		  std::int32_t regs[4] = { 0,0,0,0 };
		  __cpuid(&regs[0], 0);

		  std::string vendor_name;
		  vendor_name.resize(3 * 4);

		  std::copy(reinterpret_cast<char*>(&regs[1]), reinterpret_cast<char*>(&regs[1]) + 4,
			  vendor_name.begin());
		  std::copy(reinterpret_cast<char*>(&regs[2]), reinterpret_cast<char*>(&regs[2]) + 4,
			  vendor_name.begin() + 4);
		  std::copy(reinterpret_cast<char*>(&regs[3]), reinterpret_cast<char*>(&regs[3]) + 4,
			  vendor_name.begin() + 8);

		  if (regs[0] != 0)
			  return vendor_name;
		  else
			  return "";
		  
	  }

      static bool has(feature::sse , ct::msvc_tag){
	return false;
      }

	  static bool has(feature::sse2, ct::msvc_tag) {
		  return false;
	  }

      static bool has(feature::sse3 , ct::msvc_tag){
	return false;
      }

      static bool has(feature::sse4 , ct::msvc_tag){
	return false;
      }

      static bool has(feature::avx , ct::msvc_tag){
	return false;
      }


      static bool has(feature::avx2 , ct::msvc_tag){
	return false;
      }

      
    };
  };
  
  
};
#endif /* _COMPASS_MSVC_IMPL_H_ */
