#ifndef _ZCURVE_REORDER_UTILS_H_
#define _ZCURVE_REORDER_UTILS_H_

#include <cstdint>
#include <iterator>

#include "traits.hpp"
#include "morton.hpp"

namespace sqeazy {

  namespace detail {


	struct zcurve {

	  std::size_t tile_size;
	  std::function<std::uint64_t(std::uint32_t,std::uint32_t,std::uint32_t)> zcurve_encode;
	  std::function<void(std::uint64_t,std::uint32_t&,std::uint32_t&,std::uint32_t&)>  zcurve_decode;

	  zcurve(std::size_t _tsize):
		tile_size(_tsize)
		{
		  switch(tile_size){
		  case 2:
			zcurve_encode = detail::morton_at_ct<>::from;
			zcurve_decode = detail::morton_at_ct<>::to;
			break;
		  case 4:
			zcurve_encode = detail::morton_at_ct<2>::from;
			zcurve_decode = detail::morton_at_ct<2>::to;
			break;
		  case 8:
			zcurve_encode = detail::morton_at_ct<3>::from;
			zcurve_decode = detail::morton_at_ct<3>::to;
			break;
		  case 16:
			zcurve_encode = detail::morton_at_ct<4>::from;
			zcurve_decode = detail::morton_at_ct<4>::to;
			break;
		  case 32:
			zcurve_encode = detail::morton_at_ct<5>::from;
			zcurve_decode = detail::morton_at_ct<5>::to;
			break;
		  case 64:
			zcurve_encode = detail::morton_at_ct<6>::from;
			zcurve_decode = detail::morton_at_ct<6>::to;
			break;
		  case 128:
			zcurve_encode = detail::morton_at_ct<7>::from;
			zcurve_decode = detail::morton_at_ct<7>::to;
			break;
		  default:
			zcurve_encode = detail::morton_at_ct<>::from;
			zcurve_decode = detail::morton_at_ct<>::to;
		  };

		}

	  template <typename value_t>
	  std::size_t common_power_of_2(const std::vector<value_t>& _shape) const {
		std::vector<std::size_t> safe_shape = _shape;

		for(std::size_t & el : safe_shape){
		  int lsb = 0;
		  for(;lsb<64 && el!=0;++lsb,el >>= 1){
		  }
		  el = lsb-1;
		}

		std::size_t common_pow2_size = 1 << (*std::min_element(safe_shape.begin(), safe_shape.end()));
		return common_pow2_size;
	  }

	  template <typename value_t>
	  std::vector<value_t> remainder(const std::vector<value_t>& _shape) const {

		auto common = common_power_of_2(_shape);
		std::vector<value_t> rem = _shape;

		for(std::size_t & el : rem){
		  el = el % common;
		}

		return rem;
	  }

	  template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
	  out_iterator_t encode(in_iterator_t _begin,
							in_iterator_t _end,
							out_iterator_t _out,
							const shape_container_t& _shape) const {

		typedef typename std::iterator_traits<in_iterator_t>::value_type in_value_type;
		typedef typename std::remove_cv<in_value_type>::type in_value_t;

		typedef typename std::iterator_traits<out_iterator_t>::value_type out_value_type;
		typedef typename std::remove_cv<out_value_type>::type out_value_t;

		typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

		static_assert(sizeof(in_value_t) == sizeof(out_value_t), "[sqeazy::detail::zcurve::encode] zcurve received non-matching types");

		if(_shape.size()!=3){
		  std::cerr << "[sqeazy::detail::zcurve::encode] received non-3D shape which is currently unsupported!\n";
		  return _out;
		}

		std::size_t n_elements = _end - _begin;
		std::size_t n_elements_from_shape = std::accumulate(_shape.begin(), _shape.end(),
															1,
															std::multiplies<std::size_t>());
		if(n_elements_from_shape != n_elements){
		  std::cerr << "[sqeazy::detail::zcurve::encode] input iterator range does not match shape in 1D size!\n";
		  return _out;
		}

		const shape_container_t rem = remainder(_shape);
		const bool has_remainder = std::count_if(rem.begin(), rem.end(), [](shape_value_t el){ return el > 0;});

		if(has_remainder)
		  return encode_with_remainder(_begin,_end,_out,_shape);
		else{
		  return encode_full(_begin,_end,_out,_shape);
		}

	  }


