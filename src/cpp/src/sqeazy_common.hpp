#ifndef _SQY_COMMON_HPP_
#define _SQY_COMMON_HPP_

#include <iostream>
#include <utility>
#include <string>

#include "compass.hpp"


#ifdef _OPENMP
#include "omp.h"
typedef typename std::make_signed<std::size_t>::type omp_size_type;//boiler plate required for MS VS 14 2015 OpenMP implementation
#else
typedef std::size_t omp_size_type;//boiler plate required for MS VS 14 2015 OpenMP implementation
#endif


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

    struct use_vectorisation {
      const static bool value = (ct::has<ft::sse2>::enabled && ct::has<ft::sse3>::enabled && ct::has<ft::sse4>::enabled);

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
