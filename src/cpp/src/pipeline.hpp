#ifndef _PIPELINE_HPP_
#define _PIPELINE_HPP_
#include <string>
#include <sstream>
#include <typeinfo>

#include "boost/mpl/vector.hpp"
#include "boost/mpl/for_each.hpp"
#include "boost/mpl/back.hpp"
#include "boost/mpl/size.hpp"
#include "boost/mpl/at.hpp"
#include "boost/mpl/reverse.hpp"


#include "sqeazy_header.hpp"
#include "sqeazy_common.hpp"
#include "sqeazy_traits.hpp"

namespace sqeazy {

  
  template <typename T, bool flag>
  struct max_bytes_encoded_detail {
    unsigned long operator()(unsigned long _data_in_bytes, 
  			     unsigned _len_header_bytes){
      return _data_in_bytes+_len_header_bytes
      ;
    }
  };

  template <typename T>
  struct max_bytes_encoded_detail<T,true> {
    unsigned long operator()(unsigned long _data_in_bytes, 
  			     unsigned _len_header_bytes){
      unsigned long compressor_bound = T::static_max_encoded_size(_data_in_bytes);
      return compressor_bound+_len_header_bytes
      ;
    }
  };


namespace bmpl = boost::mpl;

  struct get_name {

    std::string* text;

    get_name(std::string* _str):
      text(_str){}

    get_name(get_name& _rhs):
      text(_rhs.text){}

    template <typename T>
    void operator()(T any) {

      if(text) {
	*text += std::string("_");
	*text += T::static_name();
      }
    }

  };

//TODO: fix the abundant use of memory in loop_encode/_decode
// 	currently every pipeline step allocates a clone of the input data
// 	this could be fixed by a static member variable that simply needs to be resized
//   	template <typename T, typename TList, int i>
//   	std::vector<T> loop_encode<T,TList,i>::temp_ = std::vector<T>();

template <typename TList, int i>
struct loop_encode {

    typedef typename bmpl::at<TList, bmpl::int_<bmpl::size<TList>::value - i - 1> >::type current_step;
    typedef typename current_step::raw_type raw_type;
    typedef typename current_step::compressed_type compressed_type;
    typedef typename loop_encode<TList,i-1>::compressed_type next_compressed_type;
    typedef typename loop_encode<TList,i-1>::raw_type next_raw_type;

    template <typename S>
    static int apply(const raw_type* _in, compressed_type* _out, std::vector<S>& _size) {

        unsigned long total_size = std::accumulate(_size.begin(),_size.end(),1, std::multiplies<S>());
        std::vector<raw_type> temp_(total_size);
        std::copy(_in, _in + total_size, temp_.begin());

        int retvalue = current_step::static_encode(&temp_[0], _out, _size);

        std::copy(_out, _out + total_size, temp_.begin());

        next_compressed_type* next_output = reinterpret_cast<next_compressed_type*>(_out);
        const next_raw_type* next_input = reinterpret_cast<const next_raw_type*>(&temp_[0]);

        return (retvalue*(10*bmpl::size<TList>::value - i - 1)) + loop_encode<TList,i-1>::apply(next_input, next_output, _size);

    }

    template <typename S>
    static int apply(const raw_type* _in, compressed_type* _out, S& _size) {

        std::vector<raw_type> temp_(_size);
        std::copy(_in, _in + _size, temp_.begin());

        int retvalue = current_step::static_encode(&temp_[0], _out, _size);

        std::copy(_out, _out + _size, temp_.begin());

        next_compressed_type* next_output = reinterpret_cast<next_compressed_type*>(_out);

        return (retvalue*(10*bmpl::size<TList>::value - i - 1)) + loop_encode<TList,i-1>::apply(&temp_[0], next_output, _size);

    }
};



template <typename TList>
struct loop_encode<TList, -1 > {

    typedef sqeazy::unknown raw_type;
    typedef sqeazy::unknown compressed_type;

    template <typename T, typename U, typename S>
    static int apply(const T* _in, U* _out, S& _size) {

        return 0;

    }

};


template <typename TList, int i>
struct loop_decode {

