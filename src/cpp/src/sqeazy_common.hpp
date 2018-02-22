#ifndef _SQY_COMMON_HPP_
#define _SQY_COMMON_HPP_

#include <iostream>
#include <utility>
#include <string>
#include <vector>

#include "compass.hpp"

#include "boost/align/aligned_allocator.hpp"


#ifdef _OPENMP
#include "omp.h"
typedef typename std::make_signed<std::size_t>::type omp_size_type;//boiler plate required for MS VS 14 2015 OpenMP implementation
#else
typedef std::size_t omp_size_type;//boiler plate required for MS VS 14 2015 OpenMP implementation
#endif


namespace sqeazy {

  //return codes, TODO: are enum classes useful here?
  enum error_code {

    SUCCESS = 0,
    FAILURE = 1,
    NOT_IMPLEMENTED_YET = 42

  };

  //tag class
  struct unknown {};

  //aligned vector classes
  //AVX
  template <typename value_type>
  using vec_32algn_t = std::vector<value_type, boost::alignment::aligned_allocator<value_type,32> >;

  //SSE
  template <typename value_type>
  using vec_16algn_t = std::vector<value_type, boost::alignment::aligned_allocator<value_type,16> >;

  /**
     \brief this namespace is meant for helpers related to the platform sqeazy was compiled on

  */
  namespace platform {

    namespace ct = compass::compiletime;
    namespace ft = compass::feature;

    struct use_vectorisation {
      #ifdef _MSC_FULL_VER
      //MSVC doesn't honor hardware specific preprocessor flags, we are happy if anyone of them is true
      const static bool value = (ct::has<ft::sse2>::enabled || ct::has<ft::sse3>::enabled || ct::has<ft::sse4>::enabled);
      #else
      const static bool value = (ct::has<ft::sse2>::enabled && ct::has<ft::sse3>::enabled && ct::has<ft::sse4>::enabled);
      #endif

      static void debug() {

        std::string slb = "\n";
        #ifndef _MSB_FULL_VER
        slb = "(not used in MSVC)\n";
        #endif
        std::cout << "sse2 ? " << ct::has<ft::sse2>::enabled << "\n"
                  << "sse3 ? " << ct::has<ft::sse3>::enabled << slb
                  << "sse4 ? " << ct::has<ft::sse4>::enabled << slb
                  << "sqyx ? " << _SQY_X_ << "\n";
      }
    };


  };

  static const std::pair<std::string, std::string> ignore_this_delimiters = std::make_pair("<verbatim>","</verbatim>");


};//sqeazy

#endif /* _SQY_COMMON_HPP_ */
