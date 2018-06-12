#ifndef _VIDEO_BENCHMARK_FIXTURES_H_
#define _VIDEO_BENCHMARK_FIXTURES_H_

#include <vector>
#include <cmath>
#include <limits>
#include <fstream>
#include <sstream>
#include <random>

#include "sqeazy_common.hpp"
#include "traits.hpp"

#include "benchmark/benchmark.h"

namespace sqeazy {

  namespace benchmark {


    template <typename T = std::uint16_t
              >
    struct dynamic_synthetic_data : public ::benchmark::Fixture
    {

      sqeazy::vec_32algn_t<T> sinus_;
      sqeazy::vec_32algn_t<T> embryo_;
      sqeazy::vec_32algn_t<T> noisy_embryo_;
      sqeazy::vec_32algn_t<T> output_;

      std::vector<std::size_t> shape_;
      std::size_t size_;

      void setup(std::size_t _size){

        if(shape_.size()!=3)
          shape_.resize(3);

        shape_[sqeazy::row_major::x] = 1280;//x264 complains if given input data is not at least 720p
        shape_[sqeazy::row_major::y] = 720;
        shape_[sqeazy::row_major::z] = std::floor(_size/(shape_[sqeazy::row_major::y]*shape_[sqeazy::row_major::x]));

        size_ = std::accumulate(shape_.begin(), shape_.end(), 1, std::multiplies<std::size_t>());

        setup_from_size();

      }

      template <typename value_type>
      void setup(const std::vector<value_type>& _shape){

        shape_ = _shape;

        size_ = std::accumulate(shape_.begin(), shape_.end(), 1, std::multiplies<std::size_t>());

        setup_from_size();

      }

      void setup_from_size(){

        sinus_.clear();
        embryo_.clear();
        noisy_embryo_.clear();
        output_.clear();

        sinus_.resize(size_);
        embryo_.resize(size_);
        noisy_embryo_.resize(size_);
        output_.resize(size_);


        static const T     maxv  = .5f*std::numeric_limits<T>::max();

        //sinus_
        unsigned index = 0;
        for( T& _element : sinus_ ){
          _element = (index++) % maxv;
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        const T signal_intensity_ = std::numeric_limits<T>::max()*.6;
        std::exponential_distribution<> dis(1.f/(.01f*signal_intensity_));

        for(uint32_t idx = 0;idx < noisy_embryo_.size();++idx)
          noisy_embryo_.data()[idx] += dis(gen);

        float ellipsoid = 0;
        float dist_to_1 = 0;

        auto embryo_itr = embryo_.begin();
        auto noisy_itr = noisy_embryo_.begin();

        for(uint32_t z=0;z<shape_[sqeazy::row_major::z];z++)
          for(uint32_t y=0;y<shape_[sqeazy::row_major::y];y++)
            for(uint32_t x=0;x<shape_[sqeazy::row_major::x];x++){

              //shift to center and square
              float x_2 = (float(x) - shape_[sqeazy::row_major::x]/2.f);x_2 *= x_2;
              float y_2 = (float(y) - shape_[sqeazy::row_major::y]/2.f);y_2 *= y_2;
              float z_2 = (float(z) - shape_[sqeazy::row_major::z]/2.f);z_2 *= z_2;

              ellipsoid =  x_2/(shape_[sqeazy::row_major::x]*shape_[sqeazy::row_major::x]*.25*.25);
              ellipsoid += y_2/(shape_[sqeazy::row_major::y]*shape_[sqeazy::row_major::y]*.25*.25);
              ellipsoid += z_2/(shape_[sqeazy::row_major::z]*shape_[sqeazy::row_major::z]*.6*.6);

              dist_to_1 = std::abs(1 - ellipsoid);
              if ( dist_to_1 < .07 ){

                *(embryo_itr++) = signal_intensity_;
                *(noisy_itr++) += signal_intensity_;
              }

            }





      }

      dynamic_synthetic_data():
        sinus_(),
        embryo_(),
        noisy_embryo_(),
        output_(),
        shape_(3,0),
        size_()
      {

        setup(1 << 16);
      }

      void SetUp(const ::benchmark::State& st) {
        setup(st.range(0));
      }

      void TearDown(const ::benchmark::State&) {  }

      unsigned long data_in_byte() const {
        return size_*sizeof(T);
      }


      virtual void BenchmarkCase(::benchmark::State& st){
        setup(st.range(0));
      }

    };


    template <typename T = unsigned short,
              //obtained by `getconf -a /usr|grep -i CACHE` on a Intel(R) Core(TM) i7-3520M
              const unsigned cache_size_in_byte_as_exponent = 22
              >
    struct static_synthetic_data : public ::benchmark::Fixture
    {

      static const unsigned long size = (1 << (cache_size_in_byte_as_exponent+1))/sizeof(T);
      static const unsigned long size_in_byte = sizeof(T)*size;

      sqeazy::vec_32algn_t<T> sin_data;
      sqeazy::vec_32algn_t<T> output_data;
      std::vector<std::size_t> shape;

      std::size_t axis_length(int index = sqeazy::row_major::x ) const {
        return shape.at(index);
      }

      void fill_self(){

        static const T maxv = .25f*std::numeric_limits<T>::max();

        unsigned index = 0;
        for( T& _element : sin_data ){
          _element = (index++) % maxv;
        }

        shape[sqeazy::row_major::x] = 1280;
        shape[sqeazy::row_major::y] = 720;
        shape[sqeazy::row_major::z] = std::floor(size/(shape[sqeazy::row_major::y]*shape[sqeazy::row_major::x]));
      }

      static_synthetic_data():
        sin_data(size,0),
        output_data(sin_data),
        shape(3,0)
      {

        fill_self();


      }

      void SetUp(const ::benchmark::State& st) {
        //m = ConstructRandomMap(st.range(0));
        fill_self();

      }

      void TearDown(const ::benchmark::State&) {  }


      //copy-constructor: reinit everything (don't copy)
      static_synthetic_data(const static_synthetic_data& _rhs):
        sin_data(size,0),
        output_data(size,0),
        shape(3,0)
      {

        *this = _rhs;

      }

      //assignment: reinit everything (don't copy)
      static_synthetic_data& operator=(const static_synthetic_data& _rhs){

        if(this!=&_rhs){

          std::copy(_rhs.sin_data.begin(), _rhs.sin_data.end(), sin_data.begin());
          std::copy(_rhs.output_data.begin(), _rhs.output_data.end(), output_data.begin());
          std::copy(_rhs.shape.begin(), _rhs.shape.end(), shape.begin());

        }

        return *this;

      }

      unsigned long data_in_byte() const {
        return sin_data.size()*sizeof(T);
      }


      virtual void BenchmarkCase(::benchmark::State&){}

      friend std::ostream& operator<<(std::ostream& _cout, const static_synthetic_data& _self){
        _cout << "[x,y,z] "
              << _self.axis_length(sqeazy::row_major::x) <<"x"
              << _self.axis_length(sqeazy::row_major::y) <<"x"
              << _self.axis_length(sqeazy::row_major::z)
              <<" uint16 = " << _self.data_in_byte()/(1<<20) << " MB";
        return _cout;
      }

    };


    bool file_exists(const std::string& _path){
      std::ifstream f(_path.c_str());
      if (f.good()) {
        f.close();
        return true;
      } else {
        f.close();
        return false;
      }
    }



  }  // benchmarking

}

#endif /* _BENCH_FIXTURES_H_ */
