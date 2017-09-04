#define __BENCHMARK_BASE64_ENCODING_CPP__

#include <thread>
#include <string>
#include <cmath>
#include <iostream>

#include "base64.hpp"
#include "benchmark/benchmark.h"



struct synthetic_data : public ::benchmark::Fixture
{

  std::size_t size_;
  std::string input_;
  std::string output_;



  void setup(std::size_t _size){

    input_.clear();
    input_.resize(_size);
    output_.resize(std::ceil(_size/3.)*4);
    size_ = _size;

    if(_size == 0)
      return;

    char element = -127;
    for(char& el : input_)
      el = element++;

  }

  synthetic_data():
    size_(0),
    input_(),
    output_(){

    setup(size_);
  }

  void SetUp(const ::benchmark::State& st) {
    setup(st.range(0));
  }

  void TearDown(const ::benchmark::State&) {  }


  virtual void BenchmarkCase(::benchmark::State& st){
    setup(st.range(0));
  }

};


BENCHMARK_DEFINE_F(synthetic_data, BM_single_thread_boost)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_.begin(), output_.end(),' ');
    state.ResumeTiming();
    sqeazy::base64_impl(input_.data(),input_.data()+size_,
                        (char*)output_.data());
  }

  auto bytes_processed = int64_t(state.iterations()) *  int64_t(size_)*sizeof(input_.front());

  state.SetBytesProcessed(bytes_processed);
}

BENCHMARK_REGISTER_F(synthetic_data,BM_single_thread_boost)->Arg({1<<8})->Arg({1<<10})->Arg({1 << 12})->Arg({1 << 16})->Arg({4 << 20});;

BENCHMARK_DEFINE_F(synthetic_data, BM_single_thread_mine)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_.begin(), output_.end(),' ');
    state.ResumeTiming();
    sqeazy::my_base64_impl(input_.data(),input_.data()+size_,
                        (char*)output_.data());
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(size_)*sizeof(input_.front()));
}

BENCHMARK_REGISTER_F(synthetic_data,BM_single_thread_mine)->Arg({1<<8})->Arg({1<<10})->Arg({1 << 12})->Arg({1 << 16})->Arg({4 << 20});

BENCHMARK_DEFINE_F(synthetic_data, BM_single_thread_boost_decoding)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  sqeazy::my_base64_impl(input_.data(),input_.data()+size_,
                         (char*)output_.data());

  auto decoded = input_;

  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(decoded.begin(), decoded.end(),' ');
    state.ResumeTiming();

    sqeazy::debase64_impl(output_.data(),output_.data()+output_.size(),
                          (char*)decoded.data());
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(output_.size())*sizeof(output_.front()));
}

BENCHMARK_REGISTER_F(synthetic_data,BM_single_thread_boost_decoding)->Arg({1<<8})->Arg({1<<10})->Arg({1 << 12})->Arg({1 << 16})->Arg({4 << 20});

BENCHMARK_MAIN();
