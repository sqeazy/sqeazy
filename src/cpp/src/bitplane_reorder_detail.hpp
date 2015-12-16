#ifndef _BITPLANE_REORDER_DETAIL_H_
#define _BITPLANE_REORDER_DETAIL_H_

#include <stdexcept>
#include <climits>

#include <xmmintrin.h>
#include <smmintrin.h>


template<typename T>
struct shift_left_m128i {
  __m128i operator()(const __m128i& _block, int num_bits) const {return _block;}
};

template<>
struct shift_left_m128i<unsigned short> {
  __m128i operator()(const __m128i& _block, int num_bits) const {
    return  _mm_slli_epi16(_block, num_bits);
  }
};

template<>
struct shift_left_m128i<short> {
  __m128i operator()(const __m128i& _block, int num_bits) const {
    return  _mm_slli_epi16(_block, num_bits);
  }
};

template<>
struct shift_left_m128i<unsigned int> {
  __m128i operator()(const __m128i& _block, int num_bits) const {
    return  _mm_slli_epi32(_block, num_bits);
  }
};

template<>
struct shift_left_m128i<int> {
  __m128i operator()(const __m128i& _block, int num_bits) const {
    return  _mm_slli_epi32(_block, num_bits);
  }
};

template<>
struct shift_left_m128i<unsigned long> {
  __m128i operator()(const __m128i& _block, long num_bits) const {
    return  _mm_slli_epi64(_block, num_bits);
  }
};

template<>
struct shift_left_m128i<long> {
  __m128i operator()(const __m128i& _block, long num_bits) const {
    return  _mm_slli_epi64(_block, num_bits);
  }
};


template<>
struct shift_left_m128i<unsigned char> {
  __m128i operator()(const __m128i& _block, int num_bits) const {

    //treat upper half
    __m128i first = _mm_cvtepu8_epi16(_block);
    first = _mm_slli_epi16(first, num_bits);
    //collapse to first half
    first = _mm_shuffle_epi8(first, _mm_set_epi8(15,13,11,9,7,5,3,1,14,12,10,8,6,4,2,0) );

    //treat lower half
#ifdef WIN32 
	__m128 casted_block = _mm_castsi128_ps(_block);
	__m128i second = _mm_castps_si128(_mm_movehl_ps(casted_block, casted_block));
#else
	__m128 casted_block = reinterpret_cast<__m128>(_block);
	__m128i second = reinterpret_cast<__m128i>(_mm_movehl_ps(casted_block, casted_block));
#endif

    
    second = _mm_cvtepu8_epi16(second);
    second = _mm_slli_epi16(second, num_bits);
    second = _mm_shuffle_epi8(second,  _mm_set_epi8(14,12,10,8,6,4,2,0,15,13,11,9,7,5,3,1) );
#ifdef WIN32 
	__m128i value = _mm_castpd_si128(_mm_move_sd(_mm_castsi128_pd(second), _mm_castsi128_pd(first)));
#else
	__m128i value = reinterpret_cast<__m128i>(_mm_move_sd(reinterpret_cast<__m128d>(second), reinterpret_cast<__m128d>(first)));
#endif
    return value;
  }
};

template<>
struct shift_left_m128i<char> {
  __m128i operator()(const __m128i& _block, int num_bits) const {
    
    shift_left_m128i<unsigned char> left_shifter = {};
    return left_shifter(_block,num_bits);
    
  }
};


template<typename T>
struct shift_right_m128i {
  __m128i operator()(const __m128i& _block, int num_bits) const {return _block;}
};

template<>
struct shift_right_m128i<unsigned short> {
  __m128i operator()(const __m128i& _block, int num_bits) const {
    return  _mm_srli_epi16(_block, num_bits);
  }
};

template<>
struct shift_right_m128i<short> {
  __m128i operator()(const __m128i& _block, int num_bits) const {
    return  _mm_srli_epi16(_block, num_bits);
  }
};

template<>
struct shift_right_m128i<unsigned int> {
  __m128i operator()(const __m128i& _block, int num_bits) const {
    return  _mm_srli_epi32(_block, num_bits);
  }
};

