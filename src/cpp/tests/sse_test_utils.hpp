#ifndef _SEE_UTILS_H_
#define _SEE_UTILS_H_


#include <xmmintrin.h>
#include <smmintrin.h>


template <unsigned num_bits_, typename T>
struct set_bits
{

  static const void inside(T& _dest, const T& _src, const unsigned& _at){
    throw std::runtime_error("set_bits]\tno implemented yet");
  }

};

template <unsigned num_bits_>
struct set_bits<num_bits_,unsigned short>
{
  typedef __m128i vec_type;


  
  static const void inside(vec_type& _dest, const vec_type& _src, const unsigned& _at){

    static const vec_type v_num_bits = _mm_set1_epi16((1 << num_bits_) - 1);

    // T where_to_insert = (( 1 << numbits )-1)<<at;
    vec_type where_to_insert = _mm_slli_epi16(v_num_bits,_at);

    // T dest_or_where = where_to_insert|destination;
    _dest = _mm_or_si128(where_to_insert, _dest);

    // T src_prep_for_xor = (~source<<at) & where_to_insert;
    where_to_insert = _mm_and_si128(//left-shift
				    _mm_slli_epi16(//negate _src
						   _mm_xor_si128(_src,
								 // generate 0xFFFFFFFF
								 _mm_cmpeq_epi32(
										 _src,_src
										 )
								 ),
						   _at), 
				    where_to_insert);

    // T value = (dest_or_where)^(src_prep_for_xor);
    _dest = _mm_xor_si128(_dest, where_to_insert);
    
    return;
  }

};



template <const unsigned size = (32*(1 << 10)), unsigned value = 0xff,typename T = unsigned short>
struct const_anyvalue_fixture{     
  std::vector<T> input; 
  std::vector<T> output; 
  std::vector<T> reference;

  const_anyvalue_fixture():
    input(size,0),
    output(size,0),
    reference(size,0){
    

    std::fill(input.begin(),input.end(), (T)value);
  

  }
};

template <const unsigned size = (32*(1 << 10)),
	    typename T = unsigned short>
struct ramp_fixture{     
  std::vector<T> input; 
  std::vector<T> output; 
  std::vector<T> calc_first_16_hand;
  std::vector<T> reference;

  ramp_fixture():
    input(size,0),
    output(size,0),
    calc_first_16_hand(16,0),
    reference(size,0){
    
    for (unsigned i = 0; i < input.size(); ++i)
      {
	input[i] = i;
      }
    
    calc_first_16_hand[11] = 0xff;  // =   255,        11111111
    calc_first_16_hand[12] = 0xf0f; // =  3855,    111100001111
    calc_first_16_hand[13] = 0x3333;// = 13107,  11001100110011 (2nd to last bit)
    calc_first_16_hand[14] = 0x5555;// = 21845, 101010101010101 (last bit)

    

  }
};


#endif /* _SEE_UTILS_H_ */
