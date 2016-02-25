#ifndef _SQEAZY_IMPL_H_
#define _SQEAZY_IMPL_H_
#include <functional>
#include <algorithm>
#include <sstream>
#include <climits>
#include <numeric>
#include "sqeazy_common.hpp"
#include "traits.hpp"

//#include "neighborhood_utils.hpp"

#include "hist_impl.hpp"
#include "huffman_utils.hpp"

//converted schemes
#include "diff_scheme_impl.hpp"
#include "bitswap_scheme_impl.hpp"
#include "remove_background_scheme_impl.hpp"
#include "flatten_to_neighborhood_scheme_impl.hpp"
#include "remove_estimated_background_scheme_impl.hpp"

#include "dynamic_stage.hpp"

namespace sqeazy {

  template <typename in_type, typename out_type = char >
  struct pass_through : public sink<in_type,out_type>  {

    typedef sink<in_type,out_type> base_type;
    typedef in_type raw_type;
    typedef typename base_type::out_type compressed_type;


    static_assert(std::is_arithmetic<raw_type>::value==true,"[pass_through] input type is non-arithmetic");

    //TODO: check syntax of lz4 configuration at runtime
    pass_through(const std::string& _payload="")
    {

    }




    std::string name() const override final {

      return std::string("pass_through");
	
    }


    /**
       \brief serialize the parameters of this filter
     
       \return 
       \retval string .. that encodes the configuration paramters
     
    */
    std::string config() const {

      return "";
    
    }

    std::intmax_t max_encoded_size(std::intmax_t _size_bytes) const override final {
    
      return _size_bytes;
    }

    
    /**
     * @brief encode input raw_type buffer and write to output (not owned, not allocated)
     *
     * @param _input input raw_type buffer
     * @param _output output char buffer (not owned, not allocated)
     * @param _length mutable std::vector<size_type>, contains the length of _input at [0] and the number of written bytes at [1]
     * @return sqeazy::error_code
     */
    compressed_type* encode( const raw_type* _in, compressed_type* _out, std::vector<std::size_t> _shape) override final {

      
      std::size_t in_elements = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());
      raw_type* out_begin = reinterpret_cast<raw_type*>(_out);
      std::copy(_in,_in+in_elements,out_begin);

      std::size_t num_written_bytes = in_elements*sizeof(raw_type)/sizeof(compressed_type);
      
      return _out+num_written_bytes;
    }



    int decode( const compressed_type* _in, raw_type* _out, std::vector<std::size_t> _shape) const override final {

      std::size_t in_elements = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());
      compressed_type* out_begin = reinterpret_cast<compressed_type*>(_out);
      std::copy(_in,_in+in_elements,out_begin);
      
      return 0;
      
    }
    
    

    ~pass_through(){};

    std::string output_type() const final override {

      return typeid(compressed_type).name();
    
    }

    bool is_compressor() const final override {
    
      return sink<in_type>::is_compressor;
    
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // DEPRECATED API
    
  static const bool is_sink = false;

  /**
   * @brief producing the name of this scheme and return it as a string
   *
   * @return const std::string
   */
  static const std::string static_name() {

    //TODO: add name of Neighborhood
    std::ostringstream msg;
    msg << "" ;
    return msg.str();

  }


  template <typename size_type>
  /**
   * @brief encoding the diff scheme, i.e. the output value for input intensity I is equal to the sum 
   * of the neighborhood divided by the number of pixels traversed in the neighborhood
   * 
   * @param _width width of input stack
   * @param _height height of the input stack
   * @param _depth depth of the input stack
   * @param _input input stack of type raw_type
   * @param _output output stack of type compressed_type but same extent than the input
   * @return sqeazy::error_code
   */
  static const error_code static_encode(const size_type& _width,
				 const size_type& _height,
				 const size_type& _depth,
				 const raw_type* _input,
				 compressed_type* _output)
  {

    unsigned long length = _width*_height*_depth;
    std::copy(_input, _input + length, _output);//crossing fingers due to possible type mismatch
    return SUCCESS;
  }


  template <typename size_type>
  static const error_code static_encode(const raw_type* _input,
				 compressed_type* _output,
				 size_type& _dim
				 )
  {

    return static_encode(_dim, 1, 1, _input, _output);
  }

  template <typename size_type>
  static const error_code static_encode(const raw_type* _input,
				 compressed_type* _output,
				 std::vector<size_type>& _dims
				 )
  {
    return static_encode(_dims.at(0), _dims.at(1), _dims.at(2), _input, _output);
  }

  template <typename size_type>
  /**
   * @brief reconstructing data that was encoded by this diff scheme
   * 
   * @return sqeazy::error_code
   */
  static const error_code static_decode(const size_type& _width,
				 const size_type& _height,
				 const size_type& _depth,
				 const compressed_type* _input,
				 raw_type* _output)
  {
    unsigned long length = _width*_height*_depth;
    std::copy(_input,_input + length, _output);
    return SUCCESS;
  }


  template <typename size_type>
  static const error_code static_decode(const compressed_type* _input,
				 raw_type* _output,
				 std::vector<size_type>& _dims
				 ) {

    return static_decode(_dims.at(0), _dims.at(1), _dims.at(2), _input, _output);

  }

  template <typename size_type>
  static const error_code static_decode(const compressed_type* _input,
				 raw_type* _output,
				 size_type& _dim
				 ) {

    const size_type one = 1;
    return static_decode(_dim, one, one, _input, _output);

  }
  
  template <typename U>
  static const unsigned long max_encoded_size(U _src_length_in_bytes){
    return _src_length_in_bytes;
  }
};





