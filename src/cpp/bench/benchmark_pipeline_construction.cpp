#define __BENCHMARK_PIPELINE_CONSTRUCTION_CPP__

#include <thread>
#include <string>

#include "dynamic_pipeline.hpp"
#include "test_dynamic_pipeline_impl.hpp"
#include "benchmark_fixtures.hpp"

static std::string def_string = "set_to(value=1073741824)->high_bits->square";

namespace sqeazy_testing {
  template <typename T>
  using dynamic_pipeline = sqeazy::dynamic_pipeline<T,filter_factory, sink_factory<T> >;
}

static void BM_create_pipeline(benchmark::State& state) {

  auto sink_pipe = sqeazy_testing::dynamic_pipeline<int>::from_string("set_to(value=1073741824)->high_bits->square");

  while (state.KeepRunning()) {
    // state.PauseTiming();

    // state.ResumeTiming();

    sink_pipe = sqeazy_testing::dynamic_pipeline<int>::from_string("set_to(value=1073741824)->high_bits->square");

  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          (def_string.size()));
}


BENCHMARK(BM_create_pipeline);

BENCHMARK_MAIN();
