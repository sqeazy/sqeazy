#ifndef _SQY_COMMON_HPP_
#define _SQY_COMMON_HPP_

#include <iostream>
#include <utility>
#include <string>

#include "compass.hpp"

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
  
    namespace ct = compass::compiletime;
    namespace ft = compass::feature;
    
    // struct use_vectorisation {
    //   const static bool value = (ct::has<ft::sse2>() && ct::has<ft::sse3>() && ct::has<ft::sse4>()) && _SQY_X_;

    //   static void debug() {

    // 	std::cout << "sse2 ? " << ct::has<ft::sse2>() << "\n"
    // 		  << "sse3 ? " << ct::has<ft::sse3>() << "\n"
    // 		  << "sse4 ? " << ct::has<ft::sse4>() << "\n"
    // 	  	  << "sqyx ? " << _SQY_X_ << "\n";
    //   }
    // };

    struct use_vectorisation {
      const static bool value = (ct::has<ft::sse2>::enabled && ct::has<ft::sse3>::enabled && ct::has<ft::sse4>::enabled) && _SQY_X_;

      static void debug() {

    	std::cout << "sse2 ? " << ct::has<ft::sse2>::enabled << "\n"
    		  << "sse3 ? " << ct::has<ft::sse3>::enabled << "\n"
    		  << "sse4 ? " << ct::has<ft::sse4>::enabled << "\n"
    	  	  << "sqyx ? " << _SQY_X_ << "\n";
      }
    };


  };

  static const std::pair<std::string, std::string> ignore_this_delimiters = std::make_pair("<verbatim>","</verbatim>");



};//sqeazy

#endif /* _SQY_COMMON_HPP_ */
