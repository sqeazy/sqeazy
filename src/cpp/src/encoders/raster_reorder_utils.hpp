#ifndef _MEMORY_REORDER_UTILS_H_
#define _MEMORY_REORDER_UTILS_H_

#include <cstdint>
#include <iterator>

#include "traits.hpp"

#include <xmmintrin.h>
#include <smmintrin.h>


namespace sqeazy {

  namespace detail {


	struct reorder {

	  std::size_t tile_size;

	  reorder(std::size_t _tsize):
		tile_size(_tsize){

	  }

	  template <typename value_t>
	  std::vector<value_t> remainder(const std::vector<value_t>& _shape) const {
		std::vector<value_t> rem = _shape;
		for(value_t & el : rem)
		  el = el % tile_size;

		return rem;
	  }

	  template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
	  out_iterator_t encode(in_iterator_t _begin,
							in_iterator_t _end,
							out_iterator_t _out,
							const shape_container_t& _shape,
							int nthreads = 1
		) const {

		typedef typename std::iterator_traits<in_iterator_t>::value_type in_value_type;
		typedef typename std::remove_cv<in_value_type>::type in_value_t;

		typedef typename std::iterator_traits<out_iterator_t>::value_type out_value_type;
		typedef typename std::remove_cv<out_value_type>::type out_value_t;

		typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

		static_assert(sizeof(in_value_t) == sizeof(out_value_t), "[sqeazy::detail::reorder::encode] reorder received non-matching types");

		if(_shape.size()!=3){
		  std::cerr << "[sqeazy::detail::reorder::encode] received non-3D shape which is currently unsupported!\n";
		  return _out;
		}

		std::size_t n_elements = _end - _begin;
		std::size_t n_elements_from_shape = std::accumulate(_shape.begin(), _shape.end(),
															1,
															std::multiplies<std::size_t>());
		if(n_elements_from_shape != n_elements){
		  std::cerr << "[sqeazy::detail::reorder::encode] input iterator range does not match shape in 1D size!\n";
		  return _out;
		}

		const shape_container_t rem = remainder(_shape);
		const bool has_remainder = std::count_if(rem.begin(), rem.end(), [](shape_value_t el){ return el > 0;});

		if(has_remainder)
		  return encode_with_remainder(_begin,_end,_out,_shape);
		else{
		  static const std::size_t n_elements_per_simd_block = 16/sizeof(in_value_t);

		  if(tile_size % n_elements_per_simd_block == 0)
			return encode_full_simd(_begin,_end,_out,_shape,nthreads);
		  else
			return encode_full(_begin,_end,_out,_shape,nthreads);
		}

	  }


	  template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
	  out_iterator_t encode_full(in_iterator_t _begin,
								 in_iterator_t _end,
								 out_iterator_t _out,
								 const shape_container_t& _shape,
								 int _nthreads = 1) const {

		typedef decltype(_shape.data()) shape_ptr_t;
		typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

		if(_nthreads <= 0)
		  _nthreads = std::thread::hardware_concurrency();

		if(_shape[row_major::z] < (shape_value_t)_nthreads)
		  _nthreads = _shape[row_major::z];

		const shape_container_t rem = remainder(_shape);
		const std::size_t n_elements_per_tile = std::pow(tile_size,_shape.size());
		const std::size_t n_elements_per_tile_frame = std::pow(tile_size,_shape.size()-1);

		shape_container_t n_full_tiles = _shape;

		for(shape_value_t & n_tiles : n_full_tiles )
		  n_tiles = n_tiles / tile_size;

		// std::size_t in_row  = 0;
		// std::size_t out_tile_offset = 0;

		// std::size_t ztile = 0;
		// std::size_t ytile = 0;
		// std::size_t xtile = 0;

		// std::size_t z_intile_row_offset = 0;
		// std::size_t y_intile_row_offset = 0;

		// std::size_t intile_row_offset = 0;

		// in_iterator_t src = _begin;
		// out_iterator_t dst = _out;
		const int chunk = tile_size;
		shape_ptr_t shape = _shape.data();
		shape_ptr_t full_tiles = n_full_tiles.data();
		auto len = std::distance(_begin,_end);
		out_iterator_t dst = _out;

#pragma omp parallel for									\
  shared( _begin, dst )										\
  schedule(static,chunk)									\
  firstprivate(shape,full_tiles)							\
  num_threads(_nthreads)
		for(shape_value_t z = 0;z<shape[row_major::z];++z){
		  std::size_t ztile = z / tile_size;
		  std::size_t z_intile_row_offset = z % tile_size;

		  for(shape_value_t y = 0;y<shape[row_major::y];++y){
			std::size_t ytile = y / tile_size;
			std::size_t y_intile_row_offset = y % tile_size;

			std::size_t xtile = 0;

			std::size_t in_row = (z*shape[row_major::y]*shape[row_major::x]) + (y*shape[row_major::x]);
			std::size_t out_tile_offset = (ztile*full_tiles[row_major::x]*full_tiles[row_major::y] + ytile*full_tiles[row_major::x]);
			std::size_t intile_row_offset = z_intile_row_offset*n_elements_per_tile_frame + y_intile_row_offset*tile_size;

			for(shape_value_t x = 0;x<shape[row_major::x];x+=tile_size){

			  xtile = x / tile_size;
			  in_iterator_t  src = _begin + in_row + x;
			  out_iterator_t dst = _out + ((out_tile_offset+xtile)*n_elements_per_tile) + intile_row_offset;

			  dst = std::copy(src,
							  src + tile_size,
							  dst
				);

			}

		  }

		}

		return _out + len;

	  }