    typedef typename bmpl::at<TList, bmpl::int_<bmpl::size<TList>::value - i - 1> >::type current_step;
    typedef typename current_step::raw_type raw_type;
    typedef typename current_step::compressed_type compressed_type;

    typedef typename loop_decode<TList,i-1>::compressed_type next_compressed_type;
    typedef typename loop_decode<TList,i-1>::raw_type next_raw_type;

    template <typename S>
    static int apply(const compressed_type* _in, raw_type* _out, std::vector<S>& _size) {

        unsigned long input_size = std::accumulate(_size.begin(),_size.end(),1,std::multiplies<S>());
        std::vector<compressed_type> temp_(_in, _in + input_size);

        int retvalue = current_step::static_decode(&temp_[0], _out, _size);

        std::copy(_out, _out + input_size, temp_.begin());

        const next_compressed_type* next_in = reinterpret_cast<const next_compressed_type*>(&temp_[0]);

        return (retvalue*(10*bmpl::size<TList>::value - i - 1)) + loop_decode<TList,i-1>::apply(next_in, _out, _size);

    }

    template <typename S>
    static int apply(const compressed_type* _in, raw_type* _out, S& _size) {

        std::vector<compressed_type> temp_(_in, _in + _size);

        int retvalue = current_step::static_decode(&temp_[0], _out, _size);

        std::copy(_out, _out + _size, temp_.begin());

        const next_compressed_type* next_in = reinterpret_cast<const next_compressed_type*>(&temp_[0]);
        return (retvalue*(10*bmpl::size<TList>::value - i - 1)) + loop_decode<TList,i-1>::apply(next_in, _out, _size);

    }
};

template <typename TList>
struct loop_decode<TList, -1 > {

    typedef sqeazy::unknown raw_type;
    typedef sqeazy::unknown compressed_type;

    template <typename T, typename U, typename S>
    static int apply(const T* _in, U* _out, S& _size) {

        return 0;

    }

};

  template <typename T>
  struct type_to_name_match {};

  template <> struct type_to_name_match<int> { static const std::string id(){ return "int";}};
  template <> struct type_to_name_match<short> { static const std::string id(){ return "short";}};
  template <> struct type_to_name_match<char> { static const std::string id(){ return "char";}};
  template <> struct type_to_name_match<unsigned int>	{ static const std::string id(){ return "uint";}};
  template <> struct type_to_name_match<unsigned short> { static const std::string id(){ return "ushort";}};
  template <> struct type_to_name_match<unsigned char>	{ static const std::string id(){ return "uchar";}};


template <typename TypeList>
struct pipeline : public bmpl::back<TypeList>::type {

  typedef typename bmpl::back<TypeList>::type compressor_type;
  typedef typename bmpl::at<TypeList, bmpl::int_<0> >::type first_step;

  typedef typename first_step::raw_type raw_type;
  typedef typename compressor_type::compressed_type compressed_type;

  static const int type_list_size = bmpl::size<TypeList>::value;

  static std::string static_name() {

    std::string temp = type_to_name_match<raw_type>::id();
      
    get_name extractor(&temp);
      
    bmpl::for_each<TypeList>(extractor);
      
    return temp;

  }

  pipeline(){}
  
