#ifndef _BENCHMARK_FIXTURES_H_
#define _BENCHMARK_FIXTURES_H_

#include <vector>
#include <cmath>
#include <limits>
#include <fstream>
#include <sstream>

#include "traits.hpp"
#include "benchmark/benchmark.h"


namespace sqeazy {

  namespace benchmark {

    template <typename T = unsigned short,
              //obtained by `getconf -a /usr|grep -i CACHE` on a Intel(R) Core(TM) i7-3520M
              const unsigned cache_size_in_byte_as_exponent = 22
              >
    struct static_synthetic_data : public ::benchmark::Fixture
    {

      static const unsigned long size = (1 << (cache_size_in_byte_as_exponent+1))/sizeof(T);
      static const unsigned long size_in_byte = sizeof(T)*size;

      std::vector<T> sin_data;
      std::vector<T> output_data;
      std::vector<std::size_t> shape;

      unsigned long axis_length() const {
        return std::pow(size,1.f/3.f);
      }

      void fill_self(){
        float factor_frequency = .25f*axis_length();
        static const float scale = .25f*std::numeric_limits<T>::max();

        unsigned index = 0;
        for( T& _element : sin_data ){
          _element = scale*std::sin(factor_frequency*index++);
        }

        shape[sqeazy::row_major::x] = 1 << int(std::round(std::log2(std::pow(size,1/3.f))));
        shape[sqeazy::row_major::y] = shape[sqeazy::row_major::x];
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
        output_data(sin_data){

        *this = _rhs;

      }

      //assignment: reinit everything (don't copy)
      static_synthetic_data& operator=(const static_synthetic_data& _rhs){

        if(this!=&_rhs){

          std::copy(_rhs.sin_data.begin(), _rhs.sin_data.end(), sin_data.begin());
          std::copy(_rhs.output_data.begin(), _rhs.output_data.end(), output_data.begin());

        }

        return *this;

      }

      unsigned long data_in_byte() const {
        return sin_data.size()*sizeof(T);
      }

      friend std::ostream& operator<<(std::ostream& _cout, const static_synthetic_data& _self){
        _cout << _self.axis_length() <<"x"<< _self.axis_length() <<"x"<< _self.axis_length()
              <<" uint16 = " << _self.data_in_byte()/(1<<20) << " MB";
        return _cout;
      }

      virtual void BenchmarkCase(::benchmark::State&){}
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
