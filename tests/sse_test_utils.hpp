#ifndef _SEE_UTILS_H_
#define _SEE_UTILS_H_


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

template <const unsigned size = (32*(1 << 10)), typename T = unsigned short>
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
    
    calc_first_16_hand[11] = 0xff;// = 255
    calc_first_16_hand[12] = 0xf0f;// = 3855
    calc_first_16_hand[13] = 0x3333;// = 13107
    calc_first_16_hand[14] = 0x5555;// = 21845

  }
};


#endif /* _SEE_UTILS_H_ */
