#ifndef _SSE_BITSHUFFLE_H_
#define _SSE_BITSHUFFLE_H_

namespace sqeazy {

  namespace detail {

        template <std::size_t N, typename iterator_t>
    void to_block_range(const std::bitset<N>& _src,
			iterator_t _begin,
			iterator_t _end){
  
      static const std::size_t ulong_bytes = sizeof(unsigned long);  
      static const std::size_t ulong_bits = ulong_bytes*CHAR_BIT;
      static const std::size_t n_chunks = (N + ulong_bits - 1)/ulong_bits;

      typedef typename std::remove_reference<decltype(*_begin)>::type value_t;
      
      // const std::size_t size = _end - _begin;
      // const std::size_t size_bytes = size*sizeof(*_begin);
      // const std::size_t size_bits = size_bytes*CHAR_BIT;
      
      auto dest = reinterpret_cast<value_t*>(&*_begin);
      unsigned long val = 0;
      
      for(std::size_t c = 0;c < n_chunks;++c){
	int right_shift = (n_chunks - 1 - c )*ulong_bits;
	auto temp = _src >> right_shift;
	temp &= ~((temp >> ulong_bits) << ulong_bits);
	val = temp.to_ulong();

	value_t* vbegin = reinterpret_cast<value_t*>(&val);
	value_t* vend = vbegin + (sizeof(val)/sizeof(value_t));
	std::reverse_iterator<value_t*> r(dest + ((c+1)*sizeof(val)/sizeof(value_t)));
	std::copy(vbegin,
		  vend,
		  r);
    
      }
    
  
    }

        template<typename in_type, std::uint32_t n_bits_per_segment = 1>
    struct bitshuffle
    {

      typedef typename std::make_unsigned<in_type>::type type;
      
      static const int type_width = sizeof(type)*CHAR_BIT;
      static const int simd_width = 128;
      static const int simd_width_bytes = simd_width/CHAR_BIT;
      static const int n_elements_per_simd = simd_width/type_width;
      static const int n_segments = type_width/n_bits_per_segment;
      static const int n_elements = simd_width_bytes*n_segments/sizeof(type);
      
      static_assert(n_bits_per_segment <= type_width,
		    "sqeazy::detail::bitshuffle received more n_bits_per_segment that given type yields");
      
      typedef typename std::bitset<simd_width> bitset_t;
      
      std::array<bitset_t, n_segments> segments;
      std::uint32_t n_bits_consumed;

      bitshuffle():
	segments(),
	n_bits_consumed(0){

	
      }


      bool empty() const {
	return n_bits_consumed == 0;
      }

      bool full() const {
	return n_bits_consumed == simd_width;
      }

      std::size_t size() const {
	return simd_width*n_segments;
      }

      std::size_t size_in_bytes() const {
	return simd_width*n_segments/CHAR_BIT;
      }

      
      std::size_t num_elements() const {
	return size_in_bytes()/sizeof(type);
      }
      
      void set(std::size_t seg_id, std::size_t bit_id = 0)  {

	if(seg_id>=segments.size())
	  return;

	if(bit_id>=segments[0].size())
	  return;

	segments[seg_id].set(bit_id);
	n_bits_consumed += 1;
      }
      
      void reset()  {
	for( auto & seg : segments ){
	  seg.reset();
	}
	n_bits_consumed = 0;;
      }
      
      bool any() const {
	bool value = false;
	for(auto & bits : segments )
	  value = value || bits.any() ;
	
	return value;
      }

      /**
	 \brief collect msb values from block in chunks of type, the resulting bitset contains the aquired values starting at the MSB

	 \param[in] 

	 \return 
	 \retval 

      */
      bitset_t gather_msb_range(__m128i block, int n_bits){

	bitset_t value;

	if(n_bits == 1){
	  gather_msb<type> op;
	  const auto val = op(block);
	  value = bitset_t(val);
	  value <<= (simd_width - (sizeof(val)*CHAR_BIT));
	}
	
	return value;
      }

      template <typename iterator_t>
      iterator_t consume(iterator_t _begin, iterator_t _end){

	static_assert(sizeof(decltype(*_begin)) == sizeof(type), "received iterator type does not match assumed type");

	std::size_t size = _end - _begin;
	
	if(size*sizeof(type) < simd_width_bytes)
	  return _begin;

	
	iterator_t value = _begin;
	const int step_size = simd_width/type_width;
	std::uint32_t segment_counter = 0;
	
	__m128i current;
	shift_left_m128i<type> left_shifter;
	bitset_t extracted;
	
	for(;value!=_end;value+=step_size, ++segment_counter){

	  if(full())
	    return value;
	  
	  current = _mm_load_si128(reinterpret_cast<const __m128i*>(&*value));

	  for(int s = 0;s<n_segments;++s){

	    //extract msb(s)
	    extracted = gather_msb_range(current,n_bits_per_segment);

	    //update segment
	    segments[s] |= (extracted >> (segment_counter*n_elements_per_simd));
	    
	    //left shift to bring the next segment to the MSB
	    current = left_shifter(current,n_bits_per_segment);
	  }

	  n_bits_consumed += (n_bits_per_segment*n_elements_per_simd);
	  
	}
		
	return value;
      }

      template <typename iterator_t>
      iterator_t write_segments(iterator_t _begin, iterator_t _end,
				std::size_t offset = 0){

	iterator_t value = _begin;
	std::size_t size = _end - _begin;


	if(size % segments.size() > n_elements_per_simd)
	  return value;

	std::size_t min_elements_required = segments[0].size()*segments.size()/(sizeof(*_begin)*CHAR_BIT);
	
	if(size < min_elements_required)
	  return value;


	std::size_t segment_spacing =  size / segments.size() ;

	if(segment_spacing < n_elements_per_simd)
	  return value;
	
	std::size_t global_offset = 0;
	
	for( std::uint32_t s = 0;s < segments.size();++s){
	  global_offset = s*segment_spacing;
	  value = _begin + global_offset + offset;
	  
	  to_block_range(segments[s],
			 value,
			 value+n_elements_per_simd);
	  
	}
	
	return _end;
      }

      
    };



    
  }
}

#endif /* _BITSHUFFLE_H_ */
