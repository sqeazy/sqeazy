#define __BENCHMARK_DIFF_SCHEME_IMPL_CPP__

/*
  notes/TODOs:
  - [2017-02-03] performance of diff appears to be stuck at 80 MB/s, first profiling pointed at boost::accumulators to be the cause
  - [2017-02-03] potential other bottleneck could be the single threaded std::sort containted therein

*/

#include <thread>

#include "encoders/diff_scheme_impl.hpp"
#include "benchmark_fixtures.hpp"

typedef sqeazy::benchmark::static_synthetic_data<> static_default_fixture;
typedef sqeazy::benchmark::dynamic_synthetic_data<> dynamic_default_fixture;

BENCHMARK_DEFINE_F(static_default_fixture, one_thread)(benchmark::State& state) {

  sqeazy::diff_scheme<std::uint16_t> local;
  local.set_n_threads(state.threads);

  local.encode(sin_data.data(),
               output_data.data(),
               shape);

  while (state.KeepRunning()) {

    state.PauseTiming();
    std::fill(output_data.begin(), output_data.end(),0);
    state.ResumeTiming();

    local.encode(sin_data.data(),
                 output_data.data(),
                 shape);
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(sin_data.size())*sizeof(sin_data.front()));
}

BENCHMARK_REGISTER_F(static_default_fixture, one_thread)->UseRealTime();

BENCHMARK_DEFINE_F(static_default_fixture, scalar_two_threads)(benchmark::State& state) {

  sqeazy::diff_scheme<std::uint16_t> local;
  local.set_n_threads(2);


  local.encode(sin_data.data(),
               output_data.data(),
               shape);


  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_data.begin(), output_data.end(),0);
    state.ResumeTiming();

    local.encode(sin_data.data(),
                 output_data.data(),
                 shape);
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(sin_data.size())*sizeof(sin_data.front()));
}

BENCHMARK_REGISTER_F(static_default_fixture, scalar_two_threads)->UseRealTime();

// BENCHMARK_DEFINE_F(static_default_fixture, sse_one_vs_two_threads)(benchmark::State& state) {

//   //heat the caches
//   sqeazy::detail::sse_bitplane_reorder_encode<1>(sin_data.data(),
//                                                  output_data.data(),
//                                                  shape,
//                                                  state.threads);

//   while (state.KeepRunning()) {
//     state.PauseTiming();
//     std::fill(output_data.begin(), output_data.end(),0);
//     state.ResumeTiming();

//     sqeazy::detail::sse_bitplane_reorder_encode<1>(sin_data.data(),
//                                                       output_data.data(),
//                                                       sin_data.size(),
//                                                       state.threads);
//   }

//   state.SetBytesProcessed(int64_t(state.iterations()) *
//                           int64_t(sin_data.size())*sizeof(sin_data.front()));
// }

// BENCHMARK_REGISTER_F(static_default_fixture, sse_one_vs_two_threads)->Threads(2)->Threads(1)->UseRealTime();

BENCHMARK_DEFINE_F(dynamic_default_fixture, single_thread)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  sqeazy::diff_scheme<std::uint16_t> local;
  local.set_n_threads(1);


  local.encode(sinus_.data(),
               output_.data(),
               shape_);

  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_.begin(), output_.end(),0);
    state.ResumeTiming();

    local.encode(sinus_.data(),
               output_.data(),
               shape_);
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(size_)*sizeof(sinus_.front()));
}

BENCHMARK_REGISTER_F(dynamic_default_fixture, single_thread)->Range(1 << 16,1 << 25);

BENCHMARK_DEFINE_F(dynamic_default_fixture, two_threads)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  sqeazy::diff_scheme<std::uint16_t> local;
  local.set_n_threads(2);


  local.encode(sinus_.data(),
               output_.data(),
               shape_);

  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_.begin(), output_.end(),0);
    state.ResumeTiming();

    local.encode(sinus_.data(),
               output_.data(),
               shape_);
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(size_)*sizeof(sinus_.front()));
}

BENCHMARK_REGISTER_F(dynamic_default_fixture, two_threads)->UseRealTime()->Range(1 << 16,1 << 25);

BENCHMARK_DEFINE_F(dynamic_default_fixture, max_threads)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  int nthreads = std::thread::hardware_concurrency();
  sqeazy::diff_scheme<std::uint16_t> local;
  local.set_n_threads(nthreads);


  local.encode(sinus_.data(),
               output_.data(),
               shape_);

  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_.begin(), output_.end(),0);
    state.ResumeTiming();

    local.encode(sinus_.data(),
               output_.data(),
               shape_);
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(size_)*sizeof(sinus_.front()));
}

BENCHMARK_REGISTER_F(dynamic_default_fixture, max_threads)->UseRealTime()->Range(1 << 16,1 << 25);

// BENCHMARK_DEFINE_F(dynamic_default_fixture, max_threads)(benchmark::State& state) {


//   // if (state.thread_index == 0) {
//   //   SetUp(state);
//   // }
//   int nthreads = std::thread::hardware_concurrency();

//   //heat the caches
//   sqeazy::detail::sse_bitplane_reorder_encode<1>(sinus_.data(),
//                                                  output_.data(),
//                                                  sinus_.size(),
//                                                  nthreads);

//   while (state.KeepRunning()) {
//     state.PauseTiming();
//     std::fill(output_.begin(), output_.end(),0);
//     state.ResumeTiming();

//     sqeazy::detail::sse_bitplane_reorder_encode<1>(sinus_.data(),
//                                                    output_.data(),
//                                                    sinus_.size(),
//                                                    nthreads);
//   }

//   state.SetBytesProcessed(int64_t(state.iterations()) *
//                           int64_t(size_)*sizeof(sinus_.front()));
// }

// BENCHMARK_REGISTER_F(dynamic_default_fixture, max_threads)->UseRealTime()->Range(1 << 16,1 << 25);

BENCHMARK_MAIN();
