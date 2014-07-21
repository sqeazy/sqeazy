#ifndef _BITSWAP_SCHEME_UTILS_H_
#define _BITSWAP_SCHEME_UTILS_H_
#include <limits>

namespace sqeazy {

  template <bool Assert, typename T>
  struct enable_if {

    typedef T type;

  };

  template <typename T>
  struct enable_if<false,T> {

  };
  
  
 
  template <typename T>
  static typename enable_if<std::numeric_limits<T>::is_signed,T>::type shift_signed_bit_if_present(const T& _in){
    return 0;
  }

  template <typename T>
  static typename enable_if<!std::numeric_limits<T>::is_signed,T>::type shift_signed_bit_if_present(const T& _in){
          //unsigned version
      return _in;
  }
  

  template < typename T>
  T setbits_of_integertype(const T& destination, const T& source, unsigned at, unsigned numbits)
  {
    T ones = ((1<<(numbits))-1)<<at;
    return (ones|destination)^((~source<<at)&ones);
  }


} //sqeazy
#endif /* _BITSWAP_SCHEME_UTILS_H_ */