	  template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
	  out_iterator_t encode_full_simd(in_iterator_t _begin,
									  in_iterator_t _end,
									  out_iterator_t _out,
									  const shape_container_t& _shape,
									  int _nthreads = 1) const {

		typedef decltype(_shape.data()) shape_ptr_t;
		typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

		typedef typename std::iterator_traits<in_iterator_t>::value_type in_value_type;
		typedef typename std::remove_cv<in_value_type>::type in_value_t;

		static const std::size_t n_elements_per_simd_block = 16/sizeof(in_value_t);

		if(tile_size % n_elements_per_simd_block != 0){
		  std::cerr << "[sqeazy::detail::reorder::encode_full_simd] given tile_size ("<< tile_size
					<<" elements) is incompatible with simd_width ("<< n_elements_per_simd_block
					<<")!\n";
		  return _out;
		}

		if(_nthreads <= 0)
		  _nthreads = std::thread::hardware_concurrency();

		if(_shape[row_major::z] < (shape_value_t)_nthreads)
		  _nthreads = _shape[row_major::z];

		const shape_container_t rem = remainder(_shape);
		const std::size_t n_elements_per_tile = std::pow(tile_size,_shape.size());
		const std::size_t n_elements_per_tile_frame = std::pow(tile_size,_shape.size()-1);

		shape_container_t n_full_tiles = _shape;

		for(shape_value_t & n_tiles : n_full_tiles )
		  n_tiles = n_tiles / tile_size;

		// std::size_t in_row  = 0;
		// std::size_t out_tile_offset = 0;

		// std::size_t ztile = 0;
		// std::size_t ytile = 0;
		// std::size_t xtile = 0;

		// std::size_t z_intile_row_offset = 0;
		// std::size_t y_intile_row_offset = 0;

		// std::size_t intile_row_offset = 0;

		// __m128i block;

		shape_ptr_t shape = _shape.data();
		shape_ptr_t full_tiles = n_full_tiles.data();
		auto len = std::distance(_begin,_end);
		out_iterator_t dst = _out;
		const int chunk = tile_size;

#pragma omp parallel for									\
  shared( _begin, dst )										\
  firstprivate(shape,full_tiles)							\
  schedule(static,chunk)									\
  num_threads(_nthreads)
		for(shape_value_t z = 0;z<shape[row_major::z];++z){
		  std::size_t ztile = z / tile_size;
		  std::size_t z_intile_row_offset = z % tile_size;

		  for(shape_value_t y = 0;y<shape[row_major::y];++y){
			std::size_t ytile = y / tile_size;
			std::size_t y_intile_row_offset = y % tile_size;

			std::size_t xtile = 0;

			std::size_t in_row = (z*shape[row_major::y]*shape[row_major::x]) + (y*shape[row_major::x]);
			std::size_t out_tile_offset = (ztile*full_tiles[row_major::x]*full_tiles[row_major::y] + ytile*full_tiles[row_major::x]);

			std::size_t intile_row_offset = z_intile_row_offset*n_elements_per_tile_frame + y_intile_row_offset*tile_size;

			for(shape_value_t x = 0;x<shape[row_major::x];x+=n_elements_per_simd_block){

			  xtile = x / tile_size;

			  const in_value_t* in_address = &*(_begin + in_row + x);
			  dst = _out + ((out_tile_offset+xtile)*n_elements_per_tile) + intile_row_offset;
			  in_value_t* out_address = &*dst;

			  __m128i block = _mm_load_si128(reinterpret_cast<const __m128i*>(in_address));
			  _mm_store_si128(reinterpret_cast<__m128i*>(out_address),block);
			  dst += n_elements_per_simd_block;

			}

		  }

		}

		return _out + len;

	  }


