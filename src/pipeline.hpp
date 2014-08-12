#ifndef _PIPELINE_HPP_
#define _PIPELINEr_HPP_
#include <string>
#include <sstream>
#include "boost/mpl/vector.hpp"
#include "boost/mpl/for_each.hpp"
#include "boost/mpl/front.hpp"
#include "boost/mpl/back.hpp"
#include "boost/mpl/size.hpp"
#include "boost/mpl/at.hpp"
#include "boost/mpl/reverse.hpp"

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
  
  template <typename T>
  struct call_encode {
    
    static const T* in_;
    static T* out_;
    static T* temp_;
    static unsigned long size_;
    
//     call_encode(const T* _in, T* _out, unsigned long _size, T* _temp = 0):
//     in_(_in),
//     out_(_out),
//     size_(_size),
//     temp_(_temp){}
    
    template <typename U>
    void operator()(U any) {

        return U::apply(in_,out_,size_);

    }
    
  };
  
  template <typename T, typename TList, int i>
  struct loop_encode{
    
    
    
    static void apply(const T* _in, T* _out, const unsigned& _size) {
      
       std::vector<T> temp_(_size);
      std::copy(_in, _in + _size, temp_.begin());
      
      
      typedef typename bmpl::at<TList, bmpl::int_<bmpl::size<TList>::value - i - 1> >::type current_step;
      current_step::apply(&temp_[0], _out, _size);
//       std::cout << "calling step " << bmpl::size<TList>::value - i - 1 << "/" << bmpl::size<TList>::value << "\t" << current_step::name() << "\n";
      
     std::copy(_out, _out + _size, temp_.begin());
      return loop_encode<T,TList,i-1>::apply(&temp_[0], _out, _size);
      
    }
    
  };
  
//   template <typename T, typename TList, int i>
//   std::vector<T> loop_encode<T,TList,i>::temp_ = std::vector<T>();
    
  
  template <typename T, typename TList>
  struct loop_encode<T,TList, -1 > {
    
    
    static void apply(const T* _in, T* _out, const unsigned& _size) {
      
      return ;
      
    }
    
  };
  
template <typename ValueT, typename TypeList>
struct pipeline : public bmpl::front<TypeList>::type, 
		 public bmpl::back<TypeList>::type {
  
  
  
  static std::string name() {
    
    static std::string temp;
    get_name extractor;
    extractor.text = &temp;
    
    bmpl::for_each<TypeList>(extractor);
    
    return temp;
    
  }
  
  template <typename SizeType>
  static int encode(const ValueT* _in, ValueT* _out, const SizeType& _size){
    
    
    static const unsigned size = bmpl::size<TypeList>::value - 1;
    
    typedef loop_encode<ValueT, TypeList , size> for_loop;
       
    for_loop::apply(_in, _out, _size);
    
    return 1;
  }
  
};
}//sqeazy

#endif /* _PIPELINEr_HPP_ */
