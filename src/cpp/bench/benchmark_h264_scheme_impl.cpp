#define __BENCHMARK_H264_IMPL_CPP__

#include <thread>

#include "encoders/h264.hpp"
#include "video_benchmark_fixtures.hpp"

/*
  notes/TODOs:
  - input data needs to be refactored so that width % 4 == 0 and anything else is %2!

*/


typedef sqeazy::benchmark::static_synthetic_data<std::uint8_t> static_default_fixture;
typedef sqeazy::benchmark::dynamic_synthetic_data<std::uint8_t> dynamic_default_fixture;

BENCHMARK_DEFINE_F(static_default_fixture, one_thread)(benchmark::State& state) {

  sqeazy::h264_scheme<std::uint8_t> local;
  local.set_n_threads(state.threads);

  local.encode(sin_data.data(),
               (char*)output_data.data(),
               shape);

  while (state.KeepRunning()) {

    state.PauseTiming();
    std::fill(output_data.begin(), output_data.end(),0);
    state.ResumeTiming();

    local.encode(sin_data.data(),
                 (char*)output_data.data(),
                 shape);
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(sin_data.size())*sizeof(sin_data.front()));
}

BENCHMARK_REGISTER_F(static_default_fixture, one_thread)->UseRealTime();

BENCHMARK_DEFINE_F(static_default_fixture, scalar_two_threads)(benchmark::State& state) {

  sqeazy::h264_scheme<std::uint8_t> local;
  local.set_n_threads(2);


  local.encode(sin_data.data(),
               (char*)output_data.data(),
               shape);


  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_data.begin(), output_data.end(),0);
    state.ResumeTiming();

    local.encode(sin_data.data(),
                 (char*)output_data.data(),
                 shape);
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(sin_data.size())*sizeof(sin_data.front()));
}

BENCHMARK_REGISTER_F(static_default_fixture, scalar_two_threads)->UseRealTime();

BENCHMARK_DEFINE_F(dynamic_default_fixture, single_thread)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  sqeazy::h264_scheme<std::uint8_t> local;
  local.set_n_threads(1);


  local.encode(sinus_.data(),
               (char*)output_.data(),
               shape_);

  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_.begin(), output_.end(),0);
    state.ResumeTiming();

    local.encode(sinus_.data(),
               (char*)output_.data(),
               shape_);
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(size_)*sizeof(sinus_.front()));
}

BENCHMARK_REGISTER_F(dynamic_default_fixture, single_thread)->Range(921600,1 << 25);

BENCHMARK_DEFINE_F(dynamic_default_fixture, two_threads)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  sqeazy::h264_scheme<std::uint8_t> local;
  local.set_n_threads(2);


  local.encode(sinus_.data(),
               (char*)output_.data(),
               shape_);

  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_.begin(), output_.end(),0);
    state.ResumeTiming();

    local.encode(sinus_.data(),
               (char*)output_.data(),
               shape_);
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(size_)*sizeof(sinus_.front()));
}

BENCHMARK_REGISTER_F(dynamic_default_fixture, two_threads)->UseRealTime()->Range(921600,1 << 25);

BENCHMARK_DEFINE_F(dynamic_default_fixture, max_threads)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  int nthreads = std::thread::hardware_concurrency();
  sqeazy::h264_scheme<std::uint8_t> local;
  local.set_n_threads(nthreads);


  local.encode(sinus_.data(),
               (char*)output_.data(),
               shape_);

  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_.begin(), output_.end(),0);
    state.ResumeTiming();

    local.encode(sinus_.data(),
               (char*)output_.data(),
               shape_);
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(size_)*sizeof(sinus_.front()));
}

BENCHMARK_REGISTER_F(dynamic_default_fixture, max_threads)->UseRealTime()->Range(921600,1 << 25);

BENCHMARK_MAIN();
