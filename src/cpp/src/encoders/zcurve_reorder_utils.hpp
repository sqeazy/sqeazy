#ifndef _ZCURVE_REORDER_UTILS_H_
#define _ZCURVE_REORDER_UTILS_H_

#include <cstdint>
#include <iterator>

#include "sqeazy_algorithms.hpp"
#include "traits.hpp"
#include "morton.hpp"

namespace sqeazy {

  namespace detail {


	struct zcurve {

	  std::size_t tile_size;

	  typedef std::uint64_t (*encode_function_t)(std::uint32_t ,std::uint32_t ,std::uint32_t );

	  typedef void (*decode_function_t)(std::uint64_t,std::uint32_t&,std::uint32_t&,std::uint32_t&);

	  // std::function<std::uint64_t(std::uint32_t,std::uint32_t,std::uint32_t)> zcurve_encode;
	  // std::function<void(std::uint64_t,std::uint32_t&,std::uint32_t&,std::uint32_t&)>  zcurve_decode;

	  encode_function_t zcurve_encode;
	  decode_function_t zcurve_decode;

	  zcurve(std::size_t _tsize):
		tile_size(_tsize),
		zcurve_encode(&detail::morton_at_ct<>::from),
		zcurve_decode(&detail::morton_at_ct<>::to)
		{
		  switch(tile_size){
		  case 2:
			zcurve_encode = &detail::morton_at_ct<>::from;
			zcurve_decode = &detail::morton_at_ct<>::to;
			break;
		  case 4:
			zcurve_encode = &detail::morton_at_ct<2>::from;
			zcurve_decode = &detail::morton_at_ct<2>::to;
			break;
		  case 8:
			zcurve_encode = &detail::morton_at_ct<3>::from;
			zcurve_decode = &detail::morton_at_ct<3>::to;
			break;
		  case 16:
			zcurve_encode = &detail::morton_at_ct<4>::from;
			zcurve_decode = &detail::morton_at_ct<4>::to;
			break;
		  case 32:
			zcurve_encode = &detail::morton_at_ct<5>::from;
			zcurve_decode = &detail::morton_at_ct<5>::to;
			break;
		  case 64:
			zcurve_encode = &detail::morton_at_ct<6>::from;
			zcurve_decode = &detail::morton_at_ct<6>::to;
			break;
		  case 128:
			zcurve_encode = &detail::morton_at_ct<7>::from;
			zcurve_decode = &detail::morton_at_ct<7>::to;
			break;
		  default:
			zcurve_encode = &detail::morton_at_ct<>::from;
			zcurve_decode = &detail::morton_at_ct<>::to;
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
							const shape_container_t& _shape,
							int _nthreads = 1) const {

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

		if(_nthreads <= 0)
		  _nthreads = std::thread::hardware_concurrency();

		if(_shape[row_major::z] < (shape_value_t)_nthreads)
		  _nthreads = _shape[row_major::z];

		if(has_remainder)
		  return encode_with_remainder(_begin,_end,_out,_shape, _nthreads);
		else{
		  return encode_full(_begin,_end,_out,_shape, _nthreads);
		}

	  }


	  template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
	  out_iterator_t encode_full(in_iterator_t _begin,
								 in_iterator_t _end,
								 out_iterator_t _out,
								 const shape_container_t& _shape,
								 int _nthreads = 1) const {

		typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

		const std::size_t n_elements_per_tile = std::pow(tile_size,_shape.size());
		const std::size_t n_elements = std::distance(_begin, _end);

		shape_container_t shape_in_tiles = _shape;

		for(shape_value_t & tile_dim : shape_in_tiles )
		  tile_dim = tile_dim / tile_size;

		const auto pshape = _shape.data();
		const auto pshape_in_tiles = shape_in_tiles.data();
		int chunk =  (pshape[row_major::z] + _nthreads - 1)/_nthreads;

		#pragma omp parallel for \
		  shared (_out) \
		  firstprivate ( _begin, pshape, pshape_in_tiles ) \
		  schedule (static, chunk)
		for(omp_size_type z = 0;z<pshape[row_major::z];++z){
		  std::uint32_t tile_offset_in_z = z / tile_size;

		  for(shape_value_t y = 0;y<pshape[row_major::y];++y){
			std::uint32_t tile_offset_in_y = y / tile_size;

			for(shape_value_t x = 0;x<pshape[row_major::x];++x){

			  std::uint32_t tile_offset_in_x = x / tile_size;
			  std::uint32_t tile_index = tile_offset_in_z*pshape_in_tiles[row_major::x]*pshape_in_tiles[row_major::y] + tile_offset_in_y*pshape_in_tiles[row_major::x] + tile_offset_in_x;

			  shape_value_t src_offset = z*pshape[row_major::y]*pshape[row_major::x] + y*pshape[row_major::x] + x;

			  auto dst = _out + (tile_index*n_elements_per_tile);

			  std::size_t dst_offset = zcurve_encode(z % tile_size,
													 y % tile_size,
													 x % tile_size);


			  auto dst_ptr = dst + dst_offset;
			  *dst_ptr = *(_begin + src_offset);

			}

		  }

		}

		return _out + n_elements;

	  }

	  template <typename shape_container_t>
	  std::vector<std::array<std::size_t, 3> > generate_tile_shapes(const shape_container_t& _shape) const {

		typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

		const shape_container_t rem = remainder(_shape);
		const bool has_remainder = std::count_if(rem.begin(), rem.end(), [](shape_value_t el){ return el > 0;});


		shape_container_t tiles_per_dim = _shape;
		for(shape_value_t & tile : tiles_per_dim )
		  tile = (tile + tile_size - 1)/ tile_size;

		const std::size_t n_tiles = std::accumulate(tiles_per_dim.begin(), tiles_per_dim.end(),
													1,
													std::multiplies<std::size_t>());

		std::array<std::size_t, 3> full_tile_shape; full_tile_shape.fill(tile_size);
		std::vector<std::array<std::size_t, 3> > value (n_tiles, full_tile_shape);

		if(has_remainder){
		  auto current_tile_shape = value.begin();


		  for(shape_value_t z = 0;z<_shape[row_major::z];z+=tile_size){
			full_tile_shape[row_major::z] = ((_shape[row_major::z] - z ) < tile_size) ? (_shape[row_major::z] - z) : tile_size;

			for(shape_value_t y = 0;y<_shape[row_major::y];y+=tile_size){
			  full_tile_shape[row_major::y] = ((_shape[row_major::y] - y) < tile_size) ? (_shape[row_major::y] - y) : tile_size;

			  for(shape_value_t x = 0;x<_shape[row_major::x];x+=tile_size){
				full_tile_shape[row_major::x] = ((_shape[row_major::x] - x) < tile_size) ? (_shape[row_major::x] - x) : tile_size;

				*current_tile_shape = full_tile_shape;
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
										   const shape_container_t& _shape,
										   int _nthreads = 1) const {


		typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

		const std::size_t n_elements = std::distance(_begin,_end);

		shape_container_t tiles_per_dim = _shape;
		for(shape_value_t & tile : tiles_per_dim )
		  tile = (tile + tile_size - 1)/ tile_size;

		std::array<std::size_t, 3> full_tile_shape;full_tile_shape.fill(tile_size);

		std::vector<std::array<std::size_t, 3> > tile_shapes = generate_tile_shapes(_shape);
		const std::size_t n_tiles = tile_shapes.size();

		std::vector<std::size_t> tile_output_offsets(n_tiles, 0);

		//prefix sum
		const auto ptile_shapes = tile_shapes.data();
		const auto ptile_output_offsets = tile_output_offsets.data();

		prefix_sum_of(ptile_shapes, ptile_shapes+n_tiles, ptile_output_offsets,
					  [](const std::array<std::size_t, 3>& _lshape){
						return std::accumulate(_lshape.begin(), _lshape.end(),1,std::multiplies<std::size_t>());
					  },
					  _nthreads);

		const auto pshape = _shape.data();
		const auto ptiles_per_dim = tiles_per_dim.data();
		const auto cptile_output_offsets = tile_output_offsets.data();

		#pragma omp parallel for \
		  shared (_out) \
		  firstprivate ( _begin, pshape, cptile_output_offsets, ptiles_per_dim )	\
		  schedule (static)
		for(omp_size_type z = 0;z<pshape[row_major::z];++z){

		  const std::size_t   ztile = z / tile_size;
		  const std::uint32_t z_intile = z % tile_size;

		  for(shape_value_t y = 0;y<pshape[row_major::y];++y){

			const std::size_t   ytile = y / tile_size;
			const std::uint32_t y_intile = y % tile_size;

			std::size_t xtile = 0;
			const std::size_t tile_index = (ztile*ptiles_per_dim[row_major::x]*ptiles_per_dim[row_major::y] + ytile*ptiles_per_dim[row_major::x]);

			for(shape_value_t x = 0;x<pshape[row_major::x];++x){

			  xtile = x / tile_size;
			  const std::uint32_t x_intile = x % tile_size;

			  std::size_t tile_output_offset = cptile_output_offsets[tile_index+xtile];
			  const auto current_shape = ptile_shapes[tile_index+xtile];

			  if(std::equal(current_shape.begin(),
							current_shape.end(),
							full_tile_shape.begin())){
				tile_output_offset += zcurve_encode(z_intile, y_intile, x_intile);
			  }
			  else{
				tile_output_offset += z_intile*current_shape[row_major::x]*current_shape[row_major::y] + y_intile*current_shape[row_major::x] + x_intile;
			  }

			  const shape_value_t src_offset = z*pshape[row_major::y]*pshape[row_major::x] + y*pshape[row_major::x] + x;
			  *(_out + tile_output_offset) = *(_begin + src_offset);

			}

		  }

		}


		return _out + n_elements;
	  }


	  template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
	  out_iterator_t decode(in_iterator_t _begin,
							in_iterator_t _end,
							out_iterator_t _out,
							const shape_container_t& _shape,
							int _nthreads = 1) const {

		typedef typename std::iterator_traits<in_iterator_t>::value_type in_value_type;
		typedef typename std::remove_cv<in_value_type>::type in_value_t;

		typedef typename std::iterator_traits<out_iterator_t>::value_type out_value_type;
		typedef typename std::remove_cv<out_value_type>::type out_value_t;

		typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

		static_assert(sizeof(in_value_t) == sizeof(out_value_t), "[sqeazy::detail::zcurve::encode] zcurve received non-matching types");

		if(_shape.size()!=3){
		  std::cerr << "[sqeazy::detail::zcurve::decode] received non-3D shape which is currently unsupported!\n";
		  return _out;
		}

		std::size_t n_elements = _end - _begin;
		std::size_t n_elements_from_shape = std::accumulate(_shape.begin(), _shape.end(),
															1,
															std::multiplies<std::size_t>());
		if(_nthreads <= 0)
		  _nthreads = std::thread::hardware_concurrency();

		if(_shape[row_major::z] < (shape_value_t)_nthreads)
		  _nthreads = _shape[row_major::z];

		if(n_elements_from_shape != n_elements){
		  std::cerr << "[sqeazy::detail::zcurve::decode] input iterator range does not match shape in 1D size!\n";
		  return _out;
		}

		return decode_with_remainder(_begin, _end, _out,_shape, _nthreads);

	  }


	  template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
	  out_iterator_t decode_with_remainder(in_iterator_t _begin,
										   in_iterator_t _end,
										   out_iterator_t _out,
										   const shape_container_t& _shape,
										   int _nthreads = 1) const {


		typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

		const std::size_t n_elements = std::distance(_begin, _end);

		shape_container_t shape_in_tiles = _shape;
		for(shape_value_t & tile_dim : shape_in_tiles )
		  tile_dim = (tile_dim + tile_size - 1)/ tile_size;

		std::array<std::size_t, 3> full_tile_shape; full_tile_shape.fill(tile_size);

		std::vector<std::array<std::size_t, 3> > tile_shapes = generate_tile_shapes(_shape);
		const std::size_t n_tiles = tile_shapes.size();

		std::vector<std::size_t> tile_output_offsets(n_tiles, 0);

		const auto ptile_shapes = tile_shapes.data();
		const auto ptile_output_offsets = tile_output_offsets.data();

		prefix_sum_of(ptile_shapes, ptile_shapes+n_tiles, ptile_output_offsets,
					  [](const std::array<std::size_t, 3>& _lshape){
						return std::accumulate(_lshape.begin(), _lshape.end(),1,std::multiplies<std::size_t>());
					  },
					  _nthreads);

		const auto pshape = _shape.data();
		const auto pshape_in_tiles = shape_in_tiles.data();
		const auto cptile_output_offsets = tile_output_offsets.data();

#pragma omp parallel for						\
		  shared (_out) \
		  firstprivate ( _begin, pshape, cptile_output_offsets,pshape_in_tiles )	\
		  schedule (static)
		for(omp_size_type z = 0;z<pshape[row_major::z];++z){

		  std::size_t ztile = z / tile_size;
		  std::size_t z_intile = z % tile_size;

		  for(shape_value_t y = 0;y<pshape[row_major::y];++y){

			std::size_t ytile = y / tile_size;
			std::size_t y_intile = y % tile_size;

			std::size_t tile_index = (ztile*pshape_in_tiles[row_major::x]*pshape_in_tiles[row_major::y] + ytile*pshape_in_tiles[row_major::x]);

			for(shape_value_t x = 0;x<pshape[row_major::x];++x){
			  std::size_t xtile = x / tile_size;
			  std::size_t x_intile = x % tile_size;

			  std::size_t tile_output_offset = cptile_output_offsets[tile_index+xtile];
			  auto current_shape = ptile_shapes[tile_index+xtile];
			  if(std::equal(ptile_shapes[tile_index+xtile].begin(),
							ptile_shapes[tile_index+xtile].end(),
							full_tile_shape.begin())){
				tile_output_offset += zcurve_encode(z_intile, y_intile, x_intile);
			  }
			  else{
				tile_output_offset += z_intile*current_shape[row_major::x]*current_shape[row_major::y] + y_intile*current_shape[row_major::x] + x_intile;}

			  const shape_value_t dst_offset = z*pshape[row_major::y]*pshape[row_major::x] + y*pshape[row_major::x] + x;

			  *(_out + dst_offset) = *(_begin + tile_output_offset);

			}

		  }

		}


		return _out+n_elements;
	  }


	};

  };

};

#endif /* _ZCURVE_REORDER_UTILS_H_ */