	  template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
	  out_iterator_t encode_full(in_iterator_t _begin,
								 in_iterator_t _end,
								 out_iterator_t _out,
								 const shape_container_t& _shape) const {

		typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

		const auto common = common_power_of_2(_shape);

		const std::size_t n_elements_per_tile = std::pow(common,_shape.size());
		// const std::size_t n_elements_per_tile_frame = std::pow(common,_shape.size()-1);

		shape_container_t shape_in_tiles = _shape;

		for(shape_value_t & tile_dim : shape_itile_dim )
		  tile_dim = tile_dim / common;

		std::size_t dst_offset  = 0;
		in_iterator_t src = _begin;
		out_iterator_t dst = _out;
		std::uint32_t tile_z = 0;
		std::uint32_t tile_y = 0;
		std::uint32_t tile_x = 0;
		std::uint32_t tile_index = 0;

		for(shape_value_t z = 0;z<_shape[row_major::z];z+=1){
		  tile_z = z / common;

		  for(shape_value_t y = 0;y<_shape[row_major::y];y+=1){
			tile_y = y / common;

			for(shape_value_t x = 0;x<_shape[row_major::x];x+=1){
			  tile_x = x / common;
			  tile_index = tile_z*shape_in_tiles[row_major::x]*shape_in_tiles[row_major::y] + tile_y*shape_in_tiles[row_major::x] + tile_x;
			  dst = _out + (tile_index*n_elements_per_tile);
			  dst_offset = zcurve_encode(z % common, y % common, x % common);

			  auto dst_ptr = dst + dst_offset;
			  *dst_ptr = *(src++);

			}

		  }

		}

		return _out + (src - _begin);

	  }

	  template <typename shape_container_t>
	  std::vector<std::array<std::size_t, 3> > generate_tile_shapes(const shape_container_t& _shape) const {

		typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

		const auto common = common_power_of_2(_shape);
		const shape_container_t rem = remainder(_shape);
		const bool has_remainder = std::count_if(rem.begin(), rem.end(), [](shape_value_t el){ return el > 0;});


		shape_container_t tiles_per_dim = _shape;
		for(shape_value_t & tile : tiles_per_dim )
		  tile = (tile + common - 1)/ common;

		const std::size_t n_tiles = std::accumulate(tiles_per_dim.begin(), tiles_per_dim.end(),
													1,
													std::multiplies<std::size_t>());

		std::array<std::size_t, 3> tile_shape; tile_shape.fill(common);
		std::vector<std::array<std::size_t, 3> > value (n_tiles, tile_shape);

		if(has_remainder){
		  auto current_tile_shape = value.begin();


		  for(shape_value_t z = 0;z<_shape[row_major::z];z+=common){
			tile_shape[row_major::z] = ((_shape[row_major::z] - z ) < common) ? (_shape[row_major::z] - z) : common;

			for(shape_value_t y = 0;y<_shape[row_major::y];y+=common){
			  tile_shape[row_major::y] = ((_shape[row_major::y] - y) < common) ? (_shape[row_major::y] - y) : common;

			  for(shape_value_t x = 0;x<_shape[row_major::x];x+=common){
				tile_shape[row_major::x] = ((_shape[row_major::x] - x) < common) ? (_shape[row_major::x] - x) : common;

				*current_tile_shape = tile_shape;
				++current_tile_shape;



			  }
			}
		  }


		}

		return value;
	  }


	  template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
	  out_iterator_t encode_with_remainder(in_iterator_t _begin,
										   in_iterator_t _end,
										   out_iterator_t _out,
										   const shape_container_t& _shape) const {


		typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

		const auto common = common_power_of_2(_shape);

		shape_container_t tiles_per_dim = _shape;
		for(shape_value_t & tile : tiles_per_dim )
		  tile = (tile + common - 1)/ common;

		std::array<std::size_t, 3> tile_shape; tile_shape.fill(common);

		std::vector<std::array<std::size_t, 3> > tile_shapes = generate_tile_shapes(_shape);
		const std::size_t n_tiles = tile_shapes.size();

		std::vector<std::size_t> tile_output_offsets(n_tiles, 0);
		std::size_t sum = 0;
		for(std::size_t i = 0;i<n_tiles;++i){
		  tile_output_offsets[i] = sum;
		  sum += std::accumulate(tile_shapes[i].begin(), tile_shapes[i].end(),1,std::multiplies<std::size_t>());
		}


		std::size_t tile_index = 0;
		std::size_t tile_output_offset = 0;

		std::size_t ztile = 0;
		std::size_t ytile = 0;
		std::size_t xtile = 0;

		std::uint32_t z_intile = 0;
		std::uint32_t y_intile = 0;
		std::uint32_t x_intile = 0;


		in_iterator_t itr  = _begin;


		for(shape_value_t z = 0;z<_shape[row_major::z];++z){

		  ztile = z / common;
		  z_intile = z % common;

		  for(shape_value_t y = 0;y<_shape[row_major::y];++y){

			ytile = y / common;
			y_intile = y % common;


			xtile = 0;


			tile_index = (ztile*tiles_per_dim[row_major::x]*tiles_per_dim[row_major::y] + ytile*tiles_per_dim[row_major::x]);


			for(shape_value_t x = 0;x<_shape[row_major::x];++x){
			  xtile = x / common;
			  x_intile = x % common;

			  tile_output_offset = tile_output_offsets[tile_index+xtile];
			  auto current_shape = tile_shapes[tile_index+xtile];
			  if(std::equal(current_shape.begin(),
							current_shape.end(),
							tile_shape.begin()))
				tile_output_offset += zcurve_encode(z_intile, y_intile, x_intile);
			  else
				tile_output_offset += z_intile*current_shape[row_major::x]*current_shape[row_major::y] + y_intile*current_shape[row_major::x] + x_intile;

			  *(_out + tile_output_offset) = *(itr++);

			}

		  }

		}


		return _out + (itr - _begin);
	  }


