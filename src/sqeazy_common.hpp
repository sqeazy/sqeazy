#ifndef _SQY_COMMON_HPP_
#define _SQY_COMMON_HPP_

namespace sqeazy {
  
enum error_code {

    SUCCESS = 0,
    FAILURE = 1,
    NOT_IMPLEMENTED_YET = 42

};

  struct compressor_tag {};
  struct unknown_tag {};

};//sqeazy

#endif /* _SQY_COMMON_HPP_ */
