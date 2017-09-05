#define BOOST_TEST_MODULE TEST_QUANTISER_IMPL
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"
#include <climits>
#include <vector>
#include <iostream>
#include <bitset>
#include <map>
#include <cstdint>
#include <iterator>
#include <random>

#include "boost/filesystem.hpp"
#include "volume_fixtures.hpp"
#include "sqeazy_algorithms.hpp"
#include "encoders/quantiser_scheme_impl.hpp"

namespace bfs = boost::filesystem;

BOOST_FIXTURE_TEST_SUITE( checkon_16bit, sqeazy::volume_fixture<uint16_t> )

BOOST_AUTO_TEST_CASE( default_constructs ){

  sqeazy::quantiser<uint16_t,uint8_t> shrinker;
  BOOST_CHECK(shrinker.sum_ == 0);

}

BOOST_AUTO_TEST_CASE( constructs_with_payload ){

  sqeazy::quantiser<uint16_t,uint8_t> shrinker(embryo_.data(),
                           embryo_.data()+embryo_.num_elements());
  BOOST_CHECK_NE(shrinker.sum_,0);

}

BOOST_AUTO_TEST_CASE( constructs_with_payload_and_weighter ){

  sqeazy::quantiser<uint16_t,uint8_t> default_ctor(embryo_.data(),
                           embryo_.data()+embryo_.num_elements()
                           );

  sqeazy::quantiser<uint16_t,uint8_t> weighted(embryo_.data(),
                                               embryo_.data()+embryo_.num_elements(),1,
                                               sqeazy::weighters::offset_power_of());

  BOOST_CHECK_NE(std::equal(default_ctor.weights_.begin(), default_ctor.weights_.end(),
                weighted.weights_.cbegin()), true);

}


BOOST_AUTO_TEST_CASE( setup ){

  sqeazy::quantiser<uint16_t,uint8_t> shrinker;
  shrinker.setup(embryo_.data(),embryo_.num_elements());
  BOOST_CHECK(shrinker.sum_ != 0);
  BOOST_CHECK_NE(std::accumulate(shrinker.lut_encode_.begin(), shrinker.lut_encode_.end(),0),0);
}

BOOST_AUTO_TEST_CASE( reset ){

  sqeazy::quantiser<uint16_t,uint8_t> shrinker(embryo_.data(),
                           embryo_.data()+embryo_.num_elements());
  BOOST_CHECK_NE(shrinker.sum_,0);

  shrinker.reset();

  BOOST_CHECK_EQUAL(shrinker.sum_,0);

}

BOOST_AUTO_TEST_CASE( max_value ){

  sqeazy::quantiser<uint16_t,uint8_t> shrinker(embryo_.data(),
                           embryo_.data()+embryo_.num_elements());

  uint16_t val = shrinker.max_value();
  uint16_t exp = signal_intensity_;
  BOOST_CHECK_EQUAL(val,exp);

}

BOOST_AUTO_TEST_CASE( min_value ){

  sqeazy::quantiser<uint16_t,uint8_t> shrinker(embryo_.data(),
                           embryo_.data()+embryo_.num_elements());

    uint16_t val = shrinker.min_value();
  BOOST_CHECK_EQUAL(val,0);

}


BOOST_AUTO_TEST_CASE( encodes ){

  sqeazy::quantiser<uint16_t,uint8_t> shrinker(embryo_.data(),
                           embryo_.data()+embryo_.num_elements());
  std::vector<uint8_t> retrieved(embryo_.num_elements(),0);
  shrinker.encode(embryo_.data(),
          embryo_.num_elements(),
          &retrieved[0]);

  float sum = std::accumulate(retrieved.begin(), retrieved.end(),0);
  BOOST_CHECK_NE(sum,0);

}



BOOST_AUTO_TEST_CASE( embryo_roundtrip ){

  std::vector<uint8_t> encoded(embryo_.num_elements(),0);

  sqeazy::quantiser<uint16_t,uint8_t> shrinker(embryo_.data(),
                           embryo_.data() + embryo_.num_elements());
  shrinker.encode(embryo_.data(),embryo_.num_elements(),&encoded[0]);

  std::vector<uint16_t> reconstructed(encoded.size(),0);
  shrinker.decode(&encoded[0],
          encoded.size(),
          &reconstructed[0]);

  BOOST_CHECK_EQUAL_COLLECTIONS(reconstructed.begin(), reconstructed.end(),embryo_.data(),
                embryo_.data()+ embryo_.num_elements());
}

BOOST_AUTO_TEST_CASE( embryo_roundtrip_2threads ){

  std::vector<uint8_t> encoded(embryo_.num_elements(),0);

  sqeazy::quantiser<uint16_t,uint8_t> shrinker(embryo_.data(),
                                               embryo_.data() + embryo_.num_elements(),
                                               2);
  shrinker.encode(embryo_.data(),embryo_.num_elements(),&encoded[0]);

  std::vector<uint16_t> reconstructed(encoded.size(),0);
  shrinker.decode(&encoded[0],
                  encoded.size(),
                  &reconstructed[0]);

  BOOST_CHECK_EQUAL_COLLECTIONS(reconstructed.begin(), reconstructed.end(),embryo_.data(),
                                embryo_.data()+ embryo_.num_elements());
}


BOOST_AUTO_TEST_CASE( embryo_roundtrip_scheme_api ){

  std::vector<uint8_t> encoded(embryo_.num_elements(),0);

  sqeazy::quantiser_scheme<uint16_t,uint8_t> shrinker;
  shrinker.encode(embryo_.data(),&encoded[0],embryo_.num_elements());

  std::vector<uint16_t> reconstructed(encoded.size(),0);
  shrinker.decode(&encoded[0],
          &reconstructed[0],
          encoded.size());

  BOOST_CHECK_EQUAL_COLLECTIONS(reconstructed.begin(), reconstructed.end(),embryo_.data(),
                embryo_.data()+ embryo_.num_elements());
}

BOOST_AUTO_TEST_CASE( embryo_roundtrip_scheme_api_2_threads ){

  std::vector<uint8_t> encoded(embryo_.num_elements(),0);

  sqeazy::quantiser_scheme<uint16_t,uint8_t> shrinker;
  shrinker.set_n_threads(2);
  shrinker.encode(embryo_.data(),&encoded[0],embryo_.num_elements());

  std::vector<uint16_t> reconstructed(encoded.size(),0);
  shrinker.decode(&encoded[0],
          &reconstructed[0],
          encoded.size());

  BOOST_CHECK_EQUAL_COLLECTIONS(reconstructed.begin(), reconstructed.end(),embryo_.data(),
                embryo_.data()+ embryo_.num_elements());
}

BOOST_AUTO_TEST_CASE( noisy_embryo_roundtrip ){

  std::vector<uint8_t> encoded(noisy_embryo_.num_elements(),0);

  sqeazy::quantiser<uint16_t,uint8_t> shrinker(noisy_embryo_.data(),
                                               noisy_embryo_.data() + noisy_embryo_.num_elements());

  shrinker.encode(noisy_embryo_.data(),noisy_embryo_.num_elements(),&encoded[0]);

  std::vector<uint16_t> reconstructed(encoded.size(),0);
  shrinker.decode(&encoded[0],
          encoded.size(),
          &reconstructed[0]);

  double rms = sqeazy::rms(reconstructed.begin(), reconstructed.end(),noisy_embryo_.data());


  BOOST_REQUIRE_GT(rms,0);

  try{
    BOOST_REQUIRE_LT(rms,20e3);
  }
  catch(...){

    std::stringstream log_file;
    log_file << "failing_checkon_16bit_"
             << boost::unit_test::framework::current_test_case().p_name
             << ".log";

    shrinker.dump(log_file.str());
  }

}

