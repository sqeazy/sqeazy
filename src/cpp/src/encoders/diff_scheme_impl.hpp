#ifndef _DIFF_SCHEME_IMPL_H_
#define _DIFF_SCHEME_IMPL_H_

#include "neighborhood_utils.hpp"
#include "sqeazy_common.hpp"
#include "traits.hpp"
#include "diff_scheme_utils.hpp"
#include "dynamic_stage.hpp"



namespace sqeazy {

  //TODO: sink versus filter type as chaning the output type is optional for this operation
  template <typename in_type,
        typename Neighborhood = last_plane_neighborhood<3>,
        typename out_type =  typename remove_unsigned<in_type>::type
        >
  struct diff_scheme : public filter<in_type> {

    typedef filter<in_type> base_type;
    typedef in_type raw_type;
    typedef typename base_type::out_type compressed_type;
    typedef typename add_unsigned<typename twice_as_wide<in_type>::type >::type sum_type;

    static_assert(std::is_arithmetic<raw_type>::value==true,"[diff_scheme] input type is non-arithmetic");
    static const std::string description() { return std::string("store difference to mean of neighboring items"); };

    //TODO: check syntax of lz4 configuration at runtime
    diff_scheme(const std::string& _payload="")
      {

      }


    std::string name() const override final {

      std::ostringstream msg;
      msg << "diff"
          << Neighborhood::x_offset_end - Neighborhood::x_offset_begin << "x"
          << Neighborhood::y_offset_end - Neighborhood::y_offset_begin << "x"
          << Neighborhood::z_offset_end - Neighborhood::z_offset_begin ;

      return msg.str();

    }


    /**
       \brief serialize the parameters of this filter

       \return
       \retval string .. that encodes the configuration paramters

    */
    std::string config() const override {

      return "";

    }

    std::intmax_t max_encoded_size(std::intmax_t _size_bytes) const override final {

      return _size_bytes;
    }


    /**
     * @brief encoding the diff scheme, i.e. the output value for input intensity I is equal to the sum
     * of the neighborhood divided by the number of pixels traversed in the neighborhood
     *
     * @param _in input stack of type raw_type
     * @param _out output stack of type compressed_type but same extent than the input
     * @param _shape dimensions of the input _in

     * @return sqeazy::error_code
     */
    compressed_type* encode( const raw_type* _raw,
                             compressed_type* _compressed,
                             const std::vector<std::size_t>& _shape) override final {

      typedef std::size_t size_type;

      if(_shape.size()!=3){
        std::cerr << "[diff_scheme] unable to process input data that is not 3D\n";
        return _compressed;
      }

      std::size_t length = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());
      std::copy(_raw, _raw + length, _compressed);//crossing fingers due to possible type mismatch

      std::vector<size_type> offsets;
      sqeazy::halo<Neighborhood, size_type> geometry(_shape[row_major::w],
                                                     _shape[row_major::h],
                                                     _shape[row_major::d]);
      geometry.compute_offsets_in_x(offsets);


      size_type halo_size_x = geometry.non_halo_end(0)-geometry.non_halo_begin(0);
      if(offsets.size()==1)//no offsets in other dimensions than x
      {
        halo_size_x = length - offsets.front();
      }


      const auto n_traversed_pixels = sqeazy::num_traversed_pixels<Neighborhood>();

      out_type* signed_compressed = reinterpret_cast<out_type*>(_compressed);

      auto offsets_begin = offsets.begin();
      const omp_size_type offset_size = offsets.size();
      const int nthreads = this->n_threads();
      //auto shape_begin = _shape.data();
	  const auto pshape = _shape.data();

#pragma omp parallel for                        \
  shared(signed_compressed)                            \
  firstprivate( offset_size, halo_size_x, offsets_begin, _raw, pshape,n_traversed_pixels) \
  num_threads(nthreads)
      for(omp_size_type offset = 0; offset < offset_size; ++offset) {

        for(std::size_t index = 0; index < (std::size_t)halo_size_x; ++index) {

          auto     local_index = index + *(offsets_begin + offset);
          sum_type local_sum = naive_sum<Neighborhood>(_raw,local_index,
                                                       pshape[row_major::w],
                                                       pshape[row_major::h],
                                                       pshape[row_major::d]
            );

          *(signed_compressed + local_index) = _raw[local_index] - local_sum/n_traversed_pixels;

        }

      }

      return _compressed+length;

    }



    int decode( const compressed_type* _in, raw_type* _out,
                const std::vector<std::size_t>& _shape,
                std::vector<std::size_t> _out_shape = std::vector<std::size_t>()) const override final {

      typedef std::size_t size_type;
      if(_out_shape.empty())
        _out_shape = _shape;

      unsigned long length = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());
      std::copy(_in,_in + length, _out);

      std::vector<size_type> offsets;
      sqeazy::halo<Neighborhood, size_type> geometry(_shape[row_major::w],
                                                     _shape[row_major::h],
                                                     _shape[row_major::d]);
      geometry.compute_offsets_in_x(offsets);

      size_type halo_size_x = geometry.non_halo_end(0)-geometry.non_halo_begin(0);
      if(offsets.size()==1)//no offsets in other dimensions than x
      {
        halo_size_x = length - offsets.front();
      }

      const sum_type n_traversed_pixels = sqeazy::num_traversed_pixels<Neighborhood>();
      const out_type* signed_in = reinterpret_cast<const out_type*>(_in);
      const int nthreads = this->n_threads();
      const omp_size_type offset_size = offsets.size();
      auto offsets_begin = offsets.begin();
      auto shape_begin   = _shape.data();

#pragma omp parallel for                               \
  shared(_out)                            \
  firstprivate( offsets_begin, offset_size, n_traversed_pixels, signed_in, shape_begin ) \
  num_threads(nthreads)
      for(omp_size_type offset=0; offset<offset_size; ++offset) {

        for(std::size_t index = 0; index < (std::size_t)halo_size_x; ++index) {

          const size_type local_index = index + *(offsets_begin + offset);
          sum_type  local_sum = naive_sum<Neighborhood>(_out,
                                                        local_index,
                                                        *(shape_begin + row_major::w),
                                                        *(shape_begin + row_major::h),
                                                        *(shape_begin + row_major::d));
          _out[local_index] = signed_in[local_index] + local_sum/n_traversed_pixels;

        }
      }

      return SUCCESS;

    }



    ~diff_scheme(){};

    std::string output_type() const final override {

      return sqeazy::header_utils::represent<compressed_type>::as_string();

    }

    bool is_compressor() const final override {

      return base_type::is_compressor;

    }


  };

}

#endif /* _DIFF_SCHEME_IMPL_H_ */
