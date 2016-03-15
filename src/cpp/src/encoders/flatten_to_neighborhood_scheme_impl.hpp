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

    raw_type threshold;
    float fraction;

    flatten_to_neighborhood_scheme(raw_type _threshold, float _frac = percentage_below/100.f):
      threshold(_threshold),
      fraction(_frac){
    }

    
    flatten_to_neighborhood_scheme(const std::string& _payload=""):
      fraction(percentage_below/100.f){

      auto config_map = parse_string_by(_payload);

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
      msg << "threshold=" << threshold << ",";
      msg << "fraction=" << fraction;
      return msg.str();
    
    }

    std::intmax_t max_encoded_size(std::intmax_t _size_bytes) const override final {
    
      return _size_bytes;
    }

    compressed_type* encode( const raw_type* _input, compressed_type* _output, std::vector<std::size_t> _shape) override final {

      typedef std::size_t size_type;
        unsigned long length = std::accumulate(_shape.begin(), _shape.end(), 1, std::multiplies<size_type>());

        std::vector<size_type> offsets;
        sqeazy::halo<Neighborhood, size_type> geometry(_shape.begin(), _shape.end());
        geometry.compute_offsets_in_x(offsets);

        size_type halo_size_x = length - offsets[0];

        //no offsets in other dimensions than x
        if(offsets.size()!=1)
        {
            halo_size_x = geometry.non_halo_end(0) - geometry.non_halo_begin(0) + 1;
        }

        unsigned long local_index=0;
        unsigned n_neighbors_below_threshold = 0;
        typename std::vector<size_type>::const_iterator offsetsItr = offsets.begin();

#ifdef _SQY_VERBOSE_
	unsigned long num_pixels_discarded=0;
#endif

        const float cut_fraction = fraction*(size<Neighborhood>()-1);
        for(; offsetsItr!=offsets.end(); ++offsetsItr) {
	  for(unsigned long index = 0; index < (unsigned long)halo_size_x; ++index) {

                local_index = index + *offsetsItr;

		//skip pixels that are below threshold
		if(*(_input + local_index)<threshold)
		  continue;

                n_neighbors_below_threshold = count_neighbors_if<Neighborhood>(_input + local_index,
                                              _shape,
                                              std::bind2nd(std::less<raw_type>(), threshold)
                                                                              );
                if(n_neighbors_below_threshold>cut_fraction){
#ifdef _SQY_VERBOSE_
		  num_pixels_discarded++;
#endif
		  _output[local_index] = 0;
		}
                else
                    _output[local_index] = _input[local_index];

            }
        }
#ifdef _SQY_VERBOSE_
	int prec = std::cout.precision();
	std::cout.precision(3);
	std::cout << "[SQY_VERBOSE] flatten_to_neighborhood " << num_pixels_discarded << " / " << length << " ("<< 100*double(num_pixels_discarded)/length <<" %) discarded due to neighborhood\n";
	std::cout.precision(prec);
#endif


	return _output+length;
      
    }

    int decode( const compressed_type* _input, raw_type* _output, std::vector<std::size_t> _shape) const override final {

      return 1;
    }

    
    ~flatten_to_neighborhood_scheme(){};

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

        std::ostringstream msg;
        msg << "rmbkrd_neighbor"
            << Neighborhood::x_offset_end - Neighborhood::x_offset_begin << "x"
            << Neighborhood::y_offset_end - Neighborhood::y_offset_begin << "x"
            << Neighborhood::z_offset_end - Neighborhood::z_offset_begin ;

        return msg.str();

    }


    template <typename size_type>
    /**
     * @brief encoding the flatten_to_neighborhood scheme
     *
     * @param _input Input 3D image stack of raw_type
     * @param _output Output 3D image stack of raw_type
     * @param _dims std::vector<int> that contains the dimensionality of _input and _output
     * @param _threshold the threshold under which a pixel is considered noise
     * @param _frac_neighb_to_null fraction of neighboring pixels (exclusive neighborhood) that decided
     * if the central pixel is to be set to 0 (count > _frac_neighb_to_null) or kept as is (count <= _frac_neighb_to_null)

     * @return sqeazy::error_code
     */
    static const error_code static_encode(raw_type* _input,
                                   raw_type* _output,
                                   const std::vector<size_type>& _dims,
                                   const raw_type& _threshold,
                                   float _frac_neighb_to_null = percentage_below/100.f)
    {


        unsigned long length = std::accumulate(_dims.begin(), _dims.end(), 1, std::multiplies<size_type>());

        std::vector<size_type> offsets;
        sqeazy::halo<Neighborhood, size_type> geometry(_dims.begin(), _dims.end());
        geometry.compute_offsets_in_x(offsets);

        size_type halo_size_x = length - offsets[0];

        //no offsets in other dimensions than x
        if(offsets.size()!=1)
        {
            halo_size_x = geometry.non_halo_end(0) - geometry.non_halo_begin(0) + 1;
        }

        unsigned long local_index=0;
        unsigned n_neighbors_below_threshold = 0;
        typename std::vector<size_type>::const_iterator offsetsItr = offsets.begin();

#ifdef _SQY_VERBOSE_
	unsigned long num_pixels_discarded=0;
#endif

        const float cut_fraction = _frac_neighb_to_null*(size<Neighborhood>()-1);
        for(; offsetsItr!=offsets.end(); ++offsetsItr) {
	  for(unsigned long index = 0; index < (unsigned long)halo_size_x; ++index) {

                local_index = index + *offsetsItr;

		//skip pixels that are below threshold
		if(*(_input + local_index)<_threshold)
		  continue;

                n_neighbors_below_threshold = count_neighbors_if<Neighborhood>(_input + local_index,
                                              _dims,
                                              std::bind2nd(std::less<raw_type>(), _threshold)
                                                                              );
                if(n_neighbors_below_threshold>cut_fraction){
#ifdef _SQY_VERBOSE_
		  num_pixels_discarded++;
#endif
		  _output[local_index] = 0;
		}
                else
                    _output[local_index] = _input[local_index];

            }
        }
#ifdef _SQY_VERBOSE_
	int prec = std::cout.precision();
	std::cout.precision(3);
	std::cout << "[SQY_VERBOSE] flatten_to_neighborhood " << num_pixels_discarded << " / " << length << " ("<< 100*double(num_pixels_discarded)/length <<" %) discarded due to neighborhood\n";
	std::cout.precision(prec);
#endif
        return SUCCESS;
    }


    template <typename SizeType>
    /**
     * @brief decoding the _input to _output given just the length of the buffers as scalar
     * here: not operation except copying is performed
     *
     * @return sqeazy::error_code
     */
    static const error_code static_decode(const raw_type* _input,
                                   raw_type* _output,
                                   const SizeType& _length)
    {
        if(_input!=_output )
            std::copy(_input, _input + _length, _output);
        return SUCCESS;
    }

    template <typename SizeType>
    /**
     * @brief decoding the _input to _output given just the length of the buffers as vector (dimensionality)
     * here: not operation except copying is performed
     *
     *
     * @return sqeazy::error_code
     */
    static const error_code static_decode(const raw_type* _input,
                                   raw_type* _output,
                                   const std::vector<SizeType>& _length)
    {
        unsigned long total_size = std::accumulate(_length.begin(), _length.end(), 1, std::multiplies<SizeType>());

        return static_decode(_input, _output, total_size);
    }




  };

}

#endif /* _FLATTEN_TO_NEIGHBORHOOD_SCHEME_IMPL_H_ */
