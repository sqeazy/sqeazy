#define __BENCHMARK_BITSWAP_SCHEME_IMPL_CPP__

#include "encoders/bitswap_scheme_impl.hpp"
#include "benchmark_fixtures.hpp"

typedef sqeazy::benchmark::static_synthetic_data<> default_fixture;

BENCHMARK_DEFINE_F(default_fixture, one_vs_two_threads)(benchmark::State& st) {

  sqeazy::bitswap_scheme<std::uint16_t> local;
  local.set_n_threads(st.threads);

  while (st.KeepRunning()) {
    local.encode(sin_data.data(),
                 output_data.data(),
                 size);
  }

}

BENCHMARK_REGISTER_F(default_fixture, one_vs_two_threads)->Threads(2)->Threads(1);

BENCHMARK_MAIN();
