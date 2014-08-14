#ifndef _PIPELINE_HPP_
#define _PIPELINEr_HPP_
#include <string>
#include <sstream>
#include "boost/mpl/vector.hpp"
#include "boost/mpl/for_each.hpp"
#include "boost/mpl/back.hpp"
#include "boost/mpl/size.hpp"
#include "boost/mpl/at.hpp"
#include "boost/mpl/reverse.hpp"
#include "boost/utility/enable_if.hpp"
 
namespace sqeazy {

  namespace bmpl = boost::mpl;
  struct get_name {
    
    std::string* text;
    
    template <typename T>
    void operator()(T any) {

        if(text)
            *text += T::name();

    }
    
  };
  
  
  template <typename T, typename TList, int i>
  struct loop_encode{
    
    
    template <typename S>
    static int apply(const T* _in, T* _out, S& _size) {
      
       std::vector<T> temp_(_size);
      std::copy(_in, _in + _size, temp_.begin());
      
      
      typedef typename bmpl::at<TList, bmpl::int_<bmpl::size<TList>::value - i - 1> >::type current_step;
      int retvalue = current_step::encode(&temp_[0], _out, _size);
//       std::cout << "calling step " << bmpl::size<TList>::value - i - 1 << "/" << bmpl::size<TList>::value << "\t" << current_step::name() << "\n";
      
     std::copy(_out, _out + _size, temp_.begin());
      return (retvalue*(10*bmpl::size<TList>::value - i - 1)) + loop_encode<T,TList,i-1>::apply(&temp_[0], _out, _size);
      
    }
    
  };

//TODO: this is left in, to remind me of fixing the abundant use of memory in loop_encode  
//   template <typename T, typename TList, int i>
//   std::vector<T> loop_encode<T,TList,i>::temp_ = std::vector<T>();
    
  
  template <typename T, typename TList>
  struct loop_encode<T,TList, -1 > {
    
    template <typename S>
    static int apply(const T* _in, T* _out, S& _size) {
      
      return 0;
      
    }
    
  };
  
  
  template <typename T, typename TList, int i>
  struct loop_decode{
    
    
    template <typename S>
    static int apply(const T* _in, T* _out, S& _size) {

        std::vector<T> temp_(_size);
        std::copy(_in, _in + _size, temp_.begin());


        typedef typename bmpl::at<TList, bmpl::int_<bmpl::size<TList>::value - i - 1> >::type current_step;

        int retvalue = current_step::decode(&temp_[0], _out, _size);
//       std::cout << "calling step " << bmpl::size<TList>::value - i - 1 << "/" << bmpl::size<TList>::value << "\t" << current_step::name() << "\n";

		
        std::copy(_out, _out + _size, temp_.begin());
        return (retvalue*(10*bmpl::size<TList>::value - i - 1)) + loop_decode<T,TList,i-1>::apply(&temp_[0], _out, _size);

    }
    
  };
  
  template <typename T, typename TList>
  struct loop_decode<T,TList, -1 > {
    
    template <typename S>
    static int apply(const T* _in, T* _out, S& _size) {
      
      return 0;
      
    }
    
  };
  

//   template <typename ValueT, typename TypeList, typename CompressType ,bool is_compressor>
//   struct defer_compressor{
//     
//     template <typename SizeType>
//     static int decompress(const ValueT* _in, ValueT* _out, SizeType& _size){
//       
//       typedef typename bmpl::reverse<TypeList>::type pipe_list;
//       
//       static const unsigned size = bmpl::size<pipe_list>::value - 1 ;
//     typedef loop_decode<ValueT, pipe_list , size> pipe_loop;
//        
//     int value = pipe_loop::apply(_in, _out, _size);
//     
//     return value;
//       
//     }
//     
//   };
//   
//   template <typename ValueT, typename TypeList, typename CompressType >
//   struct defer_compressor<ValueT,TypeList,CompressType,true> {
//     
//     template <typename SizeType>
//     static int decompress(const ValueT* _in, ValueT* _out, SizeType& _size){
//       
//       typedef typename bmpl::reverse<TypeList>::type pipe_list;
//     
//     
//     unsigned long buffer_size = CompressType::decoded_size(_in, _size);
//     std::vector<ValueT> temp(buffer_size);
//     int dec_result = CompressType::decode(_in, &temp[0], _size);
//     
//     static const unsigned size = bmpl::size<pipe_list>::value - 1 - 1;
//     typedef loop_decode<ValueT, pipe_list , size> pipe_loop;
//        
//     int value = pipe_loop::apply(&temp[0], _out, buffer_size);
//     
//     return value+10+dec_result;
//     
//     }
//     
//   };


template <typename ValueT, typename TypeList>
struct pipeline : public bmpl::back<TypeList>::type {
  
  typedef typename bmpl::back<TypeList>::type compressor_type;
  
  static std::string name() {
    
    static std::string temp;
    get_name extractor;
    extractor.text = &temp;
    
    bmpl::for_each<TypeList>(extractor);
    
    return temp;
    
  }
  
  template <typename SizeType>
  static int compress(const ValueT* _in, ValueT* _out, SizeType& _size){
    
    
    static const unsigned size = bmpl::size<TypeList>::value - 1;
    
    typedef loop_encode<ValueT, TypeList , size> pipe_loop;
       
    int value = pipe_loop::apply(_in, _out, _size);
    
    return value;
  }
  
  
  
  template <typename SizeType>
  static typename boost::enable_if_c<sizeof(SizeType) && compressor_type::is_compressor,int>::type decompress(const ValueT* _in, ValueT* _out, SizeType& _size){
         
      
    typedef typename bmpl::reverse<TypeList>::type pipe_list;
    
    
    unsigned long buffer_size = compressor_type::decoded_size(_in, _size);
    std::vector<ValueT> temp(buffer_size);
    int dec_result = compressor_type::decode(_in, &temp[0], _size);
    
    static const unsigned size = bmpl::size<pipe_list>::value - 1 - 1;
    typedef loop_decode<ValueT, pipe_list , size> pipe_loop;
       
    int value = pipe_loop::apply(&temp[0], _out, buffer_size);
    
    return value+10+dec_result;
  }
  
  template <typename SizeType>
  static typename boost::enable_if_c<sizeof(SizeType) && compressor_type::is_compressor!=true,int>::type decompress(const ValueT* _in, ValueT* _out, SizeType& _size){
         
    
        typedef typename bmpl::reverse<TypeList>::type pipe_list;

        static const unsigned size = bmpl::size<pipe_list>::value - 1 ;
        typedef loop_decode<ValueT, pipe_list , size> pipe_loop;

        int value = pipe_loop::apply(_in, _out, _size);

        return value;
    
  }
  
};
}//sqeazy

#endif /* _PIPELINEr_HPP_ */
