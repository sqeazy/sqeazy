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


		const int chunk = tile_size;
		shape_ptr_t shape = _shape.data();
		shape_ptr_t full_tiles = n_full_tiles.data();
		auto len = std::distance(_begin,_end);


#pragma omp parallel for									\
  shared( _begin, _out )										\
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

		typedef decltype(_shape.data()) shape_ptr_t;

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


		const int chunk = tile_size;
		shape_ptr_t shape = _shape.data();
		shape_ptr_t ptiles_per_dim = tiles_per_dim.data();
		const std::array<std::size_t, 3>* ptile_shapes = tile_shapes.data();
		const std::size_t* ptile_output_offsets = tile_output_offsets.data();
		auto len = std::distance(_begin,_end);

#pragma omp parallel for									\
  shared( _begin, _out )										\
  schedule(static,chunk)									\
  firstprivate(shape,ptiles_per_dim, ptile_shapes)			\
  num_threads(_nthreads)
		for(shape_value_t z = 0;z<shape[row_major::z];++z){

		  std::size_t ztile = z / tile_size;
		  std::size_t z_intile_row_offset = z % tile_size;

		  for(shape_value_t y = 0;y<shape[row_major::y];++y){

			std::size_t ytile = y / tile_size;
			std::size_t y_intile_row_offset = y % tile_size;

			std::size_t xtile = 0;

			std::size_t in_row = (z*shape[row_major::y]*shape[row_major::x]) + (y*shape[row_major::x]);
			std::size_t tile_index = (ztile*ptiles_per_dim[row_major::x]*ptiles_per_dim[row_major::y] + ytile*ptiles_per_dim[row_major::x]);


			for(shape_value_t x = 0;x<shape[row_major::x];x+=tile_size,++xtile,++tile_index){

			  std::size_t intile_row_offset = (z_intile_row_offset*ptile_shapes[tile_index][row_major::x]*ptile_shapes[tile_index][row_major::y]) + y_intile_row_offset*ptile_shapes[tile_index][row_major::x];
			  std::size_t tile_output_offset = ptile_output_offsets[tile_index];

			  std::copy(_begin + in_row + x,
						_begin + in_row + x + tile_shapes[tile_index][row_major::x],
						_out + tile_output_offset + intile_row_offset
				);

			}

		  }

		}


		return _out + len;
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


		static_assert(sizeof(in_value_t) == sizeof(out_value_t), "[sqeazy::detail::reorder::encode] reorder received non-matching types");

		if(_shape.size()!=3){
		  std::cerr << "[sqeazy::detail::reorder::encode] received non-3D shape which is currently unsupported!\n";
		  return _out;
		}

		std::size_t n_elements = std::distance(_begin,_end);
		std::size_t n_elements_from_shape = std::accumulate(_shape.begin(), _shape.end(),
															1,
															std::multiplies<std::size_t>());
		if(n_elements_from_shape != n_elements){
		  std::cerr << "[sqeazy::detail::reorder::encode] input iterator range does not match shape in 1D size!\n";
		  return _out;
		}

		const shape_container_t rem = remainder(_shape);

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

		typedef typename std::iterator_traits<in_iterator_t>::value_type in_value_type;
		typedef typename std::remove_cv<in_value_type>::type in_value_t;

		typedef typename std::iterator_traits<decltype(_shape.begin())>::value_type shape_value_type;
		typedef typename std::remove_cv<shape_value_type>::type shape_value_t;
		typedef decltype(_shape.data()) shape_ptr_t;

		if(_nthreads <= 0)
		  _nthreads = std::thread::hardware_concurrency();

		if(_shape[row_major::z] < (shape_value_t)_nthreads)
		  _nthreads = _shape[row_major::z];


		const shape_container_t rem = remainder(_shape);
		const bool has_remainder = std::count_if(rem.begin(), rem.end(), [](shape_value_t el){ return el > 0;});
		const std::size_t n_elements = std::distance(_begin,_end);

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


		if(has_remainder){
		  auto current_tile_shape = tile_shapes.begin();
		  auto current_tile_sizes = tile_sizes.begin()  ;

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

		std::vector<std::size_t> tile_size_sums(tile_sizes.size(),0);
		std::size_t running_sum = 0;
		for(std::size_t i = 0;i<tile_size_sums.size();++i){
		  tile_size_sums[i] = running_sum;
		  running_sum += tile_sizes[i];
		}

		shape_value_t ztile = 0;
		shape_value_t ytile = 0;
		shape_value_t xtile = 0;


		std::vector<std::array<std::size_t, 3> > tile_spatial_offsets(n_tiles, tile_shape);
		auto linear_offsets_begin = tile_spatial_offsets.begin();
		std::array<std::size_t, 3> loc;

		for(shape_value_t z = 0;z<_shape[row_major::z];z+=tile_size){
		  for(shape_value_t y = 0;y<_shape[row_major::y];y+=tile_size){
			for(shape_value_t x = 0;x<_shape[row_major::x];x+=tile_size){
			  loc[row_major::x] = x;
			  loc[row_major::y] = y;
			  loc[row_major::z] = z;
			  *linear_offsets_begin = loc;//copy-by-assignment
			  ++linear_offsets_begin;
			}
		  }
		}


		std::vector<in_value_t> buffer_row(_shape[row_major::x],0);
		std::vector<std::vector<in_value_t>> buffer_row_per_thread(_nthreads, buffer_row);
		auto pbuffer_row_per_thread = buffer_row_per_thread.data();

		std::size_t write_count = 0;
		const std::array<std::size_t, 3>* ptile_shapes = tile_shapes.data();

		shape_ptr_t pshape = _shape.data();
		const std::size_t* ptile_size_sums = tile_size_sums.data();
		shape_value_t n_rows_in_output = _shape[row_major::z]*_shape[row_major::y];

#pragma omp parallel for												\
  shared( _begin, _out, write_count )									\
  firstprivate(pshape,ptile_shapes,  ptile_size_sums, pbuffer_row_per_thread) \
  num_threads(_nthreads)
		for(std::size_t row_in_output = 0;row_in_output<n_rows_in_output;++row_in_output)
		{

		  int tid = omp_get_thread_num();
		  auto buffer_row_begin = pbuffer_row_per_thread[tid].begin();
		  auto row_global_offset = row_in_output*pshape[row_major::x];

		  std::size_t global_y = row_in_output % pshape[row_major::y];
		  std::size_t global_z = row_in_output / pshape[row_major::y];

		  std::size_t tile_y = global_y / tile_size;
		  std::size_t tile_z = global_z / tile_size;

		  std::size_t in_tile_y = global_y % tile_size;
		  std::size_t in_tile_z = global_z % tile_size;


		  std::size_t tile_offset = tile_z*tiles_per_dim[row_major::x]*tiles_per_dim[row_major::y];
		  tile_offset += tile_y*tiles_per_dim[row_major::x];

		  for(std::size_t tile = tile_offset;tile<(tile_offset+tiles_per_dim[row_major::x]);++tile)
		  {


			const std::size_t tile_extent_x = ptile_shapes[tile][row_major::x];
			const std::size_t tile_extent_y = ptile_shapes[tile][row_major::y];
			std::size_t in_tile_offset = in_tile_z*tile_extent_y*tile_extent_x + in_tile_y*tile_extent_x;

			auto src = _begin + ptile_size_sums[tile] + in_tile_offset;

			buffer_row_begin = std::copy(src, src + tile_extent_x,
										 buffer_row_begin);

		  }

		  std::copy(pbuffer_row_per_thread[tid].begin(),
					pbuffer_row_per_thread[tid].end(),
					_out + row_global_offset);
#pragma omp critical
		  {
			write_count += pshape[row_major::x];
		  }
		}


		if(write_count == n_elements)
		  return _out + write_count;
		else
		  return _out;

	  }


	};

  };

};

#endif /* _MEMORY_REORDER_UTILS_H_ */
