#ifndef _SCALAR_UTILS_H_
#define _SCALAR_UTILS_H_

#include "traits.hpp"

namespace sqeazy {

  namespace detail {
    /*
      rotate bits of integer by <shift> to the left (wrapping around the ends)
      input = 0001 0111
      rotate_left<1>(input): 0010 1110
      rotate_left<2>(input): 0101 1100
  
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
      rotate_right<1>(input): 1000 1011
      rotate_right<2>(input): 1100 0101
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

    /**
       \brief xor the given type with a 01...10 bitmask, for a 8-bit number 0x0b perform:
       -42 ^ 127 = -87
       0xd6 ^ 0x7f = 0xa9
       11010110 ^ 01111111 = 10101001

       \param[in] _in number to XOR

       \return 
       \retval 

    */
    template <typename T>
    static typename boost::enable_if_c<std::numeric_limits<T>::is_signed,T>::type xor_if_signed(const T& _in){
      //signed version
      static const T mask = ~(T(1 << ((sizeof(T)*CHAR_BIT) - 1)));
      if ( _in & ~mask )
	return _in ^ mask;
      else
	return _in;
    }

    template <typename T>
    static typename boost::enable_if_c<!std::numeric_limits<T>::is_signed,T>::type xor_if_signed(const T& _in){
      //unsigned version
      return _in;
    }
  

    template < typename T>
    T setbits_of_integertype(const T& destination, const T& source, unsigned at, unsigned numbits)
    {
      T ones = ((1<<(numbits))-1)<<at;
      return (ones|destination)^((~source<<at)&ones);
    }


  }

}

#endif /* _SCALAR_UTILS_H_ */
