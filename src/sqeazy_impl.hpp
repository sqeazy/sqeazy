#ifndef _SQEAZY_IMPL_H_
#define _SQEAZY_IMPL_H_

namespace sqeazy {

  template <typename T> struct remove_unsigned { typedef T type; };
  template <> struct remove_unsigned<unsigned short> 		{ typedef short		type; };
  template <> struct remove_unsigned<unsigned int  > 		{ typedef int		type; };
  template <> struct remove_unsigned<unsigned long > 		{ typedef long		type; };
  template <> struct remove_unsigned<unsigned long long >	{ typedef long long	type; };

  template <typename T> struct add_unsigned { typedef T type; };
  template <> struct add_unsigned<short> 	{ typedef unsigned short	type; };
  template <> struct add_unsigned<int  > 	{ typedef unsigned int		type; };
  template <> struct add_unsigned<long > 	{ typedef unsigned long		type; };
  template <> struct add_unsigned<long long >	{ typedef unsigned long long	type; };

  template <typename T> struct twice_as_wide	{};
  template	<> struct twice_as_wide<unsigned char> { typedef unsigned short		type; 	};
  template	<> struct twice_as_wide<unsigned short>{ typedef unsigned int		type; 	};
  template	<> struct twice_as_wide<unsigned int>  { typedef unsigned long		type;	};
  template	<> struct twice_as_wide<unsigned long> { typedef unsigned long long	type;	};
  template	<> struct twice_as_wide<char>	       { typedef short		type; };
  template	<> struct twice_as_wide<short>	       { typedef int		type; };
  template	<> struct twice_as_wide<int>	       { typedef long		type; };
  template	<> struct twice_as_wide<long>	       { typedef long long	type; };



  enum error_code {
    
    SUCCESS = 0,
    FAILED = 1
    
  };

  template <typename T, unsigned neighborhood_size = 3>
  struct diff_scheme {
    
    typedef T value_type;
    typedef add_unsigned<twice_as_wide<T>::type>::type sum_type;
    typedef size_t size_type;

    static const unsigned nb_size = neighborhood_size;
    static const unsigned nb_half = neighborhood_size/2;

        
    static const error_code encode(const size_type& _width,
				   const size_type& _height,
				   const size_type& _depth,
				   const value_type* _input,
				   value_type* _output) 
    {
      unsigned long length = _width*_height*_depth;
      sum_type sum = 0;
      
      
      for(unsigned long index = 0;index < length;++index){
	
	for(unsigned long z_offset = -nb_half;z_offset<=0;++z_offset){
	  for(unsigned long y_offset = -nb_half;y_offset<=0;++y_offset){
	    for(unsigned long x_offset = -nb_half;x_offset<=nb_half;++x_offset){
	      
	    }
	  }
	}
	
      }
      
      return SUCCESS;
    }
    
  };

}

#endif /* _SQEAZY_IMPL_H_ */
