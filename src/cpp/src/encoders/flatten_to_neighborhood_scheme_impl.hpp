#ifndef _FLATTEN_TO_NEIGHBORHOOD_SCHEME_IMPL_H_
#define _FLATTEN_TO_NEIGHBORHOOD_SCHEME_IMPL_H_

#include <sstream>
#include <string>

#include "neighborhood_utils.hpp"
#include "sqeazy_common.hpp"
#include "traits.hpp"
#include "dynamic_stage.hpp"
#include "string_parsers.hpp"
#include "background_scheme_utils.hpp"

namespace sqeazy {


  /**
* @brief this implements a shot noise type removal scheme in static member function encode, i.e. inside the neighborhood of a pixel
* in _input, the number of pixels are counted that fall under a certain threshold, if this count exceeds
* the limit "percentage_below" (or the one given at runtime), the central pixel
* (around which the neighborhood is located) is set to 0 as well.
*
* this scheme cannot be run inplace.
* this scheme is not reversable.
*
*/
  template <typename in_type,
        typename Neighborhood = cube_neighborhood<5>,
        short percentage_below = 50>
  struct flatten_to_neighborhood_scheme : public filter<in_type> {

    typedef filter<in_type> base_type;
    typedef in_type raw_type;
    typedef in_type compressed_type;

    static const std::string description() { return std::string("shot noise removal scheme, if <faction|default = 50%> of pixels in neighborhood (default: 5x5x5) fall below <threshold|default = 1>, set pixel to 0"); };
    raw_type threshold;
    float fraction;

    flatten_to_neighborhood_scheme(raw_type _threshold, float _frac = percentage_below/100.f):
      threshold(_threshold),
      fraction(_frac){
    }


    flatten_to_neighborhood_scheme(const std::string& _payload=""):
      threshold(1),
      fraction(percentage_below/100.f){

      pipeline_parser p;
      auto config_map = p.minors(_payload.begin(),_payload.end());

      if(config_map.size()){
        auto f_itr = config_map.find("fraction");
        if(f_itr!=config_map.end())
          fraction = std::stof(f_itr->second);

        f_itr = config_map.find("threshold");
        if(f_itr!=config_map.end())
          threshold = std::stoi(f_itr->second);
      }
    }

    std::string name() const override final {

      std::ostringstream msg;
      msg << "rmbkrd_neighbor"
          << Neighborhood::x_offset_end - Neighborhood::x_offset_begin << "x"
          << Neighborhood::y_offset_end - Neighborhood::y_offset_begin << "x"
          << Neighborhood::z_offset_end - Neighborhood::z_offset_begin ;

      return msg.str();

    }

    std::string config() const override final {

      std::ostringstream msg;
      msg << "threshold=" << std::to_string(threshold) << ",";
      msg << "fraction=" << std::to_string(fraction);
      return msg.str();

    }

    std::intmax_t max_encoded_size(std::intmax_t _size_bytes) const override final {

      return _size_bytes;
    }

    compressed_type* encode( const raw_type* _input,
                             compressed_type* _output,
                             const std::vector<std::size_t>& _shape) override final {

      typedef std::size_t size_type;
      unsigned long length = std::accumulate(_shape.begin(), _shape.end(), 1, std::multiplies<size_type>());

      std::vector<size_type> offsets;
      sqeazy::halo<Neighborhood, size_type> geometry(_shape.begin(), _shape.end());
      geometry.compute_offsets_in_x(offsets);

      size_type halo_size_x = length - offsets[row_major::x];

      //no offsets in other dimensions than x
      if(offsets.size()!=1)
      {
        halo_size_x = geometry.non_halo_end(row_major::x) - geometry.non_halo_begin(row_major::x) + 1;
      }


      unsigned n_neighbors_below_threshold = 0;

      const float cut_fraction = fraction*(size<Neighborhood>()-1);

      const int nthreads = this->n_threads();
      const omp_size_type offset_size = offsets.size();
      auto offsets_begin = offsets.begin();
	  auto local_shape = _shape;
	  const auto local_threshold = threshold;

#pragma omp parallel for                  \
  shared(_output)                                                          \
  firstprivate( _input, offsets_begin, offset_size, local_threshold, local_shape , n_neighbors_below_threshold, cut_fraction) \
  num_threads(nthreads)
      for(omp_size_type offset=0; offset<offset_size; ++offset) {

        for(std::size_t index = 0; index < (std::size_t)halo_size_x; ++index) {

          const size_type local_index = index + *(offsets_begin + offset);

          //skip pixels that are below threshold
          if(*(_input + local_index)<local_threshold)
            continue;

          n_neighbors_below_threshold = count_neighbors_if<Neighborhood>(_input + local_index,
                                                                         local_shape,
                                                                         // std::bind2nd(std::less<raw_type>(), threshold)
                                                                         [&](raw_type element){
                                                                           return element < local_threshold;
                                                                         }
            );

          if(n_neighbors_below_threshold>cut_fraction){
            _output[local_index] = 0;
          }
          else
            _output[local_index] = _input[local_index];

        }
      }


      return _output+length;

    }

    int decode( const compressed_type* _input, raw_type* _output,
                const std::vector<std::size_t>& _shape,
                std::vector<std::size_t>) const override final {

      std::size_t total_size = std::accumulate(_shape.begin(), _shape.end(), 1, std::multiplies<std::size_t>());

      if(_input!=_output ){

        if(this->n_threads() == 1)
          std::copy(_input, _input + total_size, _output);
        else{
          const int nthreads = this->n_threads();
          omp_size_type len = total_size;

          #pragma omp parallel for                  \
            shared(_output)                                             \
            firstprivate( _input )                                      \
            num_threads(nthreads)
          for(omp_size_type i = 0;i<len;++i){
            _output[i] = _input[i];
          }
        }
      }
      return 0;

    }


    ~flatten_to_neighborhood_scheme(){};

    std::string output_type() const final override {

      return sqeazy::header_utils::represent<compressed_type>::as_string();

    }

    bool is_compressor() const final override {

      return base_type::is_compressor;

    }



  };

}

#endif /* _FLATTEN_TO_NEIGHBORHOOD_SCHEME_IMPL_H_ */