template<>
struct shift_right_m128i<int> {
  __m128i operator()(const __m128i& _block, int num_bits) const {
    return  _mm_srli_epi32(_block, num_bits);
  }
};

template<>
struct shift_right_m128i<unsigned long> {
  __m128i operator()(const __m128i& _block, long num_bits) const {
    return  _mm_srli_epi64(_block, num_bits);
  }
};

template<>
struct shift_right_m128i<long> {
  __m128i operator()(const __m128i& _block, long num_bits) const {
    return  _mm_srli_epi64(_block, num_bits);
  }
};


template<>
struct shift_right_m128i<unsigned char> {
  __m128i operator()(const __m128i& _block, int num_bits) const {

    //treat upper half
    __m128i first = _mm_cvtepu8_epi16(_block);
    first = _mm_srli_epi16(first, num_bits);
    //corlapse to first half
    first = _mm_shuffle_epi8(first, _mm_set_epi8(15,13,11,9,7,5,3,1,14,12,10,8,6,4,2,0) );

    //treat lower half
#ifdef WIN32 
	__m128 casted_block = _mm_castsi128_ps(_block);
	__m128i second = _mm_castps_si128(_mm_movehl_ps(casted_block, casted_block));
#else
    __m128 casted_block = reinterpret_cast<__m128>(_block);
    __m128i second = reinterpret_cast<__m128i>(_mm_movehl_ps(casted_block,casted_block));
#endif
    second = _mm_cvtepu8_epi16(second);
    second = _mm_srli_epi16(second, num_bits);
    second = _mm_shuffle_epi8(second,  _mm_set_epi8(14,12,10,8,6,4,2,0,15,13,11,9,7,5,3,1) );

#ifdef WIN32 
	__m128i value = _mm_castpd_si128(_mm_move_sd(_mm_castsi128_pd(second), _mm_castsi128_pd(first)));
#else
	__m128i value = reinterpret_cast<__m128i>(_mm_move_sd(reinterpret_cast<__m128d>(second), reinterpret_cast<__m128d>(first)));
#endif
	return value;
  }
};

template<>
struct shift_right_m128i<char> {
  __m128i operator()(const __m128i& _block, int num_bits) const {
    
    shift_right_m128i<unsigned char> left_shifter;
    return left_shifter(_block,num_bits);
    
  }
};

template <typename T>
struct vec_xor {

  typedef T value_type;

  void operator()(__m128i* _inout){
    throw std::runtime_error("vec_xor called with unknown type");
    return;
  }

};

template <>
struct vec_xor<unsigned short> {

  typedef unsigned short value_type;
  static const value_type mask = ~(value_type(1 << ((sizeof(value_type)*CHAR_BIT) - 1)));

  void operator()(__m128i* _inout){
    
    static const __m128i vec_mask = _mm_set1_epi16(mask);
    
    *_inout = _mm_xor_si128(*_inout,vec_mask);

    return;
  }

};

template <>
struct vec_xor< short> {

  typedef  short value_type;
  static const value_type mask = ~(value_type(1 << ((sizeof(value_type)*CHAR_BIT) - 1)));

  void operator()(__m128i* _inout){
    
    static const __m128i vec_mask = _mm_set1_epi16(mask);
    
    *_inout = _mm_xor_si128(*_inout,vec_mask);

    return;
  }

};

template <>
struct vec_xor<unsigned char> {

  typedef unsigned short value_type;
  static const value_type mask = ~(value_type(1 << ((sizeof(value_type)*CHAR_BIT) - 1)));

  void operator()(__m128i* _inout){
    
    static const __m128i vec_mask = _mm_set1_epi8(mask);
    
    *_inout = _mm_xor_si128(*_inout,vec_mask);

    return;
  }

};


template <>
struct vec_xor< char> {

  typedef  short value_type;
  static const value_type mask = ~(value_type(1 << ((sizeof(value_type)*CHAR_BIT) - 1)));

  void operator()(__m128i* _inout){
    
    static const __m128i vec_mask = _mm_set1_epi8(mask);
    
    *_inout = _mm_xor_si128(*_inout,vec_mask);

    return;
  }

};

