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

  /*
    rotate bits of integer by <shift> to the left (wrapping around the ends)
    input = 0001 0111
    rotate_left<1>(input): 0010 1110
   */
  template <unsigned shift,typename T >
  T rotate_left(const T& _in)  {
    typedef typename sqeazy::add_unsigned<T>::type type;
    static const unsigned num_bits = (sizeof(type) * CHAR_BIT) - shift;
    
    type shifted = _in << shift;
    static const type mask = ~(~0 << shift);
    type left_over = (_in >> num_bits) & mask;

    return  (shifted | left_over);
  }

  /*
    rotate bits of integer by <shift> to the left (wrapping around the ends)
    input = 0001 0111
    rotate_left<1>(input): 1000 1011
   */
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
    static const T mask = ~(T(1 << ((sizeof(T)*CHAR_BIT) - 1)));
    if ( _in & ~mask )
        return _in ^ mask;
    else
        return _in;
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
