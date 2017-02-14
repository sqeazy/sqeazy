#ifndef _REMOVE_BACKGROUND_SCHEME_IMPL_H_
#define _REMOVE_BACKGROUND_SCHEME_IMPL_H_

//#include "neighborhood_utils.hpp"
#include "sqeazy_common.hpp"
#include "traits.hpp"
#include "dynamic_stage.hpp"
#include "string_parsers.hpp"
#include "background_scheme_utils.hpp"

namespace sqeazy {

  //TODO: remove static_num_bits_per_plane as soon as deprecated API is gone
  template <typename in_type>
  struct remove_background_scheme : public filter<in_type> {

    typedef filter<in_type> base_type;
    typedef in_type raw_type;
    typedef in_type compressed_type;

    in_type threshold;

    static_assert(std::is_arithmetic<raw_type>::value==true,"[remove_background_scheme] input type is non-arithmetic");
    static const std::string description() { return std::string("set all items to 0 that are below <threshold>, reduce any other item by threshold"); };

    //TODO: check syntax of lz4 configuration at runtime
    remove_background_scheme(const std::string& _payload=""):
      threshold(0)
      {

        pipeline_parser p;
        auto config_map = p.minors(_payload.begin(),_payload.end());

        if(config_map.size()){
          auto f_itr = config_map.find("threshold");

          if(f_itr!=config_map.end())
            threshold = std::stoi(f_itr->second);
        }

      }

    remove_background_scheme(raw_type _tvalue):
      threshold(_tvalue)
      {
      }

    std::string name() const override final {

      return std::string("remove_background");

    }


    /**
       \brief serialize the parameters of this filter

       \return
       \retval string .. that encodes the configuration paramters

    */
    std::string config() const {

      std::ostringstream msg;
      msg << "threshold=" << std::to_string(threshold);
      return msg.str();

    }

    std::intmax_t max_encoded_size(std::intmax_t _size_bytes) const override final {

      return _size_bytes;
    }

    compressed_type* encode( const raw_type* _input, compressed_type* _output, std::size_t _input_size) override final {

      compressed_type* end_itr = nullptr;
      // if(this->nthreads()==1){
      //   end_itr = std::transform(_input,_input + _input_size,
      //                               _output,
      //                               [=](const raw_type& value){
      //                                 if(value > threshold)
      //                                   return value - threshold;
      //                                 else
      //                                   return 0;
      //                               });
      // } else {
      const omp_size_type nthreads  = this->nthreads();
      const omp_size_type chunk_size = (_input_size + nthreads -1)/nthreads;
      const omp_size_type len = _input_size;

#pragma omp parallel for                        \
  shared(_output)                               \
  firstprivate( _input, chunk_size, threshold)  \
  schedule(static,chunk_size)                   \
      num_threads(nthreads)
        for(omp_size_type i=0; i<len; ++i){
          _output[i] = _input[i] > threshold ? _input[i] - threshold : 0;
        }

      end_itr = _output + len;
      // }
      return end_itr;

    }

    compressed_type* encode( const raw_type* _input, compressed_type* _output, const std::vector<std::size_t>& _shape) override final {

      std::size_t length = std::accumulate(_shape.begin(), _shape.end(), 1, std::multiplies<std::size_t>());
      return encode(_input,_output,length);

    }


    int decode( const compressed_type* _input,
                raw_type* _output,
                std::size_t _input_size,
                std::size_t _output_size = 0) const override final {

      if(_input!=_output ){

        if(this->n_threads() == 1)
          std::copy(_input, _input + _input_size, _output);
        else{
          const int nthreads = this->n_threads();
          omp_size_type len = _input_size;

          #pragma omp parallel for                  \
            shared(_output)                                             \
            firstprivate( _input )                                      \
            num_threads(nthreads)
          for(omp_size_type i = 0;i<len;++i){
            _output[i] = _input[i];
          }
        }
      }

      // std::copy(_input, _input + _input_size,_output);

      return 0;
    }

    int decode( const compressed_type* _input, raw_type* _output,
                const std::vector<std::size_t>& _shape,
                std::vector<std::size_t>) const override final {

      std::size_t length = std::accumulate(_shape.begin(), _shape.end(), 1, std::multiplies<std::size_t>());
      return decode(_input,_output,length);
    }