  /**
     \brief compress through pipeline, given that pipeline contains compressor, i.e. data will shrink effectively
     
     \param[in] _in buffer containing input data
     \param[out] _out buffer containing compressed data (must at least of size max_bytes_encoded)
     \param[in] _size vector or integral describing shape of _in in units of raw_type
     \param[out] _num_compressed_bytes number of bytes that were written to _out
     \return 
     \retval 
     
  */
  template <typename SizeType, typename ScalarType>
  static typename boost::enable_if_c<sizeof(ScalarType) && compressor_type::is_sink == true,int>::type
									   compress(const raw_type* _in, 
										    compressed_type* _out,
										    SizeType& _size,
										    ScalarType& _num_compressed_bytes) {

    typedef typename first_step::compressed_type output_type;

    static const int size = type_list_size - 1;

    typedef loop_encode<TypeList , size> pipe_loop;

    //compress
    int value = pipe_loop::apply(_in, (output_type*)_out, _size);
	
    _num_compressed_bytes = compressor_type::last_num_encoded_bytes;

    //produce header
    sqeazy::image_header hdr(raw_type(),
			     _size,
			     pipeline::static_name(),
			     _num_compressed_bytes);
	
    //shift output 
    char* output_buffer = reinterpret_cast<char*>(_out);
    unsigned long hdr_shift = hdr.size();//hdr is always aligned to raw_type

    std::copy(output_buffer,
	      output_buffer+_num_compressed_bytes,
	      output_buffer+hdr_shift);

    //insert header
    std::copy(hdr.begin(), hdr.end(), output_buffer);

    _num_compressed_bytes += hdr.size();

    return value;

  }

  /**
     \brief compress through pipeline, given that pipeline is not a compressing one, i.e. data will keep it's volume
     
     \param[in] _in input buffer of size given by _size
     \param[out] _out output buffer of size equal to the input plus header
     \param[in] _size shape of in the input buffer
	  
     \return 
     \retval 
     
  */
  template <typename SizeType>
  static int compress(const raw_type* _in, 
		      compressed_type* _out,
		      SizeType& _size) {

    typedef typename first_step::compressed_type output_type;

    static const int size = type_list_size - 1;

    typedef loop_encode<TypeList , size> pipe_loop;

    unsigned long in_size_in_bytes = sizeof(raw_type)*sqeazy::collapse::sum<SizeType>(_size);
    sqeazy::image_header hdr(raw_type(),
			     _size,
			     pipeline::static_name(),
			     in_size_in_bytes);
	
    char* output_buffer = reinterpret_cast<char*>(_out);

    const unsigned long hdr_shift = hdr.size();//header is always aligned to raw_type

    std::copy(hdr.begin(), hdr.end(), output_buffer);
    output_type* first_output = reinterpret_cast<output_type*>(output_buffer+hdr_shift);

    int value = pipe_loop::apply(_in, first_output, _size);

	
    return value;
  }

  /**
     \brief compression on a pipeline that is NOT a compressor, i.e. data will keep it's volume
     
     \param[in] _in input buffer of size given by _size
     \param[in] _out output buffer of size equal to the input plus header
     \param[in] _size shape of in the input buffer
     \param[out] _num_compressed_byte number of bytes that have been written to the output buffer

     \return 
     \retval 
     
  */
  template <typename SizeType, typename ScalarType>
  static typename boost::enable_if_c<sizeof(ScalarType) && compressor_type::is_sink == false,int>::type
									   compress(const raw_type* _in, 
										    compressed_type* _out,
										    SizeType& _size,
										    ScalarType& _num_compressed_bytes) {

    int value = 1;

    value = compress(_in,_out,_size);

    unsigned long in_size_in_bytes = sizeof(raw_type)*sqeazy::collapse::sum<SizeType>(_size);
    sqeazy::image_header hdr(raw_type(),_size,pipeline::static_name(),in_size_in_bytes);
    
    _num_compressed_bytes = hdr.raw_size_byte_as<raw_type>() + hdr.size();
    return value;
      
  }

