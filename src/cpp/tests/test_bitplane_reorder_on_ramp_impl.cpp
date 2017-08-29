#define BOOST_TEST_MODULE TEST_BITPLANE_REORDER_IMPL
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"

#include <vector>
#include <iostream>
#include <algorithm> // for copy
#include <iterator> // for ostream_iterator

#include "encoders/sqeazy_impl.hpp"
#include "sse_test_utils.hpp"

typedef ramp_fixture<(1 << 15)> default_ramp_fixture;


BOOST_FIXTURE_TEST_SUITE( check_sse_ramp_16bit, default_ramp_fixture )

BOOST_AUTO_TEST_CASE( insert_into_simd_128 ){


  std::copy(input.begin(),input.end(),output.begin());

  const std::size_t n_elements_per_simd = 16/sizeof(value_t);


  __m128i block;

  for(std::size_t i = 0;i<128;i+=n_elements_per_simd){


    block = _mm_set1_epi16(0);
    for(std::size_t pos = 0;pos<n_elements_per_simd;++pos){
      block = sqeazy::detail::sse_insert_epi16(block,input[i+pos],pos);
    }

    _mm_store_si128(reinterpret_cast<__m128i*>(&reference[i]),block);
  }

  for(unsigned i = 0;i<128;++i){

    try{
      BOOST_REQUIRE(output[i] == reference[i]);
    }
    catch(...){
      std::cerr << "[insert_into_simd_128] " << i << " / "<< 128<<" item does not match "
        << "scalar :" <<  output[i] << ", "
        << "sse    :" <<  reference[i]
        << "\n";
      throw;
    }

  }
}


BOOST_AUTO_TEST_CASE( rotate_left_128 ){



  const std::size_t n_elements_per_simd = 16/sizeof(input[0]);

  //scalar
  std::transform(input.cbegin(),input.cbegin()+128,
         output.begin(),
         [](const std::uint16_t& el){
           std::uint16_t value = sqeazy::detail::xor_if_signed(el);
           value = sqeazy::detail::rotate_left<1>(value);
           return value;
         });

  __m128i block;
  sqeazy::detail::vec_xor<value_t> xoring;
  sqeazy::detail::vec_rotate_left<value_t> vrotate_left;

  for(std::size_t i = 0;i<128;i+=n_elements_per_simd){
    block = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[i]));

    if(std::numeric_limits<value_t>::is_signed)
      xoring(&block);
    block = vrotate_left(&block); //

    _mm_store_si128(reinterpret_cast<__m128i*>(&reference[i]),block);
  }

  for(unsigned i = 0;i<128;++i){

    try{
      BOOST_REQUIRE(output[i] == reference[i]);
    }
    catch(...){
      std::cerr << "[rotate_left_first_128] " << i << " / "<< 128<<" item does not match "
        << "scalar :" <<  output[i] << ", "
        << "sse    :" <<  reference[i]
        << "\n";
      throw;
    }

  }
}

BOOST_AUTO_TEST_CASE( rotate_left_all ){



  const std::size_t n_elements_per_simd = 16/sizeof(input[0]);

  //scalar
  std::transform(input.cbegin(),input.cend(),
         output.begin(),
         [](const std::uint16_t& el){
           std::uint16_t value = sqeazy::detail::xor_if_signed(el);
           value = sqeazy::detail::rotate_left<1>(value);
           return value;
         });

  __m128i block;
  sqeazy::detail::vec_xor<value_t> xoring;
  sqeazy::detail::vec_rotate_left<value_t> vrotate_left;

  for(std::size_t i = 0;i<input.size();i+=n_elements_per_simd){
    block = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[i]));

    if(std::numeric_limits<value_t>::is_signed)
      xoring(&block);
    block = vrotate_left(&block); //

    _mm_store_si128(reinterpret_cast<__m128i*>(&reference[i]),block);
  }

  for(unsigned i = 0;i<128;++i){

    try{
      BOOST_REQUIRE(output[i] == reference[i]);
    }
    catch(...){
      std::cerr << "[rotate_left_first_128] " << i << " / "<< input.size()<<" item does not match "
        << "scalar :" <<  output[i] << ", "
        << "sse    :" <<  reference[i]
        << "\n";
      throw;
    }

  }
}