BOOST_AUTO_TEST_CASE( noisy_embryo_roundtrip_2threads ){

  std::vector<uint8_t> encoded(noisy_embryo_.num_elements(),0);

  sqeazy::quantiser<uint16_t,uint8_t> shrinker(noisy_embryo_.data(),
                                               noisy_embryo_.data() + noisy_embryo_.num_elements(),
                                               2);

  shrinker.encode(noisy_embryo_.data(),noisy_embryo_.num_elements(),&encoded[0]);

  std::vector<uint16_t> reconstructed(encoded.size(),0);
  shrinker.decode(&encoded[0],
                  encoded.size(),
                  &reconstructed[0]);

  double rms = sqeazy::rms(reconstructed.begin(), reconstructed.end(),noisy_embryo_.data());


  BOOST_REQUIRE_GT(rms,0);

  try{
    BOOST_REQUIRE_LT(rms,20e3);
  }
  catch(...){

    std::stringstream log_file;
    log_file << "failing_checkon_16bit_"
             << boost::unit_test::framework::current_test_case().p_name
             << ".log";

    shrinker.dump(log_file.str());
  }
}

BOOST_AUTO_TEST_CASE( noisy_embryo_roundtrip_scheme_api ){

  std::vector<uint8_t> encoded(noisy_embryo_.num_elements(),0);

  sqeazy::quantiser_scheme<uint16_t,uint8_t> shrinker;
  shrinker.encode(noisy_embryo_.data(),&encoded[0],noisy_embryo_.num_elements());

  std::vector<uint16_t> reconstructed(encoded.size(),0);
  shrinker.decode(&encoded[0],
          &reconstructed[0],
          encoded.size());

  double rms = sqeazy::rms(reconstructed.begin(), reconstructed.end(),noisy_embryo_.data());


  BOOST_REQUIRE_GT(rms,0);

  try{
    BOOST_REQUIRE_LT(rms,20e3);
  }
  catch(...){

    std::stringstream log_file;
    log_file << "failing_checkon_16bit_"
             << boost::unit_test::framework::current_test_case().p_name
             << ".log";

    shrinker.dump(log_file.str());
  }

}

BOOST_AUTO_TEST_CASE( write_to_file ){

  std::stringstream lut_file;
  lut_file << "checkon_16bit_"
       << boost::unit_test::framework::current_test_case().p_name
       << ".log";

  bfs::path tgt = lut_file.str();
  sqeazy::quantiser<uint16_t,uint8_t> shrinker(embryo_.data(),
                           embryo_.data() + embryo_.num_elements());

  shrinker.lut_to_file(lut_file.str(),shrinker.lut_decode_);

  BOOST_REQUIRE(bfs::exists(tgt));
  BOOST_REQUIRE_NE(bfs::file_size(tgt),0u);

  std::vector<uint8_t> encoded(embryo_.num_elements(),0);
  shrinker.encode(embryo_.data(),embryo_.num_elements(),&encoded[0]);


  std::vector<uint16_t> reconstructed(encoded.size(),0);
  shrinker.decode(lut_file.str(),
          &encoded[0],
          encoded.size(),
          &reconstructed[0]);

  BOOST_REQUIRE_EQUAL_COLLECTIONS(reconstructed.begin(), reconstructed.end(),embryo_.data(),
                  embryo_.data()+ embryo_.num_elements());

  if(bfs::exists(tgt))
    bfs::remove(tgt);

}

BOOST_AUTO_TEST_CASE( write_to_string ){

  std::stringstream lut_file;
  lut_file << "checkon_16bit_"
       << boost::unit_test::framework::current_test_case().p_name
       << ".log";

  sqeazy::quantiser<uint16_t,uint8_t> shrinker(embryo_.data(),
                           embryo_.data() + embryo_.num_elements());

  std::string lut = shrinker.lut_to_string(shrinker.lut_decode_);

  BOOST_CHECK_GT(lut.size(),0u);

  std::vector<uint8_t> encoded(embryo_.num_elements(),0);
  shrinker.encode(embryo_.data(),embryo_.num_elements(),&encoded[0]);
  std::vector<uint16_t> reconstructed(encoded.size(),0);

  decltype(shrinker.lut_decode_) lut_decode;
  sqeazy::quantiser<uint16_t,uint8_t> other;
  other.lut_from_string(lut,lut_decode);
  other.decode(lut_decode,
               &encoded[0],
               encoded.size(),
               &reconstructed[0]);


  // shrinker.lut_from_string(lut,lut_decode);

  // shrinker.decode(lut_decode,
  //         &encoded[0],
  //         encoded.size(),
  //         &reconstructed[0]);

  BOOST_REQUIRE_EQUAL_COLLECTIONS(reconstructed.begin(), reconstructed.end(),embryo_.data(),
                  embryo_.data()+ embryo_.num_elements());

}