  template <typename SizeType>
  static typename boost::enable_if_c<sizeof(SizeType) && compressor_type::is_sink,int>::type
									 decompress(const compressed_type* _in, raw_type* _out, SizeType& _size) {

    typedef typename bmpl::reverse<TypeList>::type pipe_list;
    static const int size = type_list_size - 1 - 1;
    typedef loop_decode<pipe_list , size> pipe_loop;
    typedef typename pipe_loop::raw_type first_step_output_type;
    typedef typename pipe_loop::compressed_type first_step_input_type;

    sqeazy::image_header hdr(_in, _in + _size);

    //assumption: raw_type == first_step_input_type
    unsigned long temp_size_byte = hdr.raw_size_byte_as<raw_type>();
    std::vector<raw_type> temp(temp_size_byte/sizeof(raw_type));

    unsigned long hdr_shift = std::ceil(float(hdr.size())/sizeof(compressed_type));
    const compressed_type* input_begin = _in + hdr_shift;
	
    unsigned long input_size = _size - hdr_shift;
    int dec_result = compressor_type::static_decode(input_begin, &temp[0], input_size , temp_size_byte);
    dec_result *= 10*(type_list_size - 1);


    if(size<0)
      std::copy(temp.begin(),temp.end(),reinterpret_cast<raw_type*>(_out));

    unsigned found_num_dims = hdr.shape()->size();

    int ret_value = 0;

    first_step_output_type* first_out= reinterpret_cast<first_step_output_type*>(_out);
    first_step_input_type* first_in = reinterpret_cast<first_step_input_type*>(&temp[0]);

    if(found_num_dims==1) {
      unsigned first_in_n_el = temp_size_byte;
      if(!boost::is_same<first_step_input_type,void>::value)
	first_in_n_el/=sizeof(first_step_input_type);

      ret_value = pipe_loop::apply(first_in, first_out, first_in_n_el);

    }

    if(found_num_dims>1) {
      std::vector<unsigned> found_dims(hdr.shape()->begin(),hdr.shape()->end());
      ret_value = pipe_loop::apply(first_in, first_out, found_dims);
    }

    return ret_value+dec_result;
  }

  template <typename SizeType>
  static typename boost::enable_if_c<sizeof(SizeType) && compressor_type::is_sink!=true,int>::type
									 decompress(const compressed_type* _in, raw_type* _out, SizeType& _size) {


    typedef typename bmpl::reverse<TypeList>::type pipe_list;
    static const int size = type_list_size - 1 ;
    typedef loop_decode<pipe_list , size> pipe_loop;
    typedef typename pipe_loop::raw_type first_step_output_type;

    first_step_output_type* first_output = reinterpret_cast<first_step_output_type*>(_out);
    int value = pipe_loop::apply(_in, first_output, _size);

    return value;

  }

  
  template <typename U>
  static const unsigned long header_size(const std::vector<U>& _in) {

    image_header value(raw_type(),_in, pipeline::static_name());
	
    return value.size();

  }
  

  static const unsigned long header_size(unsigned long _in) {

      
    image_header value(raw_type(), _in, pipeline::static_name());
	
    return value.size();

  }

  /**
     \brief give the size of the entire encoded chunk of data in byte
     that includes header, separator and payload
     
     \param[in] _data_in_byte size of raw input data
     \param[in] _len_header_bytes size of header

     \return 
     \retval 
     
  */
  static const unsigned long static_max_bytes_encoded(unsigned long _data_in_bytes, 
					       unsigned _len_header_bytes = 0
					       ) {

    max_bytes_encoded_detail<compressor_type, compressor_type::is_sink> detail;

    return detail(_data_in_bytes, 
		  2*_len_header_bytes//to safe-guard invalid writes due to header inserts
		  );
    
  }





  template <typename U>
  static const unsigned long decoded_size_byte(const compressed_type* _buf, const U& _size) {
    
    image_header found_header(_buf, _buf + _size);
    return found_header.raw_size_byte_as<raw_type>();
    
  }

  template <typename U>
  static const std::vector<unsigned long> decode_dimensions(const compressed_type* _buf, const U& _size) {

    image_header found_header(_buf, _buf + _size);
    std::vector<unsigned long> result(found_header.shape()->begin(),found_header.shape()->end());
    return result;

  }


  template <typename U>
  static const int decoded_num_dims(const compressed_type* _buf, const U& _size) {

    image_header found_header(_buf, _buf + _size);
    return found_header.shape()->size();

  }

  static const int sizeof_raw_type(){
    return sizeof(raw_type);
  }

  static const int sizeof_compressed_type(){
    return sizeof(compressed_type);
  }
  
};
}//sqeazy

#endif /* _PIPELINEr_HPP_ */
