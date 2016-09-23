#ifndef _COMPASS_MSVC_IMPL_H_
#define _COMPASS_MSVC_IMPL_H_

#include <string>

#include "compass_tags.hpp"

namespace compass {

  namespace runtime {

    namespace detail {
      
      static bool works(compiletime::msvc_tag) {

	return false;//not implemented yet
      
      }

      static std::string vendor(compiletime::msvc_tag) {

	return "";//not implemented yet
      
      }

      static bool has(feature::sse , ct::msvc_tag){
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
