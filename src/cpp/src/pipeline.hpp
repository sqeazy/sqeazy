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
#include "boost/utility/enable_if.hpp"

#include "sqeazy_header.hpp"
#include "sqeazy_common.hpp"

namespace sqeazy {

  template <typename T, bool flag>
  struct max_bytes_encoded_detail {
    unsigned long operator()(unsigned long _data_in_bytes, 
  			     unsigned _len_header_bytes){
      return _data_in_bytes+_len_header_bytes;
    }
  };

  template <typename T>
  struct max_bytes_encoded_detail<T,true> {
    unsigned long operator()(unsigned long _data_in_bytes, 
  			     unsigned _len_header_bytes){
      unsigned long compressor_bound = T::max_encoded_size(_data_in_bytes);
    return compressor_bound+_len_header_bytes;
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
	*text += T::name();
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

        int retvalue = current_step::encode(&temp_[0], _out, _size);

        std::copy(_out, _out + total_size, temp_.begin());

        next_compressed_type* next_output = reinterpret_cast<next_compressed_type*>(_out);
        const next_raw_type* next_input = reinterpret_cast<const next_raw_type*>(&temp_[0]);

        return (retvalue*(10*bmpl::size<TList>::value - i - 1)) + loop_encode<TList,i-1>::apply(next_input, next_output, _size);

    }

    template <typename S>
    static int apply(const raw_type* _in, compressed_type* _out, S& _size) {

        std::vector<raw_type> temp_(_size);
        std::copy(_in, _in + _size, temp_.begin());

        int retvalue = current_step::encode(&temp_[0], _out, _size);

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

        int retvalue = current_step::decode(&temp_[0], _out, _size);

        std::copy(_out, _out + input_size, temp_.begin());

        const next_compressed_type* next_in = reinterpret_cast<const next_compressed_type*>(&temp_[0]);

        return (retvalue*(10*bmpl::size<TList>::value - i - 1)) + loop_decode<TList,i-1>::apply(next_in, _out, _size);

    }

    template <typename S>
    static int apply(const compressed_type* _in, raw_type* _out, S& _size) {

        std::vector<compressed_type> temp_(_in, _in + _size);

        int retvalue = current_step::decode(&temp_[0], _out, _size);

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


template <typename TypeList>
struct pipeline : public bmpl::back<TypeList>::type {

    typedef typename bmpl::back<TypeList>::type compressor_type;
    typedef typename bmpl::at<TypeList, bmpl::int_<0> >::type first_step;

    typedef typename first_step::raw_type raw_type;
    typedef typename compressor_type::compressed_type compressed_type;

    static const int type_list_size = bmpl::size<TypeList>::value;

    static std::string name() {

      std::string temp;
      
      get_name extractor(&temp);
      
      bmpl::for_each<TypeList>(extractor);
      
      return std::string(temp,1);

    }

  /**
     \brief compress through pipeline, given that pipeline contains compressor, i.e. data will shrink effectively
     
     \param[in] _in buffer containing input data
     \param[out] _out buffer containing compressed data (must at least of size max_bytes_encoded)
     
     \return 
     \retval 
     
  */
    template <typename SizeType, typename ScalarType>
    static typename boost::enable_if_c<sizeof(ScalarType) && compressor_type::is_compressor,int>::type
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
	sqeazy::image_header<raw_type> hdr(_size,pipeline::name());

	//shift output 
	char* output_buffer = reinterpret_cast<char*>(_out);
	unsigned long hdr_shift = hdr.size();

	std::copy(output_buffer,output_buffer+_num_compressed_bytes,output_buffer+hdr_shift);

	//insert header
	std::copy(hdr.header.begin(), hdr.header.end(), output_buffer);

	_num_compressed_bytes += hdr.size();

        return value;

    }

  /**
     \brief compress through pipeline, given that pipeline is not a compressing one, i.e. data will keep it's volume
     
     \param[in] 
     
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

	sqeazy::image_header<raw_type> hdr(_size,pipeline::name());
	char* output_buffer = reinterpret_cast<char*>(_out);
	std::copy(hdr.header.begin(), hdr.header.end(), output_buffer);

	unsigned long hdr_shift_bytes = hdr.size()// /sizeof(output_type)
	  ;

        output_type* first_output = reinterpret_cast<output_type*>(output_buffer+hdr_shift_bytes);

        int value = pipe_loop::apply(_in, first_output, _size);

	
        return value;
    }



    template <typename SizeType>
    static typename boost::enable_if_c<sizeof(SizeType) && compressor_type::is_compressor,int>::type
    decompress(const compressed_type* _in, raw_type* _out, SizeType& _size) {

        typedef typename bmpl::reverse<TypeList>::type pipe_list;
        static const int size = type_list_size - 1 - 1;
        typedef loop_decode<pipe_list , size> pipe_loop;
        typedef typename pipe_loop::raw_type first_step_output_type;
        typedef typename pipe_loop::compressed_type first_step_input_type;

	sqeazy::image_header<raw_type> hdr(_in, _in + _size);

	//assumption: raw_type == first_step_input_type
        unsigned long temp_size_byte = hdr.payload_size_byte();
        std::vector<raw_type> temp(temp_size_byte/sizeof(raw_type));

	unsigned long hdr_shift = hdr.size()/sizeof(compressed_type);
	const compressed_type* input_begin = _in + hdr_shift;
	
	unsigned long input_size = _size - (hdr.size()/sizeof(compressed_type));
        int dec_result = compressor_type::decode(input_begin, &temp[0], input_size , temp_size_byte);
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
	  std::vector<unsigned> found_dims = *hdr.shape();
	  ret_value = pipe_loop::apply(first_in, first_out, found_dims);
	}

        return ret_value+dec_result;
    }

    template <typename SizeType>
    static typename boost::enable_if_c<sizeof(SizeType) && compressor_type::is_compressor!=true,int>::type
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

      image_header<raw_type> value(_in, pipeline::name());
	
      return value.size();

    }
  

    static const unsigned long header_size(unsigned long _in) {

      
      image_header<raw_type> value(_in, pipeline::name());
	
      return value.size();

    }

  static const // typename boost::enable_if_c<compressor_type::is_compressor==true,
				     unsigned long// >::type
    max_bytes_encoded(unsigned long _data_in_bytes, 
		      unsigned _len_header_bytes = 0
		      ) {

    max_bytes_encoded_detail<compressor_type, compressor_type::is_compressor> detail;

    return detail(_data_in_bytes, _len_header_bytes);
    
  }





  template <typename U>
  static const unsigned long decoded_size_byte(const compressed_type* _buf, const U& _size) {
    
    image_header<raw_type> found_header(_buf, _buf + _size);
    return found_header.payload_size_byte();
    
  }

    template <typename U>
    static const std::vector<unsigned> decode_dimensions(const compressed_type* _buf, const U& _size) {

      image_header<raw_type> found_header(_buf, _buf + _size);

      return *(found_header.shape());

    }


    template <typename U>
    static const int decoded_num_dims(const compressed_type* _buf, const U& _size) {

      image_header<raw_type> found_header(_buf, _buf + _size);
      
      return found_header.shape()->size();

    }

};
}//sqeazy

#endif /* _PIPELINEr_HPP_ */