    ~remove_background_scheme(){};

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


      return std::string("rmbkrd");

    }


/**
 * @brief [length given as vector] removal of _threshold from _input buffer, i.e. if any value
 * inside _input is found to be <= _threshold it is set to 0, it is kept otherwise (inplace version)
 * @param _input 3D input stack of type raw_type
 * @param _output 3D output stack of type raw_type
 * @param _length length of the above in units of raw_type
 * @param _threshold threshold to apply
 * @return sqeazy::error_code
 */
    template <typename size_type>
    static const error_code static_encode(raw_type* _input,
                                          raw_type* _output,
                                          const std::vector<size_type>& _data,
                                          const raw_type& _threshold
      )
      {

        unsigned long length = std::accumulate(_data.begin(), _data.end(), 1, std::multiplies<size_type>());

        return static_encode(_input, _output, length, _threshold);

      }

    /**
     * @brief [length given as scalar] removal of _threshold from _input buffer, i.e. if any value
     * inside _input is found to be <= _threshold it is set to 0, it is kept otherwise (inplace version)
     * @param _input 3D input stack of type raw_type
     * @param _output 3D output stack of type raw_type
     * @param _length length of the above in units of raw_type
     * @param _threshold threshold to apply
     * @return sqeazy::error_code
     */
    template <typename size_type>
    static const error_code static_encode(raw_type* _input,
                                          raw_type* _output,
                                          const size_type& _length,
                                          const raw_type& _threshold)
      {
        if(_output)
          return static_encode_out_of_place(_input, _output, _length, _threshold);
        else
          return static_encode_inplace(_input, _length, _threshold);

      }



    /**
     * @brief clipping removal of _threshold from _input buffer, i.e. if any value
     * inside _input is found to be <= _threshold it is set to 0, it is kept otherwise (inplace version)
     * @param _input 3D input stack of type raw_type
     * @param _output 3D output stack of type raw_type
     * @param _length length of the above in units of raw_type
     * @param _threshold threshold to apply
     * @return sqeazy::error_code
     */
    template <typename size_type>
    static const error_code static_encode_out_of_place(raw_type* _input,
                                                       raw_type* _output,
                                                       const size_type& _length,
                                                       const raw_type& _threshold)
      {
        const unsigned long size = _length;
        for(unsigned long vox = 0; vox<size; ++vox) {
          _output[vox] = (_input[vox] > _threshold) ? _input[vox] - _threshold : 0;
        }

        return SUCCESS;
      }



    /**
     * @brief clipping removal of _threshold from _input buffer, i.e. if any value
     * inside _input is found to be <= _threshold it is set to 0, it is kept otherwise (inplace version)
     * @param _input 3D input stack of type raw_type
     * @param _length length of the above in units of raw_type
     * @param _threshold threshold to apply
     * @return sqeazy::error_code
     */
    template <typename size_type>
    static const error_code static_encode_inplace(raw_type* _input,
                                                  const size_type& _length,
                                                  const raw_type& _threshold)
      {
        const unsigned long size = _length;
        for(unsigned long vox = 0; vox<size; ++vox) {
          _input[vox] = (_input[vox] > _threshold) ? _input[vox] - _threshold : 0;
        }


        return SUCCESS;
      }



    /**
     * @brief reconstructing the background removal from an estimate is impossible (so far),
     * therefor the input buffer is copied to the output buffer of size given by dimensionality
     *
     * @return sqeazy::error_code
     */
    template <typename size_type>
    static const error_code static_decode(const raw_type* _input,
                                          raw_type* _output,
                                          const size_type& _length)
      {
        std::copy(_input, _input + _length, _output);
        return SUCCESS;
      }


    /**
     * @brief reconstructing the background removal from an estimate is impossible (so far),
     * therefor the input buffer is copied to the output buffer of size given by dimensionality
     *
     * @return sqeazy::error_code
     */
    template <typename size_type>
    static const error_code static_decode(const raw_type* _input,
                                          raw_type* _output,
                                          const std::vector<size_type>& _length)
      {
        unsigned long total_size = std::accumulate(_length.begin(), _length.end(), 1, std::multiplies<size_type>());

        return static_decode(_input, _output, total_size);
      }


  };

}

#endif /* _REMOVE_BACKGROUND_SCHEME_IMPL_H_ */
