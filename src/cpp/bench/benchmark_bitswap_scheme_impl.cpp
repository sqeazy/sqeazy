#define __BENCHMARK_BITSWAP_SCHEME_IMPL_CPP__

#include <thread>

#include "encoders/bitswap_scheme_impl.hpp"
#include "benchmark_fixtures.hpp"

typedef sqeazy::benchmark::static_synthetic_data<> static_default_fixture;
typedef sqeazy::benchmark::dynamic_synthetic_data<> dynamic_default_fixture;

BENCHMARK_DEFINE_F(static_default_fixture, one_vs_two_threads)(benchmark::State& state) {

  sqeazy::bitswap_scheme<std::uint16_t> local;
  local.set_n_threads(state.threads);

  local.encode(sin_data.data(),
                 output_data.data(),
                 size);

  while (state.KeepRunning()) {

    state.PauseTiming();
    std::fill(output_data.begin(), output_data.end(),0);
    state.ResumeTiming();

    local.encode(sin_data.data(),
                 output_data.data(),
                 size);
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(sin_data.size())*sizeof(sin_data.front()));
}

BENCHMARK_REGISTER_F(static_default_fixture, one_vs_two_threads)->Threads(2)->Threads(1)->UseRealTime();

BENCHMARK_DEFINE_F(static_default_fixture, scalar_one_vs_two_threads)(benchmark::State& state) {

  sqeazy::bitswap_scheme<std::uint16_t> local;
  local.set_n_threads(state.threads);


  //heat the caches
  sqeazy::detail::scalar_bitplane_reorder_encode<1>(sin_data.data(),
                                                    output_data.data(),
                                                    sin_data.size(),
                                                    state.threads);

  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_data.begin(), output_data.end(),0);
    state.ResumeTiming();

    sqeazy::detail::scalar_bitplane_reorder_encode<1>(sin_data.data(),
                                                      output_data.data(),
                                                      sin_data.size(),
                                                      state.threads);
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(sin_data.size())*sizeof(sin_data.front()));
}

BENCHMARK_REGISTER_F(static_default_fixture, scalar_one_vs_two_threads)->Threads(2)->Threads(1)->UseRealTime();

BENCHMARK_DEFINE_F(static_default_fixture, sse_one_vs_two_threads)(benchmark::State& state) {

  //heat the caches
  sqeazy::detail::sse_bitplane_reorder_encode<1>(sin_data.data(),
                                                 output_data.data(),
                                                 sin_data.size(),
                                                 state.threads);

  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_data.begin(), output_data.end(),0);
    state.ResumeTiming();

    sqeazy::detail::sse_bitplane_reorder_encode<1>(sin_data.data(),
                                                      output_data.data(),
                                                      sin_data.size(),
                                                      state.threads);
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(sin_data.size())*sizeof(sin_data.front()));
}

BENCHMARK_REGISTER_F(static_default_fixture, sse_one_vs_two_threads)->Threads(2)->Threads(1)->UseRealTime();

BENCHMARK_DEFINE_F(dynamic_default_fixture, single_thread)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  //heat the caches
  sqeazy::detail::sse_bitplane_reorder_encode<1>(sinus_.data(),
                                                 output_.data(),
                                                 sinus_.size(),
                                                 state.threads);

  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_.begin(), output_.end(),0);
    state.ResumeTiming();

    sqeazy::detail::sse_bitplane_reorder_encode<1>(sinus_.data(),
                                                   output_.data(),
                                                   sinus_.size(),
                                                   state.threads);
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(size_)*sizeof(sinus_.front()));
}

BENCHMARK_REGISTER_F(dynamic_default_fixture, single_thread)->Range(1 << 16,1 << 25);

BENCHMARK_DEFINE_F(dynamic_default_fixture, two_threads)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  //heat the caches
  sqeazy::detail::sse_bitplane_reorder_encode<1>(sinus_.data(),
                                                 output_.data(),
                                                 sinus_.size(),
                                                 2);

  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_.begin(), output_.end(),0);
    state.ResumeTiming();

    sqeazy::detail::sse_bitplane_reorder_encode<1>(sinus_.data(),
                                                   output_.data(),
                                                   sinus_.size(),
                                                   2);
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(size_)*sizeof(sinus_.front()));
}

BENCHMARK_REGISTER_F(dynamic_default_fixture, two_threads)->UseRealTime()->Range(1 << 16,1 << 25);

BENCHMARK_DEFINE_F(dynamic_default_fixture, max_threads)(benchmark::State& state) {


  // if (state.thread_index == 0) {
  //   SetUp(state);
  // }
  int nthreads = std::thread::hardware_concurrency();

  //heat the caches
  sqeazy::detail::sse_bitplane_reorder_encode<1>(sinus_.data(),
                                                 output_.data(),
                                                 sinus_.size(),
                                                 nthreads);

  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_.begin(), output_.end(),0);
    state.ResumeTiming();

    sqeazy::detail::sse_bitplane_reorder_encode<1>(sinus_.data(),
                                                   output_.data(),
                                                   sinus_.size(),
                                                   nthreads);
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(size_)*sizeof(sinus_.front()));
}

BENCHMARK_REGISTER_F(dynamic_default_fixture, max_threads)->UseRealTime()->Range(1 << 16,1 << 25);

BENCHMARK_MAIN();
