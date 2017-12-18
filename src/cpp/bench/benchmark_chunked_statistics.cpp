#define __BENCHMARK_FRAME_SHUFFLE_SCHEME_IMPL_CPP__

/*
  notes/TODOs:
  - [2017-02-03] performance of frame_shuffle appears to be stuck at 80 MB/s, first profiling pointed at boost::accumulators to be the cause
  - [2017-02-03] potential other bottleneck could be the single threaded std::sort containted therein

*/

#include <thread>

#include "sqeazy_algorithms.hpp"
#include "benchmark_fixtures.hpp"

typedef sqeazy::benchmark::dynamic_synthetic_data<> dynamic_default_fixture;

BENCHMARK_DEFINE_F(dynamic_default_fixture, single_thread)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  float value = 0.f;
  value = sqeazy::mean<float>(sinus_.data(),sinus_.data()+sinus_.size());
  value = 0.;

  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(value = sqeazy::mean<float>(sinus_.data(),sinus_.data()+sinus_.size()));
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(size_)*sizeof(sinus_.front()));
}

BENCHMARK_REGISTER_F(dynamic_default_fixture, single_thread)->Range(1 << 16,1 << 25);


BENCHMARK_DEFINE_F(dynamic_default_fixture, max_threads)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  int nthreads = std::thread::hardware_concurrency();

  float value = 0.f;
  value = sqeazy::mean<float>(sinus_.data(),sinus_.data()+sinus_.size());
  value = 0.;

  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(value = sqeazy::mean<float>(sinus_.data(),sinus_.data()+sinus_.size(), nthreads));
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(size_)*sizeof(sinus_.front()));
}

BENCHMARK_REGISTER_F(dynamic_default_fixture, max_threads)->UseRealTime()->Range(1 << 16,1 << 25);

BENCHMARK_MAIN();