	  template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
	  out_iterator_t encode_with_remainder(in_iterator_t _begin,
										   in_iterator_t _end,
										   out_iterator_t _out,
										   const shape_container_t& _shape,
										   int _nthreads = 1) const {


		typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

		if(_nthreads <= 0)
		  _nthreads = std::thread::hardware_concurrency();

		if(_shape[row_major::z] < (shape_value_t)_nthreads)
		  _nthreads = _shape[row_major::z];

		const shape_container_t rem = remainder(_shape);
		const bool has_remainder = std::count_if(rem.begin(), rem.end(), [](shape_value_t el){ return el > 0;});

		shape_container_t full_tiles_per_dim = _shape;
		for(shape_value_t & tile : full_tiles_per_dim )
		  tile = tile / tile_size;

		shape_container_t tiles_per_dim = _shape;
		for(shape_value_t & tile : tiles_per_dim )
		  tile = (tile + tile_size - 1)/ tile_size;

		const std::size_t n_tiles = std::accumulate(tiles_per_dim.begin(), tiles_per_dim.end(),
													1,
													std::multiplies<std::size_t>());

		std::array<std::size_t, 3> tile_shape; tile_shape.fill(tile_size);

		std::vector<std::array<std::size_t, 3> > tile_shapes (n_tiles, tile_shape);
		std::vector<std::size_t>		 tile_sizes  (n_tiles, std::pow(tile_size,_shape.size()));


		if(has_remainder){
		  auto current_tile_shape = tile_shapes.begin();
		  auto current_tile_sizes = tile_sizes.begin() ;

		  for(shape_value_t z = 0;z<tiles_per_dim[row_major::z];++z){
			tile_shape[row_major::z] = z != tiles_per_dim[row_major::z]-1 ? tile_size : rem[row_major::z];

			for(shape_value_t y = 0;y<tiles_per_dim[row_major::y];++y){
			  tile_shape[row_major::y] = y != tiles_per_dim[row_major::y]-1 ? tile_size : rem[row_major::y];

			  for(shape_value_t x = 0;x<tiles_per_dim[row_major::x];++x){
				tile_shape[row_major::x] = x != tiles_per_dim[row_major::x]-1 ? tile_size : rem[row_major::x];

				*current_tile_shape = tile_shape;
				++current_tile_shape;

				*current_tile_sizes = std::accumulate(tile_shape.begin(), tile_shape.end(),1,std::multiplies<std::size_t>());
				++current_tile_sizes;


			  }
			}
		  }


		}

		std::vector<std::size_t> tile_output_offsets(n_tiles, 0);
		std::size_t sum = 0;
		for(std::size_t i = 0;i<tile_sizes.size();++i){
		  tile_output_offsets[i] = sum;
		  sum += tile_sizes[i];
		}

		std::size_t in_row  = 0;
		std::size_t tile_index = 0;
		std::size_t tile_output_offset = 0;

		std::size_t ztile = 0;
		std::size_t ytile = 0;
		std::size_t xtile = 0;

		std::size_t z_intile_row_offset = 0;
		std::size_t y_intile_row_offset = 0;

		std::size_t intile_row_offset = 0;

		out_iterator_t dst = _out;
		const int chunk = tile_size;

#pragma omp parallel for									\
  shared( _begin, dst )									\
  schedule(static,chunk)				\
  num_threads(_nthreads)
		for(shape_value_t z = 0;z<_shape[row_major::z];++z){

		  ztile = z / tile_size;
		  z_intile_row_offset = z % tile_size;

		  for(shape_value_t y = 0;y<_shape[row_major::y];++y){

			ytile = y / tile_size;
			y_intile_row_offset = y % tile_size;


			xtile = 0;

			in_row = (z*_shape[row_major::y]*_shape[row_major::x]) + (y*_shape[row_major::x]);
			tile_index = (ztile*tiles_per_dim[row_major::x]*tiles_per_dim[row_major::y] + ytile*tiles_per_dim[row_major::x]);


			for(shape_value_t x = 0;x<_shape[row_major::x];x+=tile_size,++xtile,++tile_index){

			  intile_row_offset = (z_intile_row_offset*tile_shapes[tile_index][row_major::x]*tile_shapes[tile_index][row_major::y]) + y_intile_row_offset*tile_shapes[tile_index][row_major::x];
			  tile_output_offset = tile_output_offsets[tile_index];

			  dst = std::copy(_begin + in_row + x,
							  _begin + in_row + x + tile_shapes[tile_index][row_major::x],
							  _out + tile_output_offset + intile_row_offset
				);

			}

		  }

		}


		return dst;
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

		// typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		// typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

		static_assert(sizeof(in_value_t) == sizeof(out_value_t), "[sqeazy::detail::reorder::encode] reorder received non-matching types");

		if(_shape.size()!=3){
		  std::cerr << "[sqeazy::detail::reorder::encode] received non-3D shape which is currently unsupported!\n";
		  return _out;
		}

		std::size_t n_elements = _end - _begin;
		std::size_t n_elements_from_shape = std::accumulate(_shape.begin(), _shape.end(),
															1,
															std::multiplies<std::size_t>());
		if(n_elements_from_shape != n_elements){
		  std::cerr << "[sqeazy::detail::reorder::encode] input iterator range does not match shape in 1D size!\n";
		  return _out;
		}

		const shape_container_t rem = remainder(_shape);
		// const bool has_remainder = std::count_if(rem.begin(), rem.end(), [](shape_value_t el){ return el > 0;});

		//return encode(_begin, _end, _out,_shape);
		return decode_with_remainder(_begin, _end, _out,_shape);

	  }

