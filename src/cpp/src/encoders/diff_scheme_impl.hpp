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
    std::string config() const {

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
    compressed_type* encode( const raw_type* _in,
			     compressed_type* _out,
			     const std::vector<std::size_t>& _shape) override final {

      typedef std::size_t size_type;

      if(_shape.size()!=3){
	std::cerr << "[diff_scheme] unable to process input data that is not 3D\n";
	return _out;
      }
      
      std::size_t length = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());
      std::copy(_in, _in + length, _out);//crossing fingers due to possible type mismatch

      std::vector<size_type> offsets;
      sqeazy::halo<Neighborhood, size_type> geometry(_shape[row_major::w],
						     _shape[row_major::h],
						     _shape[row_major::d]);
      geometry.compute_offsets_in_x(offsets);


      size_type halo_size_x = geometry.non_halo_end(0)-geometry.non_halo_begin(0);
      if(offsets.size()==1)//no offsets in other dimensions than x
        {
	  halo_size_x = length - offsets[0];
        }
      sum_type local_sum = 0;
      size_type local_index = 0;
      const sum_type n_traversed_pixels = sqeazy::num_traversed_pixels<Neighborhood>();

      out_type* signed_out = reinterpret_cast<out_type*>(_out);
	
      typename std::vector<size_type>::const_iterator offsetsItr = offsets.begin();
      for(; offsetsItr!=offsets.end(); ++offsetsItr) {
	for(unsigned long index = 0; index < (unsigned long)halo_size_x; ++index) {

	  local_index = index + *offsetsItr;
	  local_sum = naive_sum<Neighborhood>(_in,local_index,
					      _shape[row_major::w],
					      _shape[row_major::h],
					      _shape[row_major::d]);
	  signed_out[local_index] = _in[local_index] - local_sum/n_traversed_pixels;

	}
      }

      return _out+length;
      
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
      typename std::vector<size_type>::const_iterator offsetsItr = offsets.begin();

      size_type halo_size_x = geometry.non_halo_end(0)-geometry.non_halo_begin(0);
      if(offsets.size()==1)//no offsets in other dimensions than x
        {
	  halo_size_x = length - offsets[0];
        }
      sum_type local_sum = 0;
      const sum_type n_traversed_pixels = sqeazy::num_traversed_pixels<Neighborhood>();
      const out_type* signed_in = reinterpret_cast<const out_type*>(_in);
      
      for(; offsetsItr!=offsets.end(); ++offsetsItr) {
	for(unsigned long index = 0; index < (unsigned long)halo_size_x; ++index) {

	  const size_type local_index = index + *offsetsItr;
	  local_sum = naive_sum<Neighborhood>(_out,local_index,
					      _shape[row_major::w],
					      _shape[row_major::h],
					      _shape[row_major::d]);
	  _out[local_index] = signed_in[local_index] + local_sum/n_traversed_pixels;

	}
      }

      return SUCCESS;
      
    }
    
    

    ~diff_scheme(){};

    std::string output_type() const final override {

      return typeid(compressed_type).name();
    
    }

    bool is_compressor() const final override {
    
      return base_type::is_compressor;
    
    }









    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // DEPRECATED API

    
    typedef typename add_unsigned<typename twice_as_wide<in_type>::type >::type sum_type;
    static const bool is_sink = false;

    /**
     * @brief producing the name of this scheme and return it as a string
     *
     * @return const std::string
     */
    static const std::string static_name() {

      //TODO: add name of Neighborhood
      std::ostringstream msg;
      msg << "diff"
	  << Neighborhood::x_offset_end - Neighborhood::x_offset_begin << "x"
	  << Neighborhood::y_offset_end - Neighborhood::y_offset_begin << "x"
	  << Neighborhood::z_offset_end - Neighborhood::z_offset_begin ;
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

      std::vector<size_type> offsets;
      sqeazy::halo<Neighborhood, size_type> geometry(_width,_height,_depth);
      geometry.compute_offsets_in_x(offsets);


      size_type halo_size_x = geometry.non_halo_end(0)-geometry.non_halo_begin(0);
      if(offsets.size()==1)//no offsets in other dimensions than x
        {
	  halo_size_x = length - offsets[0];
        }
      sum_type local_sum = 0;
      size_type local_index = 0;
      const sum_type n_traversed_pixels = sqeazy::num_traversed_pixels<Neighborhood>();

      typename std::vector<size_type>::const_iterator offsetsItr = offsets.begin();
      for(; offsetsItr!=offsets.end(); ++offsetsItr) {
	for(unsigned long index = 0; index < (unsigned long)halo_size_x; ++index) {

	  local_index = index + *offsetsItr;
	  local_sum = naive_sum<Neighborhood>(_input,local_index,_width, _height,_depth);
	  _output[local_index] = _input[local_index] - local_sum/n_traversed_pixels;

	}
      }

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

      std::vector<size_type> offsets;
      sqeazy::halo<Neighborhood, size_type> geometry(_width,_height,_depth);
      geometry.compute_offsets_in_x(offsets);
      typename std::vector<size_type>::const_iterator offsetsItr = offsets.begin();

      size_type halo_size_x = geometry.non_halo_end(0)-geometry.non_halo_begin(0);
      if(offsets.size()==1)//no offsets in other dimensions than x
        {
	  halo_size_x = length - offsets[0];
        }
      sum_type local_sum = 0;
      const sum_type n_traversed_pixels = sqeazy::num_traversed_pixels<Neighborhood>();

      for(; offsetsItr!=offsets.end(); ++offsetsItr) {
	for(unsigned long index = 0; index < (unsigned long)halo_size_x; ++index) {

	  const size_type local_index = index + *offsetsItr;
	  local_sum = naive_sum<Neighborhood>(_output,local_index,_width, _height,_depth);
	  _output[local_index] = _input[local_index] + local_sum/n_traversed_pixels;

	}
      }

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

  };

}

#endif /* _DIFF_SCHEME_IMPL_H_ */