BOOST_AUTO_TEST_CASE( inject_unknown_values ){

  typedef typename sqeazy::quantiser<uint16_t,uint8_t>::raw_t lut_value_t;
  typedef typename std::iterator_traits<sqeazy::quantiser<uint16_t,uint8_t>::lut_decode_t::iterator>::value_type value_t;
  static_assert(std::is_same<lut_value_t,value_t>::value,"raw_t is unqeual the value_type of the lut_decode_t::const_iterator");

  std::stringstream lut_file;
  lut_file << "checkon_16bit_"
           << boost::unit_test::framework::current_test_case().p_name
           << ".log";

  std::vector<std::uint16_t> synthetic(512,0);
  for(std::size_t i = 0;i<synthetic.size();++i)
    synthetic.data()[i] = 1024 + 20*std::sin(i/4);

  sqeazy::quantiser<uint16_t,uint8_t> shrinker(synthetic.data(),
                                               synthetic.data() + synthetic.size());

  auto unary = [](value_t item){
    return (item != 0);
  };

  auto fitr = std::find_if(shrinker.lut_decode_.rbegin(),
                           shrinker.lut_decode_.rend(),
                           unary);

  std::size_t highest_nonzero_lut_key = std::distance(fitr,shrinker.lut_decode_.rend()-1);


  BOOST_CHECK_NE(highest_nonzero_lut_key,0u);
  BOOST_CHECK_LT(highest_nonzero_lut_key,shrinker.lut_decode_.size());
  BOOST_CHECK_NE(shrinker.lut_decode_[highest_nonzero_lut_key],0);
  // BOOST_CHECK_EQUAL(shrinker.lut_decode_[highest_nonzero_lut_key+1],0);


  std::string lut = shrinker.lut_to_string(shrinker.lut_decode_);
  std::vector<std::uint8_t> encoded(synthetic.size(),0);
  shrinker.encode(synthetic.data(),synthetic.size(),&encoded[0]);

  std::size_t highest_encoded_value   = *std::max_element(encoded.begin(),encoded.end());
  std::size_t highest_value   = *std::max_element(synthetic.begin(),synthetic.end());

  BOOST_CHECK_EQUAL(shrinker.lut_decode_[highest_nonzero_lut_key],highest_value);

  //inject unknown values into the encoded volume from [0,10%)
  std::size_t chunksize = (encoded.size()/2);
  auto seq_begin = encoded.begin();
  auto seq_end   = encoded.begin() + chunksize;
  const std::uint8_t max_val = highest_encoded_value;
  std::uint8_t max_repl = max_val + 10;

  std::replace(seq_begin, seq_end,
               max_val,
               max_repl);
  int count_replaced = std::count(seq_begin, seq_end,max_repl);
  BOOST_CHECK_GT(count_replaced,0);

  //inject unknown values into the encoded volume from [10%,25%)
  seq_begin = seq_end;
  seq_end = encoded.end();
  max_repl = std::numeric_limits<std::uint8_t>::max();

  std::replace(seq_begin, seq_end,
               max_val,
               max_repl);

  count_replaced = std::count(seq_begin, seq_end,max_repl);
  BOOST_CHECK_GT(count_replaced,0);

  decltype(shrinker.lut_decode_) decode_lut_reloaded;
  shrinker.lut_from_string(lut,decode_lut_reloaded);

  BOOST_REQUIRE_EQUAL_COLLECTIONS(decode_lut_reloaded.begin(),
                                  decode_lut_reloaded.end(),
                                  shrinker.lut_decode_.begin(),
                                  shrinker.lut_decode_.end());

  std::vector<std::uint16_t> reconstructed(encoded.size(),0);
  shrinker.decode(decode_lut_reloaded,
                  &encoded[0],
                  encoded.size(),
                  &reconstructed[0]);

  //this would be due to highest_nonzero_lut_key+1
  BOOST_REQUIRE_EQUAL_COLLECTIONS(reconstructed.begin(),
                                  reconstructed.begin()+chunksize,
                                  synthetic.begin(),
                                  synthetic.begin()+ chunksize);

  //this would be due to std::numeric_limits<uint8_t>::max()
  BOOST_REQUIRE_EQUAL_COLLECTIONS(reconstructed.begin()+chunksize,
                                  reconstructed.end(),
                                  synthetic.begin()+ chunksize,
                                  synthetic.end());

}


BOOST_AUTO_TEST_SUITE_END()

#include "quantiser_fixtures.hpp"

BOOST_FIXTURE_TEST_SUITE( loics_suite, sqeazy::quantise_fixture<uint16_t> )

BOOST_AUTO_TEST_CASE( shifted ){


  std::vector<uint8_t> encoded(shifted_.size(),0);
  sqeazy::quantiser<uint16_t,uint8_t> shrinker;
  BOOST_REQUIRE_NO_THROW(shrinker.encode(&shifted_[0],
                     shifted_.size(),
                     &encoded[0]));

  aligned_vector reconstructed(encoded.size(),0);
  shrinker.decode(&encoded[0],
          encoded.size(),
          &reconstructed[0]);

  double psnr = sqeazy::psnr(reconstructed.begin(), reconstructed.end(),
                 shifted_.begin());
  BOOST_CHECK_GT(psnr,0);


  double mse = sqeazy::mse(reconstructed.begin(), reconstructed.end(),
               shifted_.begin());
  BOOST_CHECK_GE(mse,0);
  BOOST_CHECK_LT(mse,.5);

  if(mse>1){
   std::stringstream log_file;
   log_file << "loics_suite_"
        << boost::unit_test::framework::current_test_case().p_name
        << ".log";

    shrinker.dump(log_file.str());
  }
  BOOST_TEST_MESSAGE("loics_suite/" << boost::unit_test::framework::current_test_case().p_name << "\tmse = " << mse);
}

BOOST_AUTO_TEST_CASE( shifted_wide ){


  std::vector<uint8_t> encoded(shifted_wide_.size(),0);
  sqeazy::quantiser<uint16_t,uint8_t> shrinker;
  BOOST_REQUIRE_NO_THROW(shrinker.encode(&shifted_wide_[0],
                     shifted_wide_.size(),
                     &encoded[0]));

  aligned_vector reconstructed(encoded.size(),0);
  shrinker.decode(&encoded[0],
          encoded.size(),
          &reconstructed[0]);


  double mse = sqeazy::mse(reconstructed.begin(), reconstructed.end(),
               shifted_wide_.begin());
  BOOST_CHECK_GE(mse,0);
  BOOST_CHECK_LT(mse,1);

  if(mse>1){
    std::stringstream log_file;

    log_file << "loics_suite_"
         << boost::unit_test::framework::current_test_case().p_name
         << ".log";

    shrinker.dump(log_file.str());
  }

  double psnr = sqeazy::psnr(reconstructed.begin(), reconstructed.end(),
                 shifted_wide_.begin());

  BOOST_CHECK_GT(psnr,0);
  BOOST_TEST_MESSAGE("loics_suite/" << boost::unit_test::framework::current_test_case().p_name << "\tmse = " << mse);
}

BOOST_AUTO_TEST_CASE( shifted_normal ){


  std::vector<uint8_t> encoded(shifted_normal_.size(),0);
  sqeazy::quantiser<uint16_t,uint8_t> shrinker;
  shrinker.encode(&shifted_normal_[0],
          shifted_normal_.size(),
          &encoded[0]);

  aligned_vector reconstructed(encoded.size(),0);
  shrinker.decode(&encoded[0],
          encoded.size(),
          &reconstructed[0]);

  double psnr = sqeazy::psnr(reconstructed.begin(), reconstructed.end(),
                 shifted_normal_.begin());

  BOOST_CHECK_GT(psnr,0);

  double mse = sqeazy::mse(reconstructed.begin(), reconstructed.end(),
               shifted_normal_.begin());
  BOOST_CHECK_GE(mse,0);
  //BOOST_CHECK_LT(mse,1.f);

  size_t dyn_range = sqeazy::dyn_range(shifted_normal_.begin(), shifted_normal_.end());
  float exp_deviation_per_value = dyn_range / (256.f);
  float exp_mse = exp_deviation_per_value*exp_deviation_per_value;

  BOOST_CHECK_LT(mse,exp_mse);

  if(mse>1.){
    std::stringstream log_file;
    log_file << "loics_suite_"
         << boost::unit_test::framework::current_test_case().p_name
         << ".log";

    shrinker.dump(log_file.str());
  }
  BOOST_TEST_MESSAGE("loics_suite/" << boost::unit_test::framework::current_test_case().p_name << "\tmse = " << mse << ", exp_mse = " << exp_mse);
}

BOOST_AUTO_TEST_CASE( strong_0 ){


  std::vector<uint8_t> encoded(strong_0_.size(),0);
  sqeazy::quantiser<uint16_t,uint8_t> shrinker;
  shrinker.encode(&strong_0_[0],
          strong_0_.size(),
          &encoded[0]);

  aligned_vector reconstructed(encoded.size(),0);
  shrinker.decode(&encoded[0],
          encoded.size(),
          &reconstructed[0]);

  double psnr = sqeazy::psnr(reconstructed.begin(), reconstructed.end(),
                 strong_0_.begin());

  BOOST_CHECK_GT(psnr,0);


  double mse = sqeazy::mse(reconstructed.begin(), reconstructed.end(),
               strong_0_.begin());
  BOOST_CHECK_GE(mse,0);
  BOOST_CHECK_LT(mse,1.f);

  if(mse>1.){
    std::stringstream log_file;
    log_file << "loics_suite_"
         << boost::unit_test::framework::current_test_case().p_name
         << ".log";

    shrinker.dump(log_file.str());
  }

  BOOST_TEST_MESSAGE("loics_suite/" << boost::unit_test::framework::current_test_case().p_name << "\tmse = " << mse);
}

