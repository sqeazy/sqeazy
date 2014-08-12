#ifndef _PIPELINE_HPP_
#define _PIPELINEr_HPP_
#include <string>
#include <sstream>

namespace sqeazy {

  template <typename T>
  struct none {
    
    static std::string name() {
    
    return std::string("");
    
  }
    
  };
  
  template <typename... Types>
  struct build_name;
  
  template <typename HeadT, typename... TailT>
  struct build_name<HeadT, TailT...> {
    
    static std::string str(){
      
      
      return std::string(HeadT::name()) + ":" + build_name<TailT...>::str();
      
    }
    
    
  };
  
  template <>
  struct build_name<> {
    
    static std::string str(){
      
	return "";
      
    }
    
    
  };
  
  template <typename ValueT, typename... Types>
  struct apply_step;
  
  template <typename ValueT, typename HeadT, typename... TailT>
  struct apply_step<ValueT,HeadT, TailT...> {
    
    typedef ValueT intermediate_type;
    
    static std::vector<intermediate_type*> temp;
        
    static void fwd(const ValueT* _in, ValueT* _out, const unsigned long& _size){
      
      apply_step::temp.push_back(new ValueT[_size]);
      
      HeadT::apply(_in,apply_step::temp.back(), _size);
      
      return  apply_step<ValueT, TailT...>::fwd(apply_step::temp.back(),_out,_size);
      
    }
    
    
  };
  
  template <typename ValueT>
  struct apply_step<ValueT> {
    
    typedef ValueT intermediate_type;
    
    static std::vector<intermediate_type*> temp;
    
    
    static void fwd(const ValueT* _in, ValueT* _out, const unsigned long& _size){
      
      std::copy(temp.back(), temp.back() + _size, _out);
      for( intermediate_type* v : temp )
	delete [] v;
      
      return ;
      
    }
    
    
  };
  
  
  template<typename ValueT, typename ... Steps> 
  struct pipeline : public Steps ... {
    
    static std::string name() {

        return build_name<Steps...>::str();

    }
    
    
    static void apply(ValueT* _in, ValueT* _out, const unsigned long& _size){
      
      return apply_step<ValueT, Steps...>::fwd(_in, _out, _size);
      
    }
  };
  
  
}//sqeazy

#endif /* _PIPELINEr_HPP_ */