template <typename T>
struct vec_rotate_left
{
  
  
  int shift;
  vec_rotate_left(int _shift = 1):
    shift(_shift)
  {};

  __m128i operator()(const __m128i* _in){
    __m128i value;
    throw std::runtime_error("rotate_left called with unknown type");
    return value;
  }
};

template <>
struct vec_rotate_left<unsigned short>
{
  typedef unsigned short value_type;
  int shift;
  vec_rotate_left(int _shift = 1):
    shift(_shift)
  {};

  __m128i operator()(const __m128i* _in){
    
    static const value_type num_bits = (sizeof(value_type) * CHAR_BIT) - shift;

    // type shifted = _in << shift;
    __m128i value = *_in;
    value = _mm_slli_epi16(value, shift);

    static const value_type mask = ~(~0 << shift);
    static const __m128i vec_mask = _mm_set1_epi16(mask);

    // type left_over = (_in >> num_bits) & mask;
    __m128i left_over = _mm_srli_epi16(*_in,num_bits);
    left_over = _mm_and_si128(left_over, vec_mask);

    // return  (shifted | left_over);
    value = _mm_or_si128(value, left_over);
    return value;
  }
};


template <>
struct vec_rotate_left<short>
{
  typedef short value_type;
  int shift;
  vec_rotate_left(int _shift = 1):
    shift(_shift)
  {};

  __m128i operator()(const __m128i* _in){
    vec_rotate_left<unsigned short> rotater(shift);
    return rotater(_in);
  }
};

template <>
struct vec_rotate_left<unsigned char>
{
  typedef unsigned char value_type;
  int shift;
  vec_rotate_left(int _shift = 1):
    shift(_shift)
  {};

  __m128i operator()(const __m128i* _in){

    static const value_type num_bits = (sizeof(value_type) * CHAR_BIT) - shift;


    static const shift_left_m128i<value_type> left_shifter = {};
    static const shift_right_m128i<value_type> right_shifter = {};

    // type shifted = _in << shift;
    __m128i value = *_in;

    
    
    // value = _mm_slli_epi16(value, shift);
    value = left_shifter(value, shift);

    static const value_type mask = ~(~0 << shift);
    static const __m128i vec_mask = _mm_set1_epi8(mask);

    // type left_over = (_in >> num_bits) & mask;
    __m128i left_over = right_shifter(*_in,num_bits);
    left_over = _mm_and_si128(left_over, vec_mask);

    // return  (shifted | left_over);
    value = _mm_or_si128(value, left_over);
    return value;
  }
};


template <typename T>
struct to_32bit_field
{
  static const __m128i conversion(const __m128i& _block){
    
    //TODO convert to static assert
    if(sizeof(T)>4){
      std::cerr << "[to_32bit_field::conversion]\tfailed due to incompatible type (sizeof(T)="<< sizeof(T) <<" > 4)\n";
      throw std::runtime_error("[to_32bit_field::conversion]\tfailed due to incompatible type");
    }
    return _block;
  }
};


template <>
struct to_32bit_field<unsigned short>
{
  static const __m128i conversion(const __m128i& _block){
    return _mm_cvtepu16_epi32(_block);
  }
};

template <>
struct to_32bit_field<short>
{
  static const __m128i conversion(const __m128i& _block){
    return _mm_cvtepi16_epi32(_block);
  }
};



template <>
struct to_32bit_field<unsigned char>
{
  static const __m128i conversion(const __m128i& _block){
    return _mm_cvtepu8_epi32(_block);
  }
};

template <>
struct to_32bit_field<char>
{
  static const __m128i conversion(const __m128i& _block){
    return _mm_cvtepi8_epi32(_block);
  }
};


