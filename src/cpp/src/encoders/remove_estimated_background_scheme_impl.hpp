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

#include "flatten_to_neighborhood_scheme_impl.hpp"
#include "remove_background_scheme_impl.hpp"

namespace sqeazy {


  template <typename in_type>
  struct remove_estimated_background_scheme : public filter<in_type> {

    typedef filter<in_type> base_type;
    typedef in_type raw_type;
    typedef in_type compressed_type;

    remove_estimated_background_scheme(const std::string& _payload=""){}
    static const std::string description() {
      return std::string("estimate noise from darkest planes, remove median of that plane from all pixels/voxels");
    };

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


    compressed_type* encode( const raw_type* _input, compressed_type* _output, std::size_t _input_size) override final {

      std::vector<std::size_t> shape = {_input_size};

      return encode(_input,_output,shape);

    }

    compressed_type* encode( const raw_type* _input, compressed_type* _output, const std::vector<std::size_t>& _shape) override final {

      std::vector<raw_type> darkest_face;
      extract_darkest_face((const raw_type*)_input, _shape, darkest_face);

      sqeazy::histogram<raw_type> t;
      t.add_from_image(darkest_face.begin(), darkest_face.end());

      std::size_t input_length = std::accumulate(_shape.begin(), _shape.end(), 1, std::multiplies<std::size_t>());

      const float reduce_by = t.calc_support(.99f);

#ifdef _SQY_VERBOSE_
      std::cout << "[SQY_VERBOSE] remove_estimated_background ";
      for(short i = 0;i<_shape.size();++i){
        std::cout << _shape[i] << ((_shape[i]!=_shape.back()) ? "x" : ", ");
      }

      std::cout << " darkest face: backgr_estimate = " << reduce_by << "\n";
      t.fill_stats();
      std::cout << "[SQY_VERBOSE] " << histogram<raw_type>::print_header() << "\n[SQY_VERBOSE] " << t << "\n";

#endif

      if(_output) {
        //copies the input to output, skipping pixels that have a neighborhood complying criteria
        flatten_to_neighborhood_scheme<raw_type> flatten(reduce_by,.5);
        //flatten_to_neighborhood_scheme<raw_type>::static_encode(_input, _output, _shape, reduce_by);
        flatten.encode(_input, _output, _shape);
        //set those pixels to 0 that fall below reduce_by
        remove_background_scheme<raw_type> reduce(reduce_by);
        reduce.encode(_output, _output,  input_length);//do it inplace!

        //remove_background_scheme<raw_type>::static_encode_inplace(_output, input_length, reduce_by);
      }
      else {
        std::cerr << "WARNING ["<< name() <<"::encode]\t inplace operation not supported\n";
        return nullptr;
      }

      return _output+input_length;

    }

    /**
       \brief decode method basically does a copy only as we cannot infer the estimated background level (yet)

       \param[in]

       \return
       \retval

    */
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

    /**
       \brief decode method basically does a copy only as we cannot infer the estimated background level (yet)

       \param[in]

       \return
       \retval

    */
    int decode( const compressed_type* _input, raw_type* _output,
                const std::vector<std::size_t>& _shape,
                std::vector<std::size_t>) const override final {

      std::size_t length = std::accumulate(_shape.begin(), _shape.end(), 1, std::multiplies<std::size_t>());
      return decode(_input,_output,length);
    }

  };

}

#endif /* _REMOVE_ESTIMATED_BACKGROUND_SCHEME_IMPL_H_ */