	  template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
	  out_iterator_t decode(in_iterator_t _begin,
							in_iterator_t _end,
							out_iterator_t _out,
							const shape_container_t& _shape) const {

		typedef typename std::iterator_traits<in_iterator_t>::value_type in_value_type;
		typedef typename std::remove_cv<in_value_type>::type in_value_t;

		typedef typename std::iterator_traits<out_iterator_t>::value_type out_value_type;
		typedef typename std::remove_cv<out_value_type>::type out_value_t;

		// typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		// typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

		static_assert(sizeof(in_value_t) == sizeof(out_value_t), "[sqeazy::detail::zcurve::encode] zcurve received non-matching types");

		if(_shape.size()!=3){
		  std::cerr << "[sqeazy::detail::zcurve::encode] received non-3D shape which is currently unsupported!\n";
		  return _out;
		}

		std::size_t n_elements = _end - _begin;
		std::size_t n_elements_from_shape = std::accumulate(_shape.begin(), _shape.end(),
															1,
															std::multiplies<std::size_t>());
		if(n_elements_from_shape != n_elements){
		  std::cerr << "[sqeazy::detail::zcurve::encode] input iterator range does not match shape in 1D size!\n";
		  return _out;
		}

		return decode_with_remainder(_begin, _end, _out,_shape);

	  }


	  template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
	  out_iterator_t decode_with_remainder(in_iterator_t _begin,
										   in_iterator_t _end,
										   out_iterator_t _out,
										   const shape_container_t& _shape) const {


		typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

		const auto common = common_power_of_2(_shape);

		shape_container_t tiles_per_dim = _shape;
		for(shape_value_t & tile : tiles_per_dim )
		  tile = (tile + common - 1)/ common;

		std::array<std::size_t, 3> tile_shape; tile_shape.fill(common);

		std::vector<std::array<std::size_t, 3> > tile_shapes = generate_tile_shapes(_shape);
		const std::size_t n_tiles = tile_shapes.size();

		std::vector<std::size_t> tile_output_offsets(n_tiles, 0);
		std::size_t sum = 0;
		for(std::size_t i = 0;i<n_tiles;++i){
		  tile_output_offsets[i] = sum;
		  sum += std::accumulate(tile_shapes[i].begin(), tile_shapes[i].end(),1,std::multiplies<std::size_t>());
		}


		std::size_t tile_index = 0;
		std::size_t tile_output_offset = 0;

		std::size_t ztile = 0;
		std::size_t ytile = 0;
		std::size_t xtile = 0;

		std::size_t z_intile = 0;
		std::size_t y_intile = 0;
		std::size_t x_intile = 0;


		in_iterator_t itr  = _begin;
		out_iterator_t dst = _out;

		for(shape_value_t z = 0;z<_shape[row_major::z];++z){

		  ztile = z / common;
		  z_intile = z % common;

		  for(shape_value_t y = 0;y<_shape[row_major::y];++y){

			ytile = y / common;
			y_intile = y % common;


			xtile = 0;


			tile_index = (ztile*tiles_per_dim[row_major::x]*tiles_per_dim[row_major::y] + ytile*tiles_per_dim[row_major::x]);


			for(shape_value_t x = 0;x<_shape[row_major::x];++x){
			  xtile = x / common;
			  x_intile = x % common;

			  tile_output_offset = tile_output_offsets[tile_index+xtile];
			  auto current_shape = tile_shapes[tile_index+xtile];
			  if(std::equal(tile_shapes[tile_index+xtile].begin(),
							tile_shapes[tile_index+xtile].end(),
							tile_shape.begin()))
				tile_output_offset += zcurve_encode(z_intile, y_intile, x_intile);
			  else
				tile_output_offset += z_intile*current_shape[row_major::x]*current_shape[row_major::y] + y_intile*current_shape[row_major::x] + x_intile;

			  *(dst++) = *(itr + tile_output_offset);

			}

		  }

		}


		return dst;
	  }


	};

  };

};

#endif /* _ZCURVE_REORDER_UTILS_H_ */
