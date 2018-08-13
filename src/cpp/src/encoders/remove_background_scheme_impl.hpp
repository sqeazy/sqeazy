#ifndef _REMOVE_BACKGROUND_SCHEME_IMPL_H_
#define _REMOVE_BACKGROUND_SCHEME_IMPL_H_

//#include "neighborhood_utils.hpp"
#include "sqeazy_common.hpp"
#include "traits.hpp"
#include "dynamic_stage.hpp"
#include "string_parsers.hpp"
#include "background_scheme_utils.hpp"

namespace sqeazy {

  template <typename in_type>
  struct remove_background_scheme : public filter<in_type> {

    typedef filter<in_type> base_type;
    typedef in_type raw_type;
    typedef in_type compressed_type;

    in_type threshold;

    static_assert(std::is_arithmetic<raw_type>::value==true,"[remove_background_scheme] input type is non-arithmetic");
    static const std::string description() { return std::string("set all items to 0 that are below <threshold>, reduce any other item by threshold"); };

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
    std::string config() const override {

      std::ostringstream msg;
      msg << "threshold=" << std::to_string(threshold);
      return msg.str();

    }

    std::intmax_t max_encoded_size(std::intmax_t _size_bytes) const override final {

      return _size_bytes;
    }

    compressed_type* encode( const raw_type* _input, compressed_type* _output, std::size_t _input_size) override final {

      compressed_type* end_itr = nullptr;

      const omp_size_type nthreads  = this->n_threads();
      const omp_size_type chunk_size = (_input_size + nthreads -1)/nthreads;
      const omp_size_type len = _input_size;
      const auto local_threshold = this->threshold;

#pragma omp parallel for                        \
  shared(_output)                               \
  firstprivate( _input, chunk_size, local_threshold)  \
  schedule(static,chunk_size)                   \
      num_threads(nthreads)
        for(omp_size_type i=0; i<len; ++i){
          _output[i] = _input[i] > threshold ? _input[i] - threshold : 0;
        }

      end_itr = _output + len;

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

      return sqeazy::header_utils::represent<compressed_type>::as_string();

    }

    bool is_compressor() const final override {

      return base_type::is_compressor;

    }


  };

}

#endif /* _REMOVE_BACKGROUND_SCHEME_IMPL_H_ */
