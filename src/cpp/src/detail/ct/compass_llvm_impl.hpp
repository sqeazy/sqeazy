#ifndef _COMPASS_LLVM_IMPL_H_
#define _COMPASS_LLVM_IMPL_H_

#include <string>

#include "compass_tags.hpp:

namespace compass {

  namespace runtime {

    namespace detail {
      
      static bool works(compiletime::llvm_tag) {

	return false;//not implemented yet
      
      }

      static std::string vendor(compiletime::llvm_tag) {

	return "";//not implemented yet
      
      }

      static bool has(feature::sse , ct::llvm_tag){
	return false;
      }

      static bool has(feature::sse3 , ct::llvm_tag){
	return false;
      }

      static bool has(feature::sse4 , ct::llvm_tag){
	return false;
      }

      static bool has(feature::avx , ct::llvm_tag){
	return false;
      }


      static bool has(feature::avx2 , ct::llvm_tag){
	return false;
      }

      
    };
  };
  
  
};
#endif /* _COMPASS_LLVM_IMPL_H_ */
