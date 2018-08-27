#define __BENCHMARK_FULL_PIPELINE_IMPL_CPP__

#include <thread>

#include "benchmark_fixtures.hpp"
#include "sqeazy_pipelines.hpp"

typedef sqeazy::benchmark::static_synthetic_data<> static_default_fixture;
typedef sqeazy::benchmark::dynamic_synthetic_data<> dynamic_default_fixture;

static const std::string default_lossless = "bitshuffle->lz4";
static const std::string default_lossy = "rmestbkrd->bitshuffle->lz4";



BENCHMARK_DEFINE_F(dynamic_default_fixture, single_thread_lossless)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  auto pipe = sqeazy::dypeline<std::uint16_t>::from_string(default_lossless);
  pipe.set_n_threads(1);

  std::size_t max_outbytes = pipe.max_encoded_size(size_in_bytes());
  output_.resize(max_outbytes/sizeof(std::uint16_t));

  benchmark::DoNotOptimize(pipe.encode(noisy_embryo_.data(),
               (char*)output_.data(),
               shape_));

  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_.begin(), output_.end(),0);
    state.ResumeTiming();

    benchmark::DoNotOptimize(pipe.encode(noisy_embryo_.data(),
               (char*)output_.data(),
               shape_));
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          size_in_bytes());
}

BENCHMARK_REGISTER_F(dynamic_default_fixture, single_thread_lossless)->Range(1 << 21,1 << 27);


BENCHMARK_DEFINE_F(dynamic_default_fixture, max_threads_lossless)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  auto pipe = sqeazy::dypeline<std::uint16_t>::from_string(default_lossless);
  pipe.set_n_threads(std::thread::hardware_concurrency());

  std::size_t max_outbytes = pipe.max_encoded_size(size_in_bytes());
  output_.resize(max_outbytes/sizeof(std::uint16_t));

  benchmark::DoNotOptimize(pipe.encode(noisy_embryo_.data(),
                                       (char*)output_.data(),
                                       shape_));

  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_.begin(), output_.end(),0);
    state.ResumeTiming();

    benchmark::DoNotOptimize(pipe.encode(noisy_embryo_.data(),
                                         (char*)output_.data(),
                                         shape_));
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          size_in_bytes());
}

BENCHMARK_REGISTER_F(dynamic_default_fixture, max_threads_lossless)->Range(1 << 21,1 << 27);


BENCHMARK_DEFINE_F(dynamic_default_fixture, single_thread_lossy)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  auto pipe = sqeazy::dypeline<std::uint16_t>::from_string(default_lossy);
  pipe.set_n_threads(1);

  std::size_t max_outbytes = pipe.max_encoded_size(size_in_bytes());
  output_.resize(max_outbytes/sizeof(std::uint16_t));

  benchmark::DoNotOptimize(pipe.encode(noisy_embryo_.data(),
               (char*)output_.data(),
               shape_));

  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_.begin(), output_.end(),0);
    state.ResumeTiming();

    benchmark::DoNotOptimize(pipe.encode(noisy_embryo_.data(),
               (char*)output_.data(),
               shape_));
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          size_in_bytes());
}

BENCHMARK_REGISTER_F(dynamic_default_fixture, single_thread_lossy)->Range(1 << 21,1 << 27);


BENCHMARK_DEFINE_F(dynamic_default_fixture, max_threads_lossy)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  auto pipe = sqeazy::dypeline<std::uint16_t>::from_string(default_lossy);
  pipe.set_n_threads(std::thread::hardware_concurrency());

  std::size_t max_outbytes = pipe.max_encoded_size(size_in_bytes());
  output_.resize(max_outbytes/sizeof(std::uint16_t));

  benchmark::DoNotOptimize(pipe.encode(noisy_embryo_.data(),
                                       (char*)output_.data(),
                                       shape_));

  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_.begin(), output_.end(),0);
    state.ResumeTiming();

    benchmark::DoNotOptimize(pipe.encode(noisy_embryo_.data(),
                                         (char*)output_.data(),
                                         shape_));
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          size_in_bytes());
}

BENCHMARK_REGISTER_F(dynamic_default_fixture, max_threads_lossy)->Range(1 << 21,1 << 27);

BENCHMARK_DEFINE_F(dynamic_default_fixture, max_threads_lossy_nobitswap)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  auto pipe = sqeazy::dypeline<std::uint16_t>::from_string("rmestbrkd->lz4");
  pipe.set_n_threads(std::thread::hardware_concurrency());

  std::size_t max_outbytes = pipe.max_encoded_size(size_in_bytes());
  output_.resize(max_outbytes/sizeof(std::uint16_t));

  benchmark::DoNotOptimize(pipe.encode(noisy_embryo_.data(),
                                       (char*)output_.data(),
                                       shape_));

  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_.begin(), output_.end(),0);
    state.ResumeTiming();

    benchmark::DoNotOptimize(pipe.encode(noisy_embryo_.data(),
                                         (char*)output_.data(),
                                         shape_));
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          size_in_bytes());
}

BENCHMARK_REGISTER_F(dynamic_default_fixture, max_threads_lossy_nobitswap)->Range(1 << 21,1 << 27);

BENCHMARK_MAIN();