BOOST_AUTO_TEST_CASE( realistic ){


  std::vector<uint8_t> encoded(realistic_.size(),0);
  sqeazy::quantiser<uint16_t,uint8_t> shrinker;
  shrinker.encode(&realistic_[0],
          realistic_.size(),
          &encoded[0]);

  size_t dyn_range = sqeazy::dyn_range(realistic_.begin(), realistic_.end());

  BOOST_CHECK_GT(dyn_range,1000u);
  BOOST_CHECK_LT(dyn_range,3000u);

  aligned_vector reconstructed(encoded.size(),0u);
  shrinker.decode(&encoded[0],
          encoded.size(),
          &reconstructed[0]);

  double psnr = sqeazy::psnr(reconstructed.begin(), reconstructed.end(),
                 realistic_.begin());

  BOOST_CHECK_GT(psnr,0u);

  float exp_deviation_per_value = dyn_range / (256.f);
  float exp_mse = exp_deviation_per_value*exp_deviation_per_value;

  double mse = sqeazy::mse(reconstructed.begin(), reconstructed.end(),
               realistic_.begin());
  BOOST_CHECK_GE(mse,0u);
  BOOST_WARN_LT(mse,1.f);
  BOOST_CHECK_LT(mse,exp_mse);
  if(mse>1.){
    std::stringstream log_file;
    log_file << "loics_suite_"
         << boost::unit_test::framework::current_test_case().p_name
         << ".log";

    shrinker.dump(log_file.str());
  }

  BOOST_TEST_MESSAGE("loics_suite/" << boost::unit_test::framework::current_test_case().p_name
             << "\t"  << "mse = " << mse << ", exp-mse = " << exp_mse);

  size_t reco_dyn_range = sqeazy::dyn_range(reconstructed.begin(), reconstructed.end());
  BOOST_CHECK_LT(reco_dyn_range,dyn_range);

  uint16_t min_val_original = *std::min_element(realistic_.begin(), realistic_.end());
  uint16_t max_val_original = *std::max_element(realistic_.begin(), realistic_.end());

  uint16_t min_val_reconstr = *std::min_element(reconstructed.begin(), reconstructed.end());
  uint16_t max_val_reconstr = *std::max_element(reconstructed.begin(), reconstructed.end());

  BOOST_CHECK_LE(max_val_reconstr,max_val_original);
  BOOST_CHECK_GE(min_val_reconstr,min_val_original);

  BOOST_TEST_MESSAGE("loics_suite/" << boost::unit_test::framework::current_test_case().p_name << " <original|reconstructed>"
             << "\t" << "min = " << min_val_original << " | " << min_val_reconstr
             << "\tmax = " << max_val_original << " | " << max_val_reconstr
             << "\tdrange = " << dyn_range << " | " << reco_dyn_range
             );

}

BOOST_AUTO_TEST_CASE( write_to_file_realistic_roundtrip ){

  std::stringstream lut_file;
  lut_file << "loics_suite_"
       << boost::unit_test::framework::current_test_case().p_name
       << ".log";

  bfs::path tgt = lut_file.str();
  sqeazy::quantiser<uint16_t,uint8_t> shrinker(&realistic_[0],
                           &realistic_[0] + realistic_.size());

  shrinker.lut_to_file(lut_file.str(),shrinker.lut_decode_);

  BOOST_REQUIRE(bfs::exists(tgt));
  BOOST_REQUIRE_NE(bfs::file_size(tgt),0u);

  std::vector<uint8_t> encoded(realistic_.size(),0u);
  shrinker.encode(&realistic_[0],realistic_.size(),&encoded[0]);


  std::vector<uint16_t> reconstructed(encoded.size(),0u);
  shrinker.decode(lut_file.str(),
          &encoded[0],
          encoded.size(),
          &reconstructed[0]);

  size_t dyn_range = sqeazy::dyn_range(realistic_.begin(), realistic_.end());
  float exp_deviation_per_value = dyn_range / (256.f);
  float exp_mse = exp_deviation_per_value*exp_deviation_per_value;

  double mse = sqeazy::mse(reconstructed.begin(), reconstructed.end(),
               realistic_.begin());
  BOOST_CHECK_GE(mse,0.);
  BOOST_WARN_LT(mse,1.f);
  BOOST_CHECK_LT(mse,exp_mse);
  if(mse>1.){
    std::stringstream log_file;
    log_file << "loics_suite_"
         << boost::unit_test::framework::current_test_case().p_name
         << ".log";

    shrinker.dump(log_file.str());
  }

  BOOST_TEST_MESSAGE("loics_suite/" << boost::unit_test::framework::current_test_case().p_name
             << "\t"  << "mse = " << mse << ", exp-mse = " << exp_mse);


  size_t reco_dyn_range = sqeazy::dyn_range(reconstructed.begin(), reconstructed.end());
  BOOST_CHECK_LT(reco_dyn_range,dyn_range);

  uint16_t min_val_original = *std::min_element(realistic_.begin(), realistic_.end());
  uint16_t max_val_original = *std::max_element(realistic_.begin(), realistic_.end());

  uint16_t min_val_reconstr = *std::min_element(reconstructed.begin(), reconstructed.end());
  uint16_t max_val_reconstr = *std::max_element(reconstructed.begin(), reconstructed.end());

  BOOST_CHECK_LE(max_val_reconstr,max_val_original);
  BOOST_CHECK_GE(min_val_reconstr,min_val_original);

  BOOST_TEST_MESSAGE("loics_suite/" << boost::unit_test::framework::current_test_case().p_name << " <original|reconstructed>"
             << "\t" << "min = " << min_val_original << " | " << min_val_reconstr
             << "\tmax = " << max_val_original << " | " << max_val_reconstr
             << "\tdrange = " << dyn_range << " | " << reco_dyn_range
             );

  if(bfs::exists(tgt))
    bfs::remove(tgt);

}

BOOST_AUTO_TEST_CASE( realistic_roundtrip_scheme_api ){

  std::stringstream lut_file;
  lut_file << "loics_suite_"
       << boost::unit_test::framework::current_test_case().p_name
       << ".log";

  bfs::path tgt = lut_file.str();
  sqeazy::quantiser_scheme<uint16_t,uint8_t> shrinker;
  std::vector<uint8_t> encoded(realistic_.size(),0);
  auto end_ptr = shrinker.encode(&realistic_[0],&encoded[0],realistic_.size());

  std::string shrinker_config = shrinker.config();

  BOOST_CHECK(end_ptr!=nullptr);
  BOOST_CHECK_EQUAL(std::distance(&encoded[0],end_ptr),ptrdiff_t(realistic_.size()));

  std::vector<uint16_t> reconstructed(encoded.size(),0);
  sqeazy::quantiser_scheme<uint16_t,uint8_t> reloaded(shrinker_config);

  int err =reloaded.decode(&encoded[0],
               &reconstructed[0],
               encoded.size());

  BOOST_CHECK_EQUAL(err,0);


  size_t dyn_range = sqeazy::dyn_range(realistic_.begin(), realistic_.end());
  float exp_deviation_per_value = dyn_range / (256.f);
  float exp_mse = exp_deviation_per_value*exp_deviation_per_value;

  double mse = sqeazy::mse(reconstructed.begin(), reconstructed.end(),
               realistic_.begin());

  BOOST_TEST_MESSAGE(boost::unit_test::framework::current_test_case().p_name << ", mse = " << mse);

  BOOST_CHECK_GE(mse,0);
  BOOST_WARN_LT(mse,1.f);
  BOOST_CHECK_LT(mse,exp_mse);
  if(mse>1.){
    std::stringstream log_file;
    log_file << "loics_suite_"
         << boost::unit_test::framework::current_test_case().p_name
         << ".log";

    shrinker.dump(log_file.str());
    BOOST_TEST_MESSAGE(boost::unit_test::framework::current_test_case().p_name<< ", wrote " << log_file.str());
  }

}

