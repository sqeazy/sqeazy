#ifndef _REMOVE_ESTIMATED_BACKGROUND_SCHEME_IMPL_H_
#define _REMOVE_ESTIMATED_BACKGROUND_SCHEME_IMPL_H_

#include <sstream>
#include <string>

#include "neighborhood_utils.hpp"
#include "sqeazy_common.hpp"
#include "traits.hpp"
#include "dynamic_stage.hpp"
#include "string_parsers.hpp"
#include "background_scheme_utils.hpp"

namespace sqeazy {

  
  template <typename in_type>
  struct remove_estimated_background_scheme : public filter<in_type> {
  
    typedef filter<in_type> base_type;
    typedef in_type raw_type;
    typedef in_type compressed_type;
    
    remove_estimated_background_scheme(const std::string& _payload=""){}
    
    std::string name() const override final {

        return std::string("rmestbkrd");
    }

    std::string config() const override final {
      return std::string("");
    }


    
    std::intmax_t max_encoded_size(std::intmax_t _size_bytes) const override final {
    
      return _size_bytes;
    }
    
    ~remove_estimated_background_scheme(){};

    std::string output_type() const final override {

      return typeid(compressed_type).name();
    
    }

    bool is_compressor() const final override {
    
      return base_type::is_compressor;
    
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


        return std::string("rmestbkrd");

    }

    template <typename ItrT>
    /**
     * @brief Calculate the mean and standard deviation of the memory region between [begin,end)
     * in one go and write it to _mean and _var (this approach assumes a high statistic
     * sample contained in [begin,end))
     *
     * @param begin begin of buffer
     * @param end exclusive end of buffer
     * @param _mean float that the resulting mean is written to
     * @param _var float that the resulting standard deviation is written to
     * @return const void
     */
    static const void mean_and_var(ItrT begin, ItrT end, float& _mean, float& _var) {

        unsigned long length = end - begin;
        float sum = 0.f;
        float sum_of_squares = 0.f;

        for(; begin!=end; ++begin) {
            sum += float(*begin);
            sum_of_squares += float(*begin) * float(*begin);
        }

        _mean = sum/length;
        _var = std::sqrt((sum_of_squares/length) - (_mean*_mean));
	
    }

    template <typename size_type>
    /**
     * @brief applying the background/noise removal from an estimate of the noise level
     * given the darkest faces of the volume. if the out-of-place operation is requested (input buffer is
     * at a different memory location than the output buffer) lonely peaks on a noisy neighborhood
     * are removed from the sample
     *
     * @param _input input 3D stack encoded as raw_type
     * @param _output output 3D stack encoded as raw_type (must have same dimensionality than _input)
     * @param _dims dimensionality of input, i.e. the extents along each dimension
     * @return sqeazy::error_code
     */
    static const error_code static_encode(raw_type* _input,
                                   compressed_type* _output,
                                   const std::vector<size_type>& _dims)
    {


        std::vector<raw_type> darkest_face;
        extract_darkest_face((const raw_type*)_input, _dims, darkest_face);
        
	sqeazy::histogram<raw_type> t;
	t.add_from_image(darkest_face.begin(), darkest_face.end());
	
	unsigned long input_length = std::accumulate(_dims.begin(), _dims.end(), 1, std::multiplies<size_type>());

        const float reduce_by = t.calc_support(.99f);

	#ifdef _SQY_VERBOSE_
	std::cout << "[SQY_VERBOSE] remove_estimated_background ";
	for(short i = 0;i<_dims.size();++i){
	  std::cout << _dims[i] << ((_dims[i]!=_dims.back()) ? "x" : ", ");
	}
	
	std::cout << " darkest face: backgr_estimate = " << reduce_by << "\n";
	t.fill_stats();
	std::cout << "[SQY_VERBOSE] " << histogram<raw_type>::print_header() << "\n[SQY_VERBOSE] " << t << "\n";
	
	#endif

        if(_output) {
	    //copies the input to output, skipping pixels that have a neighborhood complying crirteria
            flatten_to_neighborhood_scheme<raw_type>::static_encode(_input, _output, _dims, reduce_by);
	    //set those pixels to 0 that fall below reduce_by
            remove_background_scheme<raw_type>::static_encode_inplace(_output, input_length, reduce_by);
        }
        else {
            std::cerr << "WARNING ["<< static_name() <<"::encode]\t inplace operation requested, flatten_to_neighborhood skipped\n";
	    //set those pixels to 0 that fall below reduce_by
            remove_background_scheme<raw_type>::static_encode_inplace(_input, input_length, reduce_by);

        }




        return SUCCESS;
    }

    template <typename SizeType>
    /**
     * @brief reconstructing the background removal from an estimate is impossible (so far),
     * therefor the input buffer is copied to the output buffer of size _length
     *
     * @return sqeazy::error_code
     */
    static const error_code static_decode(const compressed_type* _input,
                                   raw_type* _output,
                                   const SizeType& _length)
    {
        std::copy(_input, _input + _length, _output);
        return SUCCESS;
    }

    template <typename SizeType>
    /**
    * @brief reconstructing the background removal from an estimate is impossible (so far),
    * therefor the input buffer is copied to the output buffer of size given by dimensionality
    *
    * @return sqeazy::error_code
    */
    static const error_code static_decode(const compressed_type* _input,
                                   raw_type* _output,
                                   const std::vector<SizeType>& _length)
    {
        unsigned long total_size = std::accumulate(_length.begin(), _length.end(), 1, std::multiplies<SizeType>());

        return static_decode(_input, _output, total_size);
    }



  };

}

#endif /* _REMOVE_ESTIMATED_BACKGROUND_SCHEME_IMPL_H_ */
