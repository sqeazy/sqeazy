#ifndef _SQY_COMMON_HPP_
#define _SQY_COMMON_HPP_

#include <iostream>

namespace sqeazy {
  
  enum error_code {

    SUCCESS = 0,
    FAILURE = 1,
    NOT_IMPLEMENTED_YET = 42

  };

  struct unknown {};

  /**
     \brief this namespace is meant for helpers related to the platform sqeazy was compiled on
          
  */
  namespace platform {
  
#if defined __SSE2__ && defined __SSE2_MATH__
    struct has_sse2   {const static bool value = __SSE2__ && __SSE2_MATH__;};
#else
    struct has_sse2   {const static bool value = false;};
#endif

#if defined __SSE3__ && defined __SSSE3__
    struct has_sse3   {const static bool value = __SSE3__ && __SSSE3__;};
#else
    struct has_sse3   {const static bool value = false;};
#endif

#if defined __SSE4_2__ && defined __SSE4_1__
    struct has_sse4   {const static bool value = __SSE4_2__ && __SSE4_1__;};
#else
    struct has_sse4   {const static bool value = false;};
#endif

    struct use_vectorisation {
      const static bool value = (has_sse2::value && has_sse3::value && has_sse4::value) && _SQY_X_;

      static void debug() {

	std::cout << "sse2 ? " << has_sse2::value << "\n"
		  << "sse3 ? " << has_sse3::value << "\n"
		  << "sse4 ? " << has_sse4::value << "\n"
	  	  << "sqyx ? " << _SQY_X_ << "\n";
      }
    };
    
  };

};//sqeazy

#endif /* _SQY_COMMON_HPP_ */