BOOST_AUTO_TEST_CASE( reloading_decode_lut_scheme_api ){

  std::stringstream lut_file;
  lut_file << "loics_suite_"
       << boost::unit_test::framework::current_test_case().p_name
       << ".log";

  bfs::path tgt = lut_file.str();
  sqeazy::quantiser_scheme<uint16_t,uint8_t> shrinker;
  std::vector<uint8_t> encoded(realistic_.size(),0);
  auto end_ptr = shrinker.encode(&realistic_[0],&encoded[0],realistic_.size());

  std::string shrinker_config = shrinker.config();

  BOOST_CHECK(end_ptr!=nullptr);
  //BOOST_CHECK_EQUAL(end_ptr-&encoded[0],realistic_.size());
  BOOST_CHECK_EQUAL(std::distance(&encoded[0],end_ptr),ptrdiff_t(realistic_.size()));

  std::vector<uint16_t> reconstructed(encoded.size(),0);
  std::vector<uint16_t> reloaded_decoded(encoded.size(),0);

  sqeazy::quantiser_scheme<uint16_t,uint8_t> reloaded(shrinker_config);
  std::string reloaded_config = reloaded.config();
  BOOST_CHECK_EQUAL_COLLECTIONS(shrinker_config.begin(), shrinker_config.end(),
                reloaded_config.begin(), reloaded_config.end());

  int err =reloaded.decode(&encoded[0],
               &reloaded_decoded[0],
               encoded.size());

  BOOST_CHECK_EQUAL(err,0);

  err =shrinker.decode(&encoded[0],
               &reconstructed[0],
               encoded.size());

  BOOST_CHECK_EQUAL(err,0);

  double mse = sqeazy::mse(reconstructed.begin(), reconstructed.end(),
               reloaded_decoded.begin());

  BOOST_TEST_MESSAGE(boost::unit_test::framework::current_test_case().p_name << ", mse = " << mse);


  BOOST_CHECK_GE(mse,0);
  BOOST_WARN_LT(mse,1.f);
  if(mse>1.){
    std::stringstream log_file;
    log_file << "loics_suite_"
         << boost::unit_test::framework::current_test_case().p_name
         << "_reconstructed_vs_reloaded.log";

    shrinker.dump(log_file.str());
    BOOST_TEST_MESSAGE(boost::unit_test::framework::current_test_case().p_name<< ", wrote " << log_file.str());
  }

}


BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ramps)

BOOST_AUTO_TEST_CASE( histogram_right ){

  std::vector<uint16_t> input(1 << 12,0);
  uint16_t value = 0;
  for( uint16_t& _el : input )
    _el = value++;

  sqeazy::quantiser<uint16_t,uint8_t> shrinker(input.data(),
                           input.data()+input.size()
                           );

  for(std::size_t i = 0;i<(1 << 12);++i)
    BOOST_REQUIRE_MESSAGE(shrinker.histo_[i]== 1, "\n["<<i<<"] "<<shrinker.histo_[i]<<" != 1 \n");

}

BOOST_AUTO_TEST_CASE( normal ){

  std::vector<uint16_t> input(1 << 12,0);
  uint16_t value = 0;
  for( uint16_t& _el : input )
    _el = value++;

  std::vector<uint8_t> encoded(input.size(),0);

  sqeazy::quantiser<uint16_t,uint8_t> shrinker(&input[0],
                           &input[0]+input.size()
                           );
  shrinker.encode(&input[0],input.size(),&encoded[0]);

  float raw_sum = std::accumulate(input.begin(), input.end(),0);
  float sum = std::accumulate(encoded.begin(), encoded.end(),0);
  BOOST_CHECK_NE(sum,0);
  //TODO!!
  BOOST_CHECK_CLOSE_FRACTION(sum,raw_sum/16,1);

  uint8_t max_value = *std::max_element(encoded.begin(), encoded.end());
  uint8_t max_expected = *std::max_element(input.begin(), input.end())/16;//16 is the bucketsize in this example

  try{
    BOOST_REQUIRE_CLOSE_FRACTION((float)max_value,(float)max_expected,5);
    BOOST_REQUIRE_EQUAL(input[0],encoded[0]);
    BOOST_REQUIRE_EQUAL(encoded[16],encoded[0]+1);
  }
  catch(...){
    std::stringstream log_file;
    log_file << "checkon_16bit_"
         << boost::unit_test::framework::current_test_case().p_name
         << ".log";

    shrinker.dump(log_file.str());
    throw;
  }
}

BOOST_AUTO_TEST_CASE( normal_scheme_api ){

  std::vector<uint16_t> input(1 << 12,0);
  uint16_t value = 0;
  for( uint16_t& _el : input )
    _el = value++;

  std::vector<uint8_t> encoded(input.size(),0);

  sqeazy::quantiser_scheme<uint16_t,uint8_t> quant;
  uint8_t* end = quant.encode(&input[0],&encoded[0], input.size());

  BOOST_CHECK(end!=nullptr);
  float raw_sum = std::accumulate(input.begin(), input.end(),0);
  float sum = std::accumulate(encoded.begin(), encoded.end(),0);
  BOOST_CHECK_NE(sum,0);
  //TODO!!
  BOOST_CHECK_CLOSE_FRACTION(sum,raw_sum/16,1);

  uint8_t max_value = *std::max_element(encoded.begin(), encoded.end());
  uint8_t max_expected = *std::max_element(input.begin(), input.end())/16;//16 is the bucketsize in this example

  BOOST_TEST_MESSAGE(boost::unit_test::framework::current_test_case().p_name << ", max_value " << (int)max_value
             << ", expected " << (int)max_expected);
  try{
    BOOST_REQUIRE_CLOSE_FRACTION((float)max_value,(float)max_expected,5);
    BOOST_REQUIRE_EQUAL(input[0],encoded[0]);
    BOOST_REQUIRE_EQUAL(encoded[16],encoded[0]+1);
  }
  catch(...){
    std::stringstream log_file;
    log_file << "checkon_16bit_"
         << boost::unit_test::framework::current_test_case().p_name
         << ".log";

    quant.dump(log_file.str());
    throw;
  }
}


