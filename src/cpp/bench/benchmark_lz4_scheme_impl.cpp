#define __BENCHMARK_QUANTISER_SCHEME_IMPL_CPP__

#include <thread>

#include "encoders/lz4.hpp"
#include "benchmark_fixtures.hpp"

typedef sqeazy::benchmark::static_synthetic_data<> static_default_fixture;
typedef sqeazy::benchmark::dynamic_synthetic_data<> dynamic_default_fixture;

BENCHMARK_DEFINE_F(static_default_fixture, one_thread)(benchmark::State& state) {

  sqeazy::lz4_scheme<std::uint16_t> local;
  local.set_n_threads(state.threads);

  std::size_t max_outbytes = local.max_encoded_size(size_in_bytes());
  output_data.resize(max_outbytes/sizeof(std::uint16_t));

  benchmark::DoNotOptimize(local.encode(sin_data.data(),
               (char*)output_data.data(),
               shape));

  while (state.KeepRunning()) {

    state.PauseTiming();
    std::fill(output_data.begin(), output_data.end(),0);
    state.ResumeTiming();

    benchmark::DoNotOptimize(local.encode(sin_data.data(),
                 (char*)output_data.data(),
                 shape));
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *size_in_bytes());
}

BENCHMARK_REGISTER_F(static_default_fixture, one_thread)->UseRealTime();

BENCHMARK_DEFINE_F(static_default_fixture, scalar_two_threads)(benchmark::State& state) {

  sqeazy::lz4_scheme<std::uint16_t> local;
  local.set_n_threads(2);

  std::size_t max_outbytes = local.max_encoded_size(size_in_bytes());
  output_data.resize(max_outbytes/sizeof(std::uint16_t));

  benchmark::DoNotOptimize(local.encode(sin_data.data(),
               (char*)output_data.data(),
               shape));


  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_data.begin(), output_data.end(),0);
    state.ResumeTiming();

    benchmark::DoNotOptimize(local.encode(sin_data.data(),
                 (char*)output_data.data(),
                 shape));
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *size_in_bytes());
}

BENCHMARK_REGISTER_F(static_default_fixture, scalar_two_threads)->UseRealTime();

BENCHMARK_DEFINE_F(dynamic_default_fixture, single_thread)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  sqeazy::lz4_scheme<std::uint16_t> local;
  local.set_n_threads(1);

  std::size_t max_outbytes = local.max_encoded_size(size_in_bytes());
  output_.resize(max_outbytes/sizeof(std::uint16_t));

  benchmark::DoNotOptimize(local.encode(sinus_.data(),
               (char*)output_.data(),
               shape_));

  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_.begin(), output_.end(),0);
    state.ResumeTiming();

    benchmark::DoNotOptimize(local.encode(sinus_.data(),
               (char*)output_.data(),
               shape_));
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          size_in_bytes());
}

BENCHMARK_REGISTER_F(dynamic_default_fixture, single_thread)->Range(1 << 16,1 << 25);

BENCHMARK_DEFINE_F(dynamic_default_fixture, two_threads)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  sqeazy::lz4_scheme<std::uint16_t> local;
  local.set_n_threads(2);

  std::size_t max_outbytes = local.max_encoded_size(size_in_bytes());
  output_.resize(max_outbytes/sizeof(std::uint16_t));

  benchmark::DoNotOptimize(local.encode(sinus_.data(),
               (char*)output_.data(),
               shape_));

  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_.begin(), output_.end(),0);
    state.ResumeTiming();

    benchmark::DoNotOptimize(local.encode(sinus_.data(),
               (char*)output_.data(),
               shape_));
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          size_in_bytes());
}

BENCHMARK_REGISTER_F(dynamic_default_fixture, two_threads)->UseRealTime()->Range(1 << 16,1 << 25);

BENCHMARK_DEFINE_F(dynamic_default_fixture, max_threads)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  int nthreads = std::thread::hardware_concurrency();
  sqeazy::lz4_scheme<std::uint16_t> local;
  local.set_n_threads(nthreads);

  std::size_t max_outbytes = local.max_encoded_size(size_in_bytes());
  output_.resize(max_outbytes/sizeof(std::uint16_t));

  benchmark::DoNotOptimize(local.encode(sinus_.data(),
               (char*)output_.data(),
               shape_));

  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_.begin(), output_.end(),0);
    state.ResumeTiming();

    benchmark::DoNotOptimize(local.encode(sinus_.data(),
               (char*)output_.data(),
               shape_));
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          size_in_bytes());
}

BENCHMARK_REGISTER_F(dynamic_default_fixture, max_threads)->UseRealTime()->Range(1 << 16,1 << 25);


BENCHMARK_DEFINE_F(dynamic_default_fixture, single_thread_1mb)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  sqeazy::lz4_scheme<std::uint16_t> local("framestep_kb=1024");
  local.set_n_threads(1);

  std::size_t max_outbytes = local.max_encoded_size(size_in_bytes());
  output_.resize(max_outbytes/sizeof(std::uint16_t));

  benchmark::DoNotOptimize(local.encode(sinus_.data(),
               (char*)output_.data(),
               shape_));

  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_.begin(), output_.end(),0);
    state.ResumeTiming();

    benchmark::DoNotOptimize(local.encode(sinus_.data(),
                 (char*)output_.data(),
                 shape_));
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          size_in_bytes());
}

BENCHMARK_REGISTER_F(dynamic_default_fixture, single_thread_1mb)->Range(1 << 16,1 << 25);

BENCHMARK_DEFINE_F(dynamic_default_fixture, two_threads_1mb)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  sqeazy::lz4_scheme<std::uint16_t> local("framestep_kb=1024");
  local.set_n_threads(2);

std::size_t max_outbytes = local.max_encoded_size(size_in_bytes());
  output_.resize(max_outbytes/sizeof(std::uint16_t));

  benchmark::DoNotOptimize(local.encode(sinus_.data(),
               (char*)output_.data(),
               shape_));

  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_.begin(), output_.end(),0);
    state.ResumeTiming();

    benchmark::DoNotOptimize(local.encode(sinus_.data(),
               (char*)output_.data(),
               shape_));
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          size_in_bytes());
}

BENCHMARK_REGISTER_F(dynamic_default_fixture, two_threads_1mb)->UseRealTime()->Range(1 << 16,1 << 25);

BENCHMARK_DEFINE_F(dynamic_default_fixture, max_threads_1mb)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  int nthreads = std::thread::hardware_concurrency();
  sqeazy::lz4_scheme<std::uint16_t> local("framestep_kb=1024");
  local.set_n_threads(nthreads);

std::size_t max_outbytes = local.max_encoded_size(size_in_bytes());
  output_.resize(max_outbytes/sizeof(std::uint16_t));

  benchmark::DoNotOptimize(local.encode(sinus_.data(),
               (char*)output_.data(),
               shape_));

  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(output_.begin(), output_.end(),0);
    state.ResumeTiming();

    benchmark::DoNotOptimize(local.encode(sinus_.data(),
               (char*)output_.data(),
               shape_));
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          size_in_bytes());
}

BENCHMARK_REGISTER_F(dynamic_default_fixture, max_threads_1mb)->UseRealTime()->Range(1 << 16,1 << 25);


BENCHMARK_MAIN();
