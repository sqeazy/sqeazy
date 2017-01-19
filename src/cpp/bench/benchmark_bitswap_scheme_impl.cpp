#define __BENCHMARK_BITSWAP_SCHEME_IMPL_CPP__

#include "encoders/bitswap_scheme_impl.hpp"
#include "benchmark_fixtures.hpp"

typedef sqeazy::benchmark::static_synthetic_data<> static_default_fixture;

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


BENCHMARK_MAIN();
