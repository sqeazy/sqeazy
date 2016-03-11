#define BOOST_TEST_MODULE TEST_QUANTISER_IMPL
#include "boost/test/unit_test.hpp"
#include <climits>
#include <vector>
#include <iostream>
#include <bitset>
#include <map>
#include <cstdint>

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

BOOST_AUTO_TEST_CASE( setup ){

  sqeazy::quantiser<uint16_t,uint8_t> shrinker;
  shrinker.setup(embryo_.data(),embryo_.num_elements());
  BOOST_CHECK(shrinker.sum_ != 0);
  BOOST_CHECK_NE(std::accumulate(shrinker.lut_encode_.begin(), shrinker.lut_encode_.end(),0),0);
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

BOOST_AUTO_TEST_CASE( embryo_roundtrip_newapi ){

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


BOOST_AUTO_TEST_CASE( noisy_embryo_roundtrip ){

  std::vector<uint8_t> encoded(noisy_embryo_.num_elements(),0);

  sqeazy::quantiser<uint16_t,uint8_t> shrinker(noisy_embryo_.data(),
					       noisy_embryo_.data() + noisy_embryo_.num_elements());
  shrinker.encode(noisy_embryo_.data(),noisy_embryo_.num_elements(),&encoded[0]);

  std::vector<uint16_t> reconstructed(encoded.size(),0);
  shrinker.decode(&encoded[0],
		  encoded.size(),
		  &reconstructed[0]);

  double l2norm = sqeazy::l2norm(reconstructed.begin(), reconstructed.end(),noisy_embryo_.data());

  BOOST_REQUIRE_GT(l2norm,0);
  BOOST_CHECK_LT(l2norm,10);

}

BOOST_AUTO_TEST_CASE( noisy_embryo_roundtrip_newapi ){

  std::vector<uint8_t> encoded(noisy_embryo_.num_elements(),0);

  sqeazy::quantiser_scheme<uint16_t,uint8_t> shrinker;
  shrinker.encode(noisy_embryo_.data(),&encoded[0],noisy_embryo_.num_elements());

  std::vector<uint16_t> reconstructed(encoded.size(),0);
  shrinker.decode(&encoded[0],
		  &reconstructed[0],
		  encoded.size());

  double l2norm = sqeazy::l2norm(reconstructed.begin(), reconstructed.end(),noisy_embryo_.data());

  BOOST_REQUIRE_GT(l2norm,0);
  BOOST_CHECK_LT(l2norm,10);
  BOOST_TEST_MESSAGE(boost::unit_test::framework::current_test_case().p_name << ", l2norm = " << l2norm);

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

  BOOST_CHECK_GT(lut.size(),0);
  
  std::vector<uint8_t> encoded(embryo_.num_elements(),0);
  shrinker.encode(embryo_.data(),embryo_.num_elements(),&encoded[0]);

  decltype(shrinker.lut_decode_) lut_decode;
  shrinker.lut_from_string(lut,lut_decode);
  
  std::vector<uint16_t> reconstructed(encoded.size(),0);
  shrinker.decode(lut_decode,
  		  &encoded[0],
  		  encoded.size(),
  		  &reconstructed[0]);

  BOOST_REQUIRE_EQUAL_COLLECTIONS(reconstructed.begin(), reconstructed.end(),embryo_.data(),
  				  embryo_.data()+ embryo_.num_elements());
  
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
  
  BOOST_CHECK_GT(dyn_range,1000);
  BOOST_CHECK_LT(dyn_range,3000);

  aligned_vector reconstructed(encoded.size(),0);
  shrinker.decode(&encoded[0],
		  encoded.size(),
		  &reconstructed[0]);

  double psnr = sqeazy::psnr(reconstructed.begin(), reconstructed.end(),
			     realistic_.begin());
  
  BOOST_CHECK_GT(psnr,0);
  
  float exp_deviation_per_value = dyn_range / (256.f);
  float exp_mse = exp_deviation_per_value*exp_deviation_per_value;

  double mse = sqeazy::mse(reconstructed.begin(), reconstructed.end(),
			   realistic_.begin());
  BOOST_CHECK_GE(mse,0);
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

  std::vector<uint8_t> encoded(realistic_.size(),0);
  shrinker.encode(&realistic_[0],realistic_.size(),&encoded[0]);

  
  std::vector<uint16_t> reconstructed(encoded.size(),0);
  shrinker.decode(lut_file.str(),
		  &encoded[0],
		  encoded.size(),
		  &reconstructed[0]);

  size_t dyn_range = sqeazy::dyn_range(realistic_.begin(), realistic_.end());
  float exp_deviation_per_value = dyn_range / (256.f);
  float exp_mse = exp_deviation_per_value*exp_deviation_per_value;
  
  double mse = sqeazy::mse(reconstructed.begin(), reconstructed.end(),
			   realistic_.begin());
  BOOST_CHECK_GE(mse,0);
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

BOOST_AUTO_TEST_CASE( realistic_roundtrip_newapi ){

  std::stringstream lut_file;
  lut_file << "loics_suite_"
	   << boost::unit_test::framework::current_test_case().p_name
	   << ".log";
    
  bfs::path tgt = lut_file.str(); 
  sqeazy::quantiser_scheme<uint16_t,uint8_t> shrinker;
  std::vector<uint8_t> encoded(realistic_.size(),0);
  auto end_ptr = shrinker.encode(&realistic_[0],&encoded[0],realistic_.size());

  BOOST_CHECK(end_ptr!=nullptr);
  BOOST_CHECK_EQUAL(end_ptr-&encoded[0],realistic_.size());
  
  std::vector<uint16_t> reconstructed(encoded.size(),0);
  int err =shrinker.decode(&encoded[0],
			   &reconstructed[0],
			   encoded.size());

  BOOST_CHECK_EQUAL(err,0);
  
  // shrinker.lut_to_file(lut_file.str(),shrinker.lut_decode_);

  // BOOST_REQUIRE(bfs::exists(tgt));
  // BOOST_REQUIRE_NE(bfs::file_size(tgt),0u);


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

  // BOOST_TEST_MESSAGE("loics_suite/" << boost::unit_test::framework::current_test_case().p_name
  // 		     << "\t"  << "mse = " << mse << ", exp-mse = " << exp_mse);


  // size_t reco_dyn_range = sqeazy::dyn_range(reconstructed.begin(), reconstructed.end());
  // BOOST_CHECK_LT(reco_dyn_range,dyn_range);

  // uint16_t min_val_original = *std::min_element(realistic_.begin(), realistic_.end());
  // uint16_t max_val_original = *std::max_element(realistic_.begin(), realistic_.end());
  
  // uint16_t min_val_reconstr = *std::min_element(reconstructed.begin(), reconstructed.end());
  // uint16_t max_val_reconstr = *std::max_element(reconstructed.begin(), reconstructed.end());

  // BOOST_CHECK_LE(max_val_reconstr,max_val_original);
  // BOOST_CHECK_GE(min_val_reconstr,min_val_original);

  // BOOST_TEST_MESSAGE("loics_suite/" << boost::unit_test::framework::current_test_case().p_name << " <original|reconstructed>"
  // 		     << "\t" << "min = " << min_val_original << " | " << min_val_reconstr
  // 		     << "\tmax = " << max_val_original << " | " << max_val_reconstr
  // 		     << "\tdrange = " << dyn_range << " | " << reco_dyn_range
  // 		     );

  // if(bfs::exists(tgt))
  //   bfs::remove(tgt);
  
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ramps)

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

BOOST_AUTO_TEST_CASE( normal_newapi ){

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
    BOOST_REQUIRE_EQUAL(reconstructed[0],input[0]);
    BOOST_REQUIRE_EQUAL(reconstructed[1],input[0]);
    BOOST_REQUIRE_EQUAL(reconstructed[15],input[0]);
    BOOST_REQUIRE_NE(reconstructed[16],input[0]);
    BOOST_REQUIRE_EQUAL(reconstructed[reconstructed.size()-1],input[reconstructed.size()-1-15]);
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

BOOST_AUTO_TEST_SUITE_END()