	  template <typename in_iterator_t, typename out_iterator_t, typename shape_container_t>
	  out_iterator_t decode_with_remainder(in_iterator_t _begin,
										   in_iterator_t _end,
										   out_iterator_t _out,
										   const shape_container_t& _shape,
										   int _nthreads = 1) const {


		typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		typedef typename std::remove_cv<shape_value_type>::type shape_value_t;

		typedef typename std::iterator_traits<in_iterator_t>::value_type in_value_type;
		typedef typename std::remove_cv<in_value_type>::type in_value_t;

		const shape_container_t rem = remainder(_shape);
		const bool has_remainder = std::count_if(rem.begin(), rem.end(), [](shape_value_t el){ return el > 0;});

		shape_container_t full_tiles_per_dim = _shape;
		for(shape_value_t & tile : full_tiles_per_dim )
		  tile = tile / tile_size;

		shape_container_t tiles_per_dim = _shape;
		for(shape_value_t & tile : tiles_per_dim )
		  tile = (tile + tile_size - 1)/ tile_size;

		const std::size_t n_tiles = std::accumulate(tiles_per_dim.begin(), tiles_per_dim.end(),
													1,
													std::multiplies<std::size_t>());

		std::array<std::size_t, 3> tile_shape; tile_shape.fill(tile_size);
		std::vector<std::array<std::size_t, 3> > tile_shapes(n_tiles, tile_shape);
		std::vector<std::size_t>		 tile_sizes  (n_tiles, std::pow(tile_size,_shape.size()));

		const std::size_t max_n_rows_per_tile = std::pow(tile_size,_shape.size()-1);
		std::vector<std::size_t>		 tile_n_rows (n_tiles, max_n_rows_per_tile);


		if(has_remainder){
		  auto current_tile_shape = tile_shapes.begin();
		  auto current_tile_sizes = tile_sizes.begin()  ;
		  auto current_tile_n_rows = tile_n_rows.begin()  ;

		  for(shape_value_t z = 0;z<tiles_per_dim[row_major::z];++z){
			tile_shape[row_major::z] = z != tiles_per_dim[row_major::z]-1 ? tile_size : rem[row_major::z];

			for(shape_value_t y = 0;y<tiles_per_dim[row_major::y];++y){
			  tile_shape[row_major::y] = y != tiles_per_dim[row_major::y]-1 ? tile_size : rem[row_major::y];

			  for(shape_value_t x = 0;x<tiles_per_dim[row_major::x];++x){
				tile_shape[row_major::x] = x != tiles_per_dim[row_major::x]-1 ? tile_size : rem[row_major::x];

				*current_tile_shape = tile_shape;
				++current_tile_shape;

				*current_tile_sizes = std::accumulate(tile_shape.begin(), tile_shape.end(),1,std::multiplies<std::size_t>());
				++current_tile_sizes;

				*current_tile_n_rows = std::accumulate(tile_shape.begin(), tile_shape.end(),1,std::multiplies<std::size_t>())/tile_shape[row_major::x];
				++current_tile_n_rows;
			  }
			}
		  }
		}

		std::vector<std::size_t> tile_size_sums(tile_sizes.size(),0);
		std::size_t running_sum = 0;
		for(std::size_t i = 0;i<tile_size_sums.size();++i){
		  tile_size_sums[i] = running_sum;
		  running_sum += tile_sizes[i];
		}

		shape_value_t ztile = 0;
		shape_value_t ytile = 0;
		shape_value_t xtile = 0;

		const std::size_t output_frame_size = _shape[row_major::x]*_shape[row_major::y];


		std::vector<std::array<std::size_t, 3> > tile_offsets(n_tiles, tile_shape);
		auto linear_offsets_begin = tile_offsets.begin();
		std::array<std::size_t, 3> loc;

		for(shape_value_t z = 0;z<_shape[row_major::z];z+=tile_size){
		  for(shape_value_t y = 0;y<_shape[row_major::y];y+=tile_size){
			for(shape_value_t x = 0;x<_shape[row_major::x];x+=tile_size){
			  loc[row_major::x] = x;
			  loc[row_major::y] = y;
			  loc[row_major::z] = z;
			  *linear_offsets_begin = loc;
			  ++linear_offsets_begin;
			}
		  }
		}

		auto dst = _out;
		shape_value_t z_in_tile = 0;

		shape_value_t y_in_tile = 0;
		shape_value_t output_offset = 0;

		std::vector<in_value_t> buffer_row(_shape[row_major::x],0);
		auto buffer_row_begin = buffer_row.begin();
		in_iterator_t src = _begin;

		std::array<std::size_t, 3> dst_offset;dst_offset.fill(0);
		std::size_t dst_start_tile = 0;
		std::size_t write_count = 0;

		for(std::size_t row_in_tile = 0;row_in_tile<max_n_rows_per_tile;++row_in_tile){


		  for(std::size_t tile = 0;tile<n_tiles;++tile){

			src = _begin + tile_size_sums[tile];

			if(!(row_in_tile<tile_n_rows[tile]))
			  continue;

			src += row_in_tile*tile_shapes[tile][row_major::x];

			buffer_row_begin = std::copy(src, src + tile_shapes[tile][row_major::x],
										 buffer_row_begin);

			if(buffer_row_begin == buffer_row.end()){

			  dst_offset = tile_offsets[tile];

			  z_in_tile = row_in_tile / tile_shapes[tile][row_major::y];
			  y_in_tile = row_in_tile % tile_shapes[tile][row_major::y];

			  dst_start_tile = (dst_offset[row_major::z]+z_in_tile)*output_frame_size;
			  dst_start_tile += (dst_offset[row_major::y]+y_in_tile)*_shape[row_major::x]  ;
			  dst_start_tile += dst_offset[row_major::x] + tile_shapes[tile][row_major::x] ;
			  dst_start_tile -= buffer_row.size();

			  dst = _out + dst_start_tile ;

			  dst = std::copy(buffer_row.begin(),
							  buffer_row.end(),
							  dst);
			  write_count += buffer_row.size();
			  buffer_row_begin = buffer_row.begin();

			}

		  }
		}

		const std::size_t n_elements = (_end - _begin);
		if(write_count == n_elements)
		  return _out + write_count;
		else
		  return _out;

	  }


	};

  };

};

#endif /* _MEMORY_REORDER_UTILS_H_ */
