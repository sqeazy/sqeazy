#ifndef _SQEAZY_TRAITS_H_
#define _SQEAZY_TRAITS_H_

#include <numeric>
#include "boost/utility/enable_if.hpp"
#include "boost/type_traits.hpp"

namespace sqeazy {

  template <typename T> struct remove_unsigned { typedef T type; };
  template <> struct remove_unsigned<unsigned char> 		{ typedef char		type; };
  template <> struct remove_unsigned<unsigned short> 		{ typedef short		type; };
  template <> struct remove_unsigned<unsigned int  > 		{ typedef int		type; };
  template <> struct remove_unsigned<unsigned long > 		{ typedef long		type; };
  template <> struct remove_unsigned<unsigned long long >	{ typedef long long	type; };

  template <typename T> struct add_unsigned { typedef T type; };
  template <> struct add_unsigned<char> 	{ typedef unsigned char	type; };
  template <> struct add_unsigned<short> 	{ typedef unsigned short	type; };
  template <> struct add_unsigned<int  > 	{ typedef unsigned int		type; };
  template <> struct add_unsigned<long > 	{ typedef unsigned long		type; };
  template <> struct add_unsigned<long long >	{ typedef unsigned long long	type; };

  template <typename T> struct twice_as_wide	{typedef T		type;};
  template	<> struct twice_as_wide<unsigned char> { typedef unsigned short		type; 	};
  template	<> struct twice_as_wide<unsigned short>{ typedef unsigned int		type; 	};
  template	<> struct twice_as_wide<unsigned int>  { typedef unsigned long		type;	};
  template	<> struct twice_as_wide<unsigned long> { typedef unsigned long long	type;	};
  template	<> struct twice_as_wide<char>	       { typedef short		type; };
  template	<> struct twice_as_wide<short>	       { typedef int		type; };
  template	<> struct twice_as_wide<int>	       { typedef long		type; };
  template	<> struct twice_as_wide<long>	       { typedef long long	type; };
  template	<> struct twice_as_wide<float>	       { typedef double		type; };

  
  
  struct collapse {

    typedef unsigned long return_type;
    
    template <typename T>
    static typename boost::enable_if_c<!(boost::is_integral<T>::value),return_type>::type sum(T& _in){

      return std::accumulate(_in.begin(),_in.end(),return_type(1),std::multiplies<return_type>());
            
    }

    template <typename T>
    static typename boost::enable_if_c<(boost::is_integral<T>::value),return_type>::type sum(T& _in){

      return _in;
            
    }
  };
  
  
} // sqeazy

#endif /* _SQEAZY_TRAITS_H_ */
