#ifndef _PIPELINE_HPP_
#define _PIPELINE_HPP_
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

    typedef void raw_type;
    typedef void compressed_type;

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

    typedef void raw_type;
    typedef void compressed_type;

    template <typename T, typename U, typename S>
    static int apply(const T* _in, U* _out, S& _size) {

        return 0;

    }

};


template <typename TypeList>
struct pipeline : public bmpl::back<TypeList>::type {

    typedef typename bmpl::back<TypeList>::type compressor_type;
    typedef typename compressor_type::compressed_type compressed_type;
    typedef typename bmpl::at<TypeList, bmpl::int_<0> >::type first_step;
    typedef typename first_step::raw_type raw_type;

    static const int type_list_size = bmpl::size<TypeList>::value;

    static std::string name() {

        std::string temp;
        get_name extractor;
        extractor.text = &temp;

        bmpl::for_each<TypeList>(extractor);

        return std::string(temp, 1);

    }

    template <typename SizeType, typename ScalarType>
    static typename boost::enable_if_c<sizeof(SizeType) && compressor_type::is_compressor,int>::type
    compress(const raw_type* _in, 
	     compressed_type* _out,
             SizeType& _size,
             ScalarType& _num_compressed_bytes) {

        typedef typename first_step::compressed_type output_type;

        static const int size = type_list_size - 1;

        typedef loop_encode<TypeList , size> pipe_loop;

        output_type* first_output = reinterpret_cast<output_type*>(_out);
        int value = pipe_loop::apply(_in, first_output, _size);
	
        _num_compressed_bytes = compressor_type::last_num_encoded_bytes;
        return value;

    }


    template <typename SizeType>
    static int compress(const raw_type* _in, compressed_type* _out,
                        SizeType& _size) {

        typedef typename first_step::compressed_type output_type;

        static const int size = type_list_size - 1;

        typedef loop_encode<TypeList , size> pipe_loop;

        output_type* first_output = reinterpret_cast<output_type*>(_out);
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

        unsigned long temp_size = compressor_type::decoded_size_byte(_in, _size)/sizeof(raw_type);
        std::vector<raw_type> temp(temp_size);

        int dec_result = compressor_type::decode(_in, &temp[0], _size);
        dec_result *= 10*(type_list_size - 1);


        if(size<0)
            std::copy(temp.begin(),temp.end(),reinterpret_cast<raw_type*>(_out));

        unsigned found_num_dims = compressor_type::decoded_num_dims(_in, _size);

        int ret_value = 0;
        first_step_output_type* first_out= reinterpret_cast<first_step_output_type*>(_out);
        first_step_input_type* first_in = reinterpret_cast<first_step_input_type*>(&temp[0]);

        if(found_num_dims==1) {

            ret_value = pipe_loop::apply(first_in, first_out, temp_size);

        }

        if(found_num_dims>1) {
            std::vector<unsigned> found_dims = compressor_type::decode_dimensions(_in, _size);
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

};
}//sqeazy

#endif /* _PIPELINEr_HPP_ */