// template < typename T >
// struct remove_estimated_background {

//     typedef T raw_type;
//     typedef T compressed_type;
//     static const bool is_sink = false;

//     /**
//      * @brief producing the name of this scheme and return it as a string
//      *
//      * @return const std::string
//      */
//     static const std::string static_name() {


//         return std::string("rmestbkrd");

//     }

//     template <typename ItrT>
//     /**
//      * @brief Calculate the mean and standard deviation of the memory region between [begin,end)
//      * in one go and write it to _mean and _var (this approach assumes a high statistic
//      * sample contained in [begin,end))
//      *
//      * @param begin begin of buffer
//      * @param end exclusive end of buffer
//      * @param _mean float that the resulting mean is written to
//      * @param _var float that the resulting standard deviation is written to
//      * @return const void
//      */
//     static const void mean_and_var(ItrT begin, ItrT end, float& _mean, float& _var) {

//         unsigned long length = end - begin;
//         float sum = 0.f;
//         float sum_of_squares = 0.f;

//         for(; begin!=end; ++begin) {
//             sum += float(*begin);
//             sum_of_squares += float(*begin) * float(*begin);
//         }

//         _mean = sum/length;
//         _var = std::sqrt((sum_of_squares/length) - (_mean*_mean));
	
//     }

//     template <typename size_type>
//     /**
//      * @brief applying the background/noise removal from an estimate of the noise level
//      * given the darkest faces of the volume. if the out-of-place operation is requested (input buffer is
//      * at a different memory location than the output buffer) lonely peaks on a noisy neighborhood
//      * are removed from the sample
//      *
//      * @param _input input 3D stack encoded as raw_type
//      * @param _output output 3D stack encoded as raw_type (must have same dimensionality than _input)
//      * @param _dims dimensionality of input, i.e. the extents along each dimension
//      * @return sqeazy::error_code
//      */
//     static const error_code static_encode(raw_type* _input,
//                                    compressed_type* _output,
//                                    const std::vector<size_type>& _dims)
//     {


//         std::vector<raw_type> darkest_face;
//         extract_darkest_face((const raw_type*)_input, _dims, darkest_face);
        
// 	sqeazy::histogram<raw_type> t;
// 	t.add_from_image(darkest_face.begin(), darkest_face.end());
	
// 	unsigned long input_length = std::accumulate(_dims.begin(), _dims.end(), 1, std::multiplies<size_type>());

//         const float reduce_by = t.calc_support(.99f);

// 	#ifdef _SQY_VERBOSE_
// 	std::cout << "[SQY_VERBOSE] remove_estimated_background ";
// 	for(short i = 0;i<_dims.size();++i){
// 	  std::cout << _dims[i] << ((_dims[i]!=_dims.back()) ? "x" : ", ");
// 	}
	
// 	std::cout << " darkest face: backgr_estimate = " << reduce_by << "\n";
// 	t.fill_stats();
// 	std::cout << "[SQY_VERBOSE] " << histogram<raw_type>::print_header() << "\n[SQY_VERBOSE] " << t << "\n";
	
// 	#endif

//         if(_output) {
// 	    //copies the input to output, skipping pixels that have a neighborhood complying crirteria
//             flatten_to_neighborhood_scheme<raw_type>::static_encode(_input, _output, _dims, reduce_by);
// 	    //set those pixels to 0 that fall below reduce_by
//             remove_background_scheme<raw_type>::static_encode_inplace(_output, input_length, reduce_by);
//         }
//         else {
//             std::cerr << "WARNING ["<< static_name() <<"::encode]\t inplace operation requested, flatten_to_neighborhood skipped\n";
// 	    //set those pixels to 0 that fall below reduce_by
//             remove_background_scheme<raw_type>::static_encode_inplace(_input, input_length, reduce_by);

//         }




//         return SUCCESS;
//     }

//     template <typename SizeType>
//     /**
//      * @brief reconstructing the background removal from an estimate is impossible (so far),
//      * therefor the input buffer is copied to the output buffer of size _length
//      *
//      * @return sqeazy::error_code
//      */
//     static const error_code static_decode(const compressed_type* _input,
//                                    raw_type* _output,
//                                    const SizeType& _length)
//     {
//         std::copy(_input, _input + _length, _output);
//         return SUCCESS;
//     }

//     template <typename SizeType>
//     /**
//     * @brief reconstructing the background removal from an estimate is impossible (so far),
//     * therefor the input buffer is copied to the output buffer of size given by dimensionality
//     *
//     * @return sqeazy::error_code
//     */
//     static const error_code static_decode(const compressed_type* _input,
//                                    raw_type* _output,
//                                    const std::vector<SizeType>& _length)
//     {
//         unsigned long total_size = std::accumulate(_length.begin(), _length.end(), 1, std::multiplies<SizeType>());

//         return static_decode(_input, _output, total_size);
//     }


// };
} //sqeazy

#endif /* _SQEAZY_IMPL_H_ */