BOOST_AUTO_TEST_CASE( ramp_roundtrip ){

  std::vector<uint16_t> input(1 << 12,0);
  uint16_t value = 0;
  for( uint16_t& _el : input )
    _el = value++;

  std::vector<uint8_t> encoded(input.size(),0);

  sqeazy::quantiser<uint16_t,uint8_t> shrinker(&input[0],&input[0]+input.size());
  shrinker.encode(&input[0],input.size(),&encoded[0]);

  std::vector<uint16_t> reconstructed(input.size(),0);
  shrinker.decode(&encoded[0],input.size(),&reconstructed[0]);

  try{
    BOOST_REQUIRE_EQUAL(reconstructed[0] ,8);
    BOOST_REQUIRE_EQUAL(reconstructed[1] ,8);
    BOOST_REQUIRE_EQUAL(reconstructed[15],8);
    BOOST_REQUIRE_NE   (reconstructed[16],8);
    BOOST_REQUIRE_EQUAL(reconstructed[reconstructed.size()-1],(1<<12) - 8);
  }
  catch(...){
    std::stringstream log_file;
    log_file << "failing_checkon_16bit_"
         << boost::unit_test::framework::current_test_case().p_name
         << ".log";

    shrinker.dump(log_file.str());
    throw;
  }

}

BOOST_AUTO_TEST_CASE( capped_ramp_roundtrip ){

  std::vector<uint16_t> input(1 << 12,0);
  uint16_t value = 0;
  for( uint16_t& _el : input )
    _el = (value++) % 63;

  std::vector<uint8_t> encoded(input.size(),0);

  sqeazy::quantiser<uint16_t,uint8_t> shrinker(&input[0],&input[0]+input.size());
  shrinker.encode(&input[0],input.size(),&encoded[0]);

  std::vector<uint16_t> reconstructed(input.size(),0);
  shrinker.decode(&encoded[0],input.size(),&reconstructed[0]);

  try{
    BOOST_REQUIRE_EQUAL_COLLECTIONS(input.begin(), input.end(),reconstructed.begin(), reconstructed.end());
  }
  catch(...){
    std::stringstream log_file;
    log_file << "checkon_16bit_"
         << boost::unit_test::framework::current_test_case().p_name
         << ".log";

    shrinker.dump(log_file.str());
    throw;
  }

}

BOOST_AUTO_TEST_CASE( decode_lut_yields_only_unique_values_wide_normal ){

  std::vector<uint16_t> input(1 << 12,0);
  std::random_device rd;
  std::mt19937 gen(rd());

  std::normal_distribution<> wide_normal(4048,500);

  for( uint16_t& _el : input )
    _el = std::round(wide_normal(gen));

  sqeazy::quantiser<uint16_t,uint8_t> shrinker(input.data(),
                           input.data() + input.size());

  std::vector<uint16_t> decode_lut(shrinker.lut_decode_.begin(), shrinker.lut_decode_.end());
  auto decode_lut_sorted = decode_lut;
  std::sort(decode_lut_sorted.begin(),decode_lut_sorted.end());
  auto new_end = std::unique(decode_lut_sorted.begin(), decode_lut_sorted.end());

  BOOST_CHECK_EQUAL(std::distance(decode_lut_sorted.begin(),new_end),std::ptrdiff_t(shrinker.lut_decode_.size()));

}

BOOST_AUTO_TEST_CASE( decode_lut_yields_only_unique_values ){

  std::vector<uint16_t> input(1 << 12,0);
  uint16_t value = 0;
  for( uint16_t& _el : input )
    _el = value++ ;

  sqeazy::quantiser<uint16_t,uint8_t> shrinker(input.data(),
                           input.data() + input.size());

  std::vector<uint16_t> decode_lut(shrinker.lut_decode_.begin(), shrinker.lut_decode_.end());
  auto decode_lut_sorted = decode_lut;
  std::sort(decode_lut_sorted.begin(),decode_lut_sorted.end());
  auto new_end = std::unique(decode_lut_sorted.begin(), decode_lut_sorted.end());

  //BOOST_CHECK_EQUAL(new_end - decode_lut_sorted.begin(),shrinker.lut_decode_.size());
  BOOST_CHECK_EQUAL(std::distance(decode_lut_sorted.begin(),new_end),std::ptrdiff_t(shrinker.lut_decode_.size()));
}

BOOST_AUTO_TEST_CASE( decode_lut_yields_only_unique_peaky_bucket ){

  std::vector<uint16_t> input(1 << 12,0);
  std::random_device rd;
  std::mt19937 gen(rd());

  std::normal_distribution<> peak(4048,.1);
  std::uniform_int_distribution<> background(0, std::numeric_limits<uint16_t>::max());


  for( uint16_t& _el : input )
    _el = std::round(peak(gen) + background(gen)) ;

  sqeazy::quantiser<uint16_t,uint8_t> shrinker(input.data(),
                           input.data() + input.size());

  std::vector<uint16_t> decode_lut(shrinker.lut_decode_.begin(), shrinker.lut_decode_.end());
  auto decode_lut_sorted = decode_lut;
  std::sort(decode_lut_sorted.begin(),decode_lut_sorted.end());
  auto new_end = std::unique(decode_lut_sorted.begin(), decode_lut_sorted.end());

  //BOOST_CHECK_EQUAL(new_end - decode_lut_sorted.begin(),shrinker.lut_decode_.size());
  BOOST_CHECK_EQUAL(std::distance(decode_lut_sorted.begin(),new_end),std::ptrdiff_t(shrinker.lut_decode_.size()));
}


BOOST_AUTO_TEST_SUITE_END()

template <typename T>
struct weighters_fixture {

  std::vector<T> constv;
  std::vector<T> output;
  std::vector<T> ramp;
  std::vector<T> ramp_w_gaps;

  constexpr static std::size_t ten_percent   = .1*std::numeric_limits<T>::max();
  constexpr static std::size_t half_elements = .5*std::numeric_limits<T>::max();


  weighters_fixture():
    constv	(std::numeric_limits<T>::max(),2),
    output	(std::numeric_limits<T>::max(),0),
    ramp    (std::numeric_limits<T>::max(),0),
    ramp_w_gaps	(std::numeric_limits<T>::max(),0)
  {

    std::size_t count = 0;
    for(T& el : ramp)
      el = count++;

    count = 0;
    for(T& el : ramp_w_gaps)
      if((count/5) % 2 == 0)
    el = count++;


  };

};

BOOST_AUTO_TEST_SUITE( weighters_api )

BOOST_AUTO_TEST_CASE( copy_ctor ){
  sqeazy::weighters::power_of first(2,11);

  sqeazy::weighters::power_of copied = first;
  BOOST_CHECK_EQUAL(first.exp_enum, copied.exp_enum);
  BOOST_CHECK_EQUAL(first.exp_denom, copied.exp_denom);

}

BOOST_AUTO_TEST_CASE( offset_copy_ctor ){

    sqeazy::weighters::offset_power_of first(2,42);
    sqeazy::weighters::offset_power_of copied = first;

    BOOST_CHECK_EQUAL(first.exp_enum, copied.exp_enum);
    BOOST_CHECK_EQUAL(first.exp_denom, copied.exp_denom);

}

BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( weighters_on_16bit , weighters_fixture<std::uint16_t> )


BOOST_AUTO_TEST_CASE( none ){

  sqeazy::weighters::none w;
  output = constv;
  w.transform(output.begin(),output.end());
  // for(std::size_t i = 0;i<constv.size();++i)
  //   output[i] = constv[i]*w(i, constv[i]);

  BOOST_CHECK_EQUAL_COLLECTIONS(constv.begin(), constv.begin()+.2*constv.size(),
                                output.begin(), output.begin()+.2*constv.size());

}