/**
   \brief bitplane reordering given sse register _block, new planes will be put into _reordered
   
   \param[in] _block SSE block that contains X times the type identified by T
   \param[out] _reordered bit planes to write reordered data into
   \param[in] _left_shift_output move the result number of bits (given by _left_shift_output) left 
   before writing it to _reordered

   Example: 
   given an input block of 
   {1000000000000000, 1000000000000000, 1000000000000000, 1000000000000000, 
    1000000000000000, 1000000000000000, 1000000000000000, 1000000000000000}
   the expected output is
   {1111111100000000, 0000000000000000, 0000000000000000, 0000000000000000, 
    0000000000000000, 0000000000000000, 0000000000000000, 0000000000000000,
    0000000000000000, 0000000000000000, 0000000000000000, 0000000000000000,
    0000000000000000, 0000000000000000, 0000000000000000, 0000000000000000}

    IMPORTANT: It is assumed that the items stored in __m128i originally were encoded as type T
   \return 
   \retval 
   
*/
template <unsigned plane_size, typename T>
void reorder_bitplanes(const __m128i& _block, 
		       std::vector<T*>& _reordered,
		       const unsigned& _left_shift_output = 0
		       ){

  //TODO: convert to static assert
  if(plane_size>(CHAR_BIT*sizeof(T))){
    std::cerr << "[reorder_bitplanes]\t plane_size larger than input type. Doing nothing.\n";
    return;
  }

  static const unsigned __m128i_in_bytes = 16;
  unsigned n_planes = (sizeof(T)*CHAR_BIT)/plane_size;

  unsigned reordered_byte = _reordered.size()*sizeof(T);

  if(!(reordered_byte>=__m128i_in_bytes)){
    std::cerr << "[reorder_bitplanes]\t buffer for output has insufficent size, " 
	      << "found " << reordered_byte << " B, min. required " << __m128i_in_bytes << "\n"
	      << "Doing Nothing.\n";
    return;
  }
    
  static const unsigned n_items_per_m128i_half = __m128i_in_bytes/(2*sizeof(T));

  const unsigned planes_per_output_item = n_planes/_reordered.size();
  const unsigned bits_per_plane_on_output_item = (sizeof(T)*CHAR_BIT)/planes_per_output_item;
  static const unsigned char left_shift_to_msb = sizeof(int) - sizeof(T);
  
  shift_left_m128i<T> left_shifter;

  for(unsigned plane = 0;plane<(n_planes);plane+=plane_size){

    //left shift by plane to make the bit of interest the msb
    
    __m128i input = left_shifter(_block,plane);

    __m128i v_first_items =  _mm_slli_si128(
					    //convert to 32bits in order to get only the first (items 0,1,2,3 in _block) half of m128i
					    to_32bit_field<T>::conversion(input),
					    left_shift_to_msb //left shift by 2 bytes to move the bits towards the MSB
					    ); 

    //invert the sequence of v_first_items
    v_first_items = _mm_shuffle_epi32(v_first_items, _MM_SHUFFLE(0,1,2,3));
    
    __m128 first_part = *reinterpret_cast<__m128*>(&v_first_items);

    //collect the msb per 32bit item into result
    T result = _mm_movemask_ps (first_part); 
    result <<= (n_items_per_m128i_half*plane_size);

    __m128 input_casted = reinterpret_cast<__m128>(input);
    
    __m128i swapped_second_first = reinterpret_cast<__m128i>(//swap second with first half of 128bits
							  _mm_movehl_ps(input_casted,input_casted)
							  );
    
    __m128i v_second_items = _mm_slli_si128(//convert to 32bits in order to get only the first (items 0,1,2,3 in _block) half of m128i
					    to_32bit_field<T>::conversion(swapped_second_first),
					    //left shift by n bytes to get the lower half of 32bits 
					    //(which is the higher half of the original)
					    left_shift_to_msb
					);

    //invert the sequence of v_second_items
    v_second_items = _mm_shuffle_epi32(v_second_items, _MM_SHUFFLE(0,1,2,3));

    //collect the msb per 32bit item into result
    result += _mm_movemask_ps (reinterpret_cast<__m128>(v_second_items)) ; 

    unsigned reordered_index = planes_per_output_item > 1 ? plane/planes_per_output_item : plane;
    
    result <<= _left_shift_output;
    
    if(planes_per_output_item!=1){
      unsigned offset = ((plane % planes_per_output_item)*bits_per_plane_on_output_item);
      *(_reordered[reordered_index]) += (result << offset);
    }
    else
      *(_reordered[reordered_index]) += result;
  }

}


#endif /* _BITPLANE_REORDER_DETAIL_H_ */
