#ifndef _SQY_COMMON_HPP_
#define _SQY_COMMON_HPP_

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
    
    struct no_vectorisation {};
    struct has_sse2 {};
    struct has_sse4_1 {};
    struct has_sse4_2 {};

    
    
  };

};//sqeazy

#endif /* _SQY_COMMON_HPP_ */
