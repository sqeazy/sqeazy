#ifndef _BITSWAP_SCHEME_UTILS_H_
#define _BITSWAP_SCHEME_UTILS_H_
#include <limits>
#include <climits>
#include <iostream>
#include "sqeazy_traits.hpp"

namespace sqeazy {

  template <bool Assert, typename T>
  struct enable_if {

    typedef T type;

  };

  template <typename T>
  struct enable_if<false,T> {

  };

  template <unsigned shift,typename T >
  T rotate_left(const T& _in)  {
    typedef typename sqeazy::add_unsigned<T>::type type;
    static const unsigned num_bits = (sizeof(type) * CHAR_BIT) - shift;
    // std::cout << "rotate_left\t" << sizeof(type) << "*" << CHAR_BIT << " - " << shift << " = "<< num_bits << "\n";
    type shifted = _in << shift;
    static const type mask = ~(~0 << shift);
    type left_over = (_in >> num_bits) & mask;

    return  (shifted | left_over);
  }

  template <unsigned shift,typename T>
  T rotate_right(const T& _in)  {
    typedef typename sqeazy::add_unsigned<T>::type type;
    static const unsigned num_bits = (sizeof(type) * CHAR_BIT) - shift;
    // std::cout << "rotate_right\t" << sizeof(type) << "*" << CHAR_BIT << " - " << shift << " = "<< num_bits << "\n";
    static const type mask = ~(~0 << num_bits);
    type shifted = (_in >> shift) & mask;
    type right_over = _in << num_bits;

    return  (shifted | right_over);
  }
 
  template <typename T>
  static typename enable_if<std::numeric_limits<T>::is_signed,T>::type xor_if_signed(const T& _in){
    //signed version
    static T mask = ~(1 << (sizeof(T)*8 - 1));
    return _in ^ mask;
  }

  template <typename T>
  static typename enable_if<!std::numeric_limits<T>::is_signed,T>::type xor_if_signed(const T& _in){
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