BOOST_AUTO_TEST_CASE( shift_left_by_1_all ){



  const std::size_t n_elements_per_simd = 16/sizeof(input[0]);

  //scalar
  std::transform(input.cbegin(),input.cend(),
         output.begin(),
         [](const std::uint16_t& el){
           return el << 1;
         });

  //simd
  __m128i block;
  sqeazy::detail::shift_left_m128i<value_t> vshift_left;

  for(std::size_t i = 0;i<input.size();i+=n_elements_per_simd){
    block = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[i]));

    block = vshift_left(block,1); //

    _mm_store_si128(reinterpret_cast<__m128i*>(&reference[i]),block);
  }

  for(unsigned i = 0;i<input.size();++i){

    try{
      BOOST_REQUIRE(output[i] == reference[i]);
    }
    catch(...){
      std::cerr << "[shift_left_first_128] " << i << " / "<< input.size()<<" item does not match "
        << "scalar :" <<  output[i] << ", "
        << "sse    :" <<  reference[i]
        << "\n";
      throw;
    }

  }
}

BOOST_AUTO_TEST_CASE( shift_left_by_2_all ){



  const std::size_t n_elements_per_simd = 16/sizeof(input[0]);

  //scalar
  std::transform(input.cbegin(),input.cend(),
         output.begin(),
         [](const std::uint16_t& el){
           return el << 2;
         });

  //simd
  __m128i block;
  sqeazy::detail::shift_left_m128i<value_t> vshift_left;

  for(std::size_t i = 0;i<input.size();i+=n_elements_per_simd){
    block = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[i]));

    block = vshift_left(block,2); //

    _mm_store_si128(reinterpret_cast<__m128i*>(&reference[i]),block);
  }

  for(unsigned i = 0;i<input.size();++i){

    try{
      BOOST_REQUIRE(output[i] == reference[i]);
    }
    catch(...){
      std::cerr << "[shift_left_first_128] " << i << " / "<< input.size()<<" item does not match "
        << "scalar :" <<  output[i] << ", "
        << "sse    :" <<  reference[i]
        << "\n";
      throw;
    }

  }
}


BOOST_AUTO_TEST_CASE( shift_left_by_4_all ){



  const std::size_t n_elements_per_simd = 16/sizeof(input[0]);

  //scalar
  std::transform(input.cbegin(),input.cend(),
         output.begin(),
         [](const std::uint16_t& el){
           return el << 4;
         });

  //simd
  __m128i block;
  sqeazy::detail::shift_left_m128i<value_t> vshift_left;

  for(std::size_t i = 0;i<input.size();i+=n_elements_per_simd){
    block = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[i]));

    block = vshift_left(block,4); //

    _mm_store_si128(reinterpret_cast<__m128i*>(&reference[i]),block);
  }

  for(unsigned i = 0;i<input.size();++i){

    try{
      BOOST_REQUIRE(output[i] == reference[i]);
    }
    catch(...){
      std::cerr << "[shift_left_first_128] " << i << " / "<< input.size()<<" item does not match "
        << "scalar :" <<  output[i] << ", "
        << "sse    :" <<  reference[i]
        << "\n";
      throw;
    }

  }
}


BOOST_AUTO_TEST_CASE( shift_left_by_8_all ){



  const std::size_t n_elements_per_simd = 16/sizeof(input[0]);

  //scalar
  std::transform(input.cbegin(),input.cend(),
         output.begin(),
         [](const std::uint16_t& el){
           return el << 8;
         });

  //simd
  __m128i block;
  sqeazy::detail::shift_left_m128i<value_t> vshift_left;

  for(std::size_t i = 0;i<input.size();i+=n_elements_per_simd){
    block = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[i]));

    block = vshift_left(block,8); //

    _mm_store_si128(reinterpret_cast<__m128i*>(&reference[i]),block);
  }

  for(unsigned i = 0;i<input.size();++i){

    try{
      BOOST_REQUIRE(output[i] == reference[i]);
    }
    catch(...){
      std::cerr << "[shift_left_first_128] " << i << " / "<< input.size()<<" item does not match "
        << "scalar :" <<  output[i] << ", "
        << "sse    :" <<  reference[i]
        << "\n";
      throw;
    }

  }
}


BOOST_AUTO_TEST_CASE( shift_left_by_9_all ){



  const std::size_t n_elements_per_simd = 16/sizeof(input[0]);

  //scalar
  std::transform(input.cbegin(),input.cend(),
         output.begin(),
         [](const std::uint16_t& el){
           return el << 9;
         });

  //simd
  __m128i block;
  sqeazy::detail::shift_left_m128i<value_t> vshift_left;

  for(std::size_t i = 0;i<input.size();i+=n_elements_per_simd){
    block = _mm_load_si128(reinterpret_cast<const __m128i*>(&input[i]));

    block = vshift_left(block,9); //

    _mm_store_si128(reinterpret_cast<__m128i*>(&reference[i]),block);
  }

  for(unsigned i = 0;i<input.size();++i){

    try{
      BOOST_REQUIRE(output[i] == reference[i]);
    }
    catch(...){
      std::cerr << "[shift_left_first_128] " << i << " / "<< input.size()<<" item does not match "
        << "scalar :" <<  output[i] << ", "
        << "sse    :" <<  reference[i]
        << "\n";
      throw;
    }

  }
}