BOOST_AUTO_TEST_CASE( power_of_on_ramp ){

  sqeazy::weighters::power_of w(2);
  w.transform(output.begin(),output.end(), ramp.begin());

  BOOST_CHECK_NE(std::equal(ramp.begin(), ramp.end(),output.begin()),
                 true);

}

BOOST_AUTO_TEST_CASE( offset_power_of_on_ramp ){

  sqeazy::weighters::offset_power_of w(2);
  w.transform(output.begin(),output.end(),ramp.begin());

  BOOST_CHECK_NE(std::equal(ramp.begin(), ramp.end(),output.begin()),
         true);



}

BOOST_AUTO_TEST_CASE( offset_vs_no_offset_on_20percent ){

  sqeazy::weighters::offset_power_of off(2);
  sqeazy::weighters::power_of no(2);

  auto output_off = output;
  auto output_no = output;

  no.transform(output_no.begin(),output_no.end(),ramp.begin());
  // for(std::size_t i = 0;i<ramp.size();++i)
  //   output_no[i] = ramp[i]*no(i, ramp[i]);

  off.transform(output_off.begin(),output_off.end(),ramp.begin());
  // for(std::size_t i = 0;i<ramp.size();++i)
  //   output_off[i] = ramp[i]*off(i, ramp[i]);

  BOOST_REQUIRE_NE(std::equal(output_off.begin(), output_off.begin() + 2*ten_percent,
                            output_no.begin()),true);


}


BOOST_AUTO_TEST_CASE( offset_versus_no_offset ){

  sqeazy::weighters::offset_power_of off(2);
  sqeazy::weighters::power_of no(2);
  auto output_off = output;
  auto output_no = output;
  ramp[0] = 42;

  no.transform(output_no.begin(),output_no.end(),ramp.begin());
  off.transform(output_off.begin(),output_off.end(),ramp.begin());

  BOOST_CHECK_EQUAL_COLLECTIONS(output_off.begin(), output_off.end(),
                                output_no.begin(), output_no.end());



}

BOOST_AUTO_TEST_CASE( offset_versus_no_offset_on_2threads ){

  sqeazy::weighters::offset_power_of off(2);
  sqeazy::weighters::power_of no(2);
  auto output_off = output;
  auto output_no = output;
  ramp[0] = 42;

  no.transform(output_no.begin(),output_no.end(),ramp.begin(),2);
  off.transform(output_off.begin(),output_off.end(),ramp.begin(),2);

  BOOST_CHECK_EQUAL_COLLECTIONS(output_off.begin(), output_off.end(),
                                output_no.begin(), output_no.end());



}


BOOST_AUTO_TEST_CASE( offset_vs_no_offset_on_gaps ){

  sqeazy::weighters::offset_power_of off(2);
  sqeazy::weighters::power_of no(2);
  auto output_off = output;
  auto output_no = output;

  no.transform(output_no.begin(),output_no.end(),ramp_w_gaps.begin());
  off.transform(output_off.begin(),output_off.end(),ramp_w_gaps.begin());

  BOOST_CHECK_NE(off.first_nonzero_index,0u);
  BOOST_CHECK_NE(std::equal(output_off.begin(), output_off.begin() + 2*ten_percent,
                output_no.begin()),true);


}

BOOST_AUTO_TEST_CASE( offset_vs_no_offset_on_gaps_on_2threads ){

  sqeazy::weighters::offset_power_of off(2);
  sqeazy::weighters::power_of no(2);
  auto output_off = output;
  auto output_no = output;

  no.transform(output_no.begin(),output_no.end(),ramp_w_gaps.begin(),2);
  off.transform(output_off.begin(),output_off.end(),ramp_w_gaps.begin(),2);

  BOOST_CHECK_NE(off.first_nonzero_index,0u);
  BOOST_CHECK_NE(std::equal(output_off.begin(), output_off.begin() + 2*ten_percent,
                            output_no.begin()),true);


}


BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( weighters_on_8bit , weighters_fixture<std::uint8_t> )

  BOOST_AUTO_TEST_CASE( copy_ctor ){
  sqeazy::weighters::power_of first(2,11);

  sqeazy::weighters::power_of copied = first;
  BOOST_CHECK_EQUAL(first.exp_enum, copied.exp_enum);
  BOOST_CHECK_EQUAL(first.exp_denom, copied.exp_denom);

}

  BOOST_AUTO_TEST_CASE( offset_copy_ctor ){

    sqeazy::weighters::offset_power_of first(2,42);
    sqeazy::weighters::offset_power_of copied = first;

    BOOST_CHECK_EQUAL(first.exp_enum, copied.exp_enum);
    BOOST_CHECK_EQUAL(first.exp_denom, copied.exp_denom);

}


BOOST_AUTO_TEST_CASE( none ){

  sqeazy::weighters::none w;
  output = constv;
  w.transform(output.begin(),output.end());
  // for(std::size_t i = 0;i<constv.size();++i)
  //   output[i] = constv[i]*w(i, constv[i]);

  BOOST_CHECK_EQUAL_COLLECTIONS(constv.begin(), constv.begin()+.2*constv.size(),
                                output.begin(), output.begin()+.2*constv.size());

}


BOOST_AUTO_TEST_CASE( power_of_on_ramp ){

  sqeazy::weighters::power_of w(2);
  w.transform(output.begin(),output.end(), ramp.begin());

  BOOST_CHECK_NE(std::equal(ramp.begin(), ramp.end(),output.begin()),
                 true);

}

BOOST_AUTO_TEST_CASE( offset_power_of_on_ramp ){

  sqeazy::weighters::offset_power_of w(2);
  w.transform(output.begin(),output.end(),ramp.begin());

  BOOST_CHECK_NE(std::equal(ramp.begin(), ramp.end(),output.begin()),
         true);



}

BOOST_AUTO_TEST_CASE( offset_vs_no_offset_on_20percent ){

  sqeazy::weighters::offset_power_of off(2);
  sqeazy::weighters::power_of no(2);

  auto output_off = output;
  auto output_no = output;

  no.transform(output_no.begin(),output_no.end(),ramp.begin());
  // for(std::size_t i = 0;i<ramp.size();++i)
  //   output_no[i] = ramp[i]*no(i, ramp[i]);

  off.transform(output_off.begin(),output_off.end(),ramp.begin());
  // for(std::size_t i = 0;i<ramp.size();++i)
  //   output_off[i] = ramp[i]*off(i, ramp[i]);

  BOOST_REQUIRE_NE(std::equal(output_off.begin(), output_off.begin() + 2*ten_percent,
                            output_no.begin()),true);


}


BOOST_AUTO_TEST_CASE( offset_versus_no_offset ){

  sqeazy::weighters::offset_power_of off(2);
  sqeazy::weighters::power_of no(2);
  auto output_off = output;
  auto output_no = output;
  ramp[0] = 42;

  no.transform(output_no.begin(),output_no.end(),ramp.begin());
  off.transform(output_off.begin(),output_off.end(),ramp.begin());

  BOOST_CHECK_EQUAL_COLLECTIONS(output_off.begin(), output_off.end(),
                                output_no.begin(), output_no.end());



}