BOOST_AUTO_TEST_CASE( versus_default_first_128 ){

  int ret1 = sqeazy::detail::sse_bitplane_reorder_encode<1>(&input[0], &output[0], 128);
  int ret2 = sqeazy::detail::scalar_bitplane_reorder_encode<1>(&input[0], &reference[0], 128);


  BOOST_REQUIRE(ret1 == ret2);

  for(unsigned i = 0;i<128;++i){

    try{
      BOOST_REQUIRE(output[i] == reference[i]);
    }
    catch(...){
      std::cerr << "[versus_default_first_128] " << i << " / "<< 128<<" item does not match "
        << "scalar :" <<  output[i] << ", "
        << "sse    :" <<  reference[i]
        << "\n";
      throw;
    }

  }
}

BOOST_AUTO_TEST_CASE( versus_default_all ){

  int ret1 = sqeazy::detail::sse_bitplane_reorder_encode<1>(&input[0], &output[0], input.size());
  int ret2 = sqeazy::detail::scalar_bitplane_reorder_encode<1>(&input[0], &reference[0], input.size());


  BOOST_REQUIRE(ret1 == ret2);
  for(unsigned i = 0;i<input.size();++i){
    try{
      BOOST_REQUIRE(output[i] == reference[i]);
    }
    catch(...){
      //BOOST_TEST_MESSAGE("[check-sse-default zick-zack] " << i << " / "<< input.size()<<" item does not match" );
      std::cerr << "[versus_default_all] " << i << " / "<< input.size()<<" item does not match "
                << "scalar = " <<  output[i] << ", "
                << "sse    = " <<  reference[i]
                << "\n";
      throw;
    }
  }
}

BOOST_AUTO_TEST_CASE( versus_default_all_2threads ){

  int ret1 = sqeazy::detail::sse_bitplane_reorder_encode<1>(&input[0], &output[0], input.size(),2);
  int ret2 = sqeazy::detail::scalar_bitplane_reorder_encode<1>(&input[0], &reference[0], input.size(),2);


  BOOST_REQUIRE(ret1 == ret2);
  for(unsigned i = 0;i<input.size();++i){
    try{
      BOOST_REQUIRE(output[i] == reference[i]);
    }
    catch(...){
      //BOOST_TEST_MESSAGE("[check-sse-default zick-zack] " << i << " / "<< input.size()<<" item does not match" );
      std::cerr << "[versus_default_all] " << i << " / "<< input.size()<<" item does not match "
                << "scalar :" <<  output[i] << ", "
                << "sse    :" <<  reference[i]
                << "\n";
      throw;
    }
  }
}

BOOST_AUTO_TEST_CASE( versus_expanded_ramp ){

  input.resize(1 << 22);
  output.resize(1 << 22);
  reference = output;

  int ret1 = sqeazy::detail::sse_bitplane_reorder_encode<1>(&input[0], &output[0], input.size());
  int ret2 = sqeazy::detail::scalar_bitplane_reorder_encode<1>(&input[0], &reference[0], input.size());


  BOOST_REQUIRE(ret1 == ret2);
  for(unsigned i = 0;i<input.size();++i){
    try{
      BOOST_REQUIRE(output[i] == reference[i]);
    }
    catch(...){
      //BOOST_TEST_MESSAGE("[check-sse-default zick-zack] " << i << " / "<< input.size()<<" item does not match" );
      std::cerr << "[versus_default_all] " << i << " / "<< input.size()<<" item does not match "
                << "scalar :" <<  output[i] << ", "
                << "sse    :" <<  reference[i]
                << "\n";
      throw;
    }
  }
}

BOOST_AUTO_TEST_CASE( versus_expanded_ramp_2threads ){

  input.resize(1 << 22);
  output.resize(1 << 22);
  reference = output;

  int ret1 = sqeazy::detail::sse_bitplane_reorder_encode<1>(&input[0], &output[0], input.size(),2);
  int ret2 = sqeazy::detail::scalar_bitplane_reorder_encode<1>(&input[0], &reference[0], input.size(),2);


  BOOST_REQUIRE(ret1 == ret2);
  for(unsigned i = 0;i<input.size();++i){
    try{
      BOOST_REQUIRE(output[i] == reference[i]);
    }
    catch(...){
      //BOOST_TEST_MESSAGE("[check-sse-default zick-zack] " << i << " / "<< input.size()<<" item does not match" );
      std::cerr << "[versus_default_all] " << i << " / "<< input.size()<<" item does not match "
                << "scalar :" <<  output[i] << ", "
                << "sse    :" <<  reference[i]
                << "\n";
      throw;
    }
  }
}


BOOST_AUTO_TEST_SUITE_END()