BOOST_AUTO_TEST_CASE( offset_versus_no_offset_on_2threads ){

  sqeazy::weighters::offset_power_of off(2);
  sqeazy::weighters::power_of no(2);
  auto output_off = output;
  auto output_no = output;
  ramp[0] = 42;

  no.transform(output_no.begin(),output_no.end(),ramp.begin(),2);
  off.transform(output_off.begin(),output_off.end(),ramp.begin(),2);

  BOOST_CHECK_EQUAL_COLLECTIONS(output_off.begin(), output_off.end(),
                                output_no.begin(), output_no.end());



}


BOOST_AUTO_TEST_CASE( offset_vs_no_offset_on_gaps ){

  sqeazy::weighters::offset_power_of off(2);
  sqeazy::weighters::power_of no(2);
  auto output_off = output;
  auto output_no = output;

  no.transform(output_no.begin(),output_no.end(),ramp_w_gaps.begin());
  off.transform(output_off.begin(),output_off.end(),ramp_w_gaps.begin());

  BOOST_CHECK_NE(off.first_nonzero_index,0u);
  BOOST_CHECK_NE(std::equal(output_off.begin(), output_off.begin() + 2*ten_percent,
                output_no.begin()),true);


}

BOOST_AUTO_TEST_CASE( offset_vs_no_offset_on_gaps_on_2threads ){

  sqeazy::weighters::offset_power_of off(2);
  sqeazy::weighters::power_of no(2);
  auto output_off = output;
  auto output_no = output;

  no.transform(output_no.begin(),output_no.end(),ramp_w_gaps.begin(),2);
  off.transform(output_off.begin(),output_off.end(),ramp_w_gaps.begin(),2);

  BOOST_CHECK_NE(off.first_nonzero_index,0u);
  BOOST_CHECK_NE(std::equal(output_off.begin(), output_off.begin() + 2*ten_percent,
                            output_no.begin()),true);


}


BOOST_AUTO_TEST_SUITE_END()


struct weighters_string_fixture {

  std::string none = "none";
  std::string pow2 = "power_of_2";
  std::string sqrt = "power_of_1_2";



};

BOOST_FIXTURE_TEST_SUITE( extract_weighters , weighters_string_fixture )

BOOST_AUTO_TEST_CASE( try_none ){

  auto res = sqeazy::extract_ratio(none);

  BOOST_CHECK_EQUAL(res.first,0);
  BOOST_CHECK_EQUAL(res.second,0);

}

BOOST_AUTO_TEST_CASE( power_of_2 ){

  auto res = sqeazy::extract_ratio(pow2);

  BOOST_CHECK_EQUAL(res.first,2);
  BOOST_CHECK_EQUAL(res.second,1);

}

BOOST_AUTO_TEST_CASE( sqrt_ratio ){

  auto res = sqeazy::extract_ratio(sqrt);

  BOOST_CHECK_EQUAL(res.first,1);
  BOOST_CHECK_EQUAL(res.second,2);

}

BOOST_AUTO_TEST_CASE( power_capped_ramp_compare_weights ){

  std::vector<uint16_t> input(1 << 12,0);
  uint16_t value = 0;
  const std::uint16_t max_value = 63;

  for( uint16_t& _el : input )
    _el = (value++) % max_value;

  std::vector<uint8_t> encoded(input.size(),0);
  std::vector<uint16_t> reconstructed(input.size(),0);

  sqeazy::weighters::power_of w(3,1);
  sqeazy::quantiser<uint16_t,uint8_t> weighted_shrinker(input.data(),input.data()+input.size(),
                                                        1,w);

  sqeazy::quantiser<uint16_t,uint8_t> unweighted_shrinker(input.data(),input.data()+input.size());

  // auto len_weights = unweighted_shrinker.weights_.size();
  for(std::size_t i = 1;i<max_value;++i)
  {
      try{
        BOOST_REQUIRE_CLOSE(std::pow(i,3),weighted_shrinker.weights_[i], 1);
      }
      catch(...){

        std::cerr << "[" << boost::unit_test::framework::current_test_case().p_name << "]\t"
                  << "unweighted weights: " << unweighted_shrinker.weights_[i]
                  << ", weighted weights by x^3: " << weighted_shrinker.weights_[i]
                  << " at " << i <<"\n";


        throw;
      }

      if(i<2 || !unweighted_shrinker.histo_[i])
        continue;
      try{
        BOOST_REQUIRE_NE(unweighted_shrinker.importance_[i],weighted_shrinker.importance_[i]);
      }
      catch(...){

        std::cerr << "[" << boost::unit_test::framework::current_test_case().p_name << "]\tat " << i << "\n\t"
                  << "unweighted : " << unweighted_shrinker.importance_[i]
                  << " (w = " << unweighted_shrinker.weights_[i] << ", h = " << unweighted_shrinker.histo_[i] << ")\n\t"
                  << "weighted   : " << weighted_shrinker.importance_[i]
                  << " (w = " << weighted_shrinker.weights_[i] << ", h = " << weighted_shrinker.histo_[i] << ")\n";


        throw;
      }

  }

}

BOOST_AUTO_TEST_CASE( offset_power_capped_ramp_compare_weights ){

  std::vector<uint16_t> input(1 << 12,0);
  uint16_t value = 0;
  const std::uint16_t max_value = 63;
  const std::uint16_t min_value = 10;
  for( uint16_t& _el : input ){
    if(value > min_value)
      _el = (value) % max_value;
    else{
      _el = min_value;
    }
    value++;
  }

  std::vector<uint8_t> encoded(input.size(),0);
  std::vector<uint16_t> reconstructed(input.size(),0);

  sqeazy::weighters::offset_power_of w(3,1);
  sqeazy::quantiser<uint16_t,uint8_t> weighted_shrinker(input.data(),input.data()+input.size(),
                                                        1,w);

  sqeazy::quantiser<uint16_t,uint8_t> unweighted_shrinker(input.data(),input.data()+input.size());

  // auto len_weights = unweighted_shrinker.weights_.size();
  for(std::size_t i = min_value-1;i<max_value;++i)
  {
      try{
        BOOST_REQUIRE_CLOSE(std::pow(i,3),weighted_shrinker.weights_[i], 1);
      }
      catch(...){

        std::cerr << "[" << boost::unit_test::framework::current_test_case().p_name << "]\t"
                  << "unweighted weights: " << unweighted_shrinker.weights_[i]
                  << ", weighted weights by x^3: " << weighted_shrinker.weights_[i]
                  << " at " << i <<"\n";


        throw;
      }

      if(!unweighted_shrinker.histo_[i])
        continue;
      try{
        BOOST_REQUIRE_NE(unweighted_shrinker.importance_[i],weighted_shrinker.importance_[i]);
      }
      catch(...){

        std::cerr << "[" << boost::unit_test::framework::current_test_case().p_name << "]\tat " << i << "\n\t"
                  << "unweighted : " << unweighted_shrinker.importance_[i]
                  << " (w = " << unweighted_shrinker.weights_[i] << ", h = " << unweighted_shrinker.histo_[i] << ")\n\t"
                  << "weighted   : " << weighted_shrinker.importance_[i]
                  << " (w = " << weighted_shrinker.weights_[i] << ", h = " << weighted_shrinker.histo_[i] << ")\n";


        throw;
      }

  }

}
BOOST_AUTO_TEST_SUITE_END()
