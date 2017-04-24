#define __BENCHMARK_RASTER_REORDER_SCHEME_IMPL_CPP__

/*
  notes:

*/

#include <thread>

#include "encoders/raster_reorder_scheme_impl.hpp"
#include "benchmark_fixtures.hpp"

typedef sqeazy::benchmark::static_synthetic_data<> static_default_fixture;
typedef sqeazy::benchmark::dynamic_synthetic_data<> dynamic_default_fixture;

BENCHMARK_DEFINE_F(static_default_fixture, one_thread)(benchmark::State& state) {

  sqeazy::raster_reorder_scheme<std::uint16_t> local;
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

// BENCHMARK_DEFINE_F(static_default_fixture, scalar_two_threads)(benchmark::State& state) {

//   sqeazy::raster_reorder_scheme<std::uint16_t> local;
//   local.set_n_threads(2);


//   local.encode(sin_data.data(),
//                output_data.data(),
//                shape);


//   while (state.KeepRunning()) {
//     state.PauseTiming();
//     std::fill(output_data.begin(), output_data.end(),0);
//     state.ResumeTiming();

//     local.encode(sin_data.data(),
//                  output_data.data(),
//                  shape);
//   }

//   state.SetBytesProcessed(int64_t(state.iterations()) *
//                           int64_t(sin_data.size())*sizeof(sin_data.front()));
// }

// BENCHMARK_REGISTER_F(static_default_fixture, scalar_two_threads)->UseRealTime();

// BENCHMARK_DEFINE_F(dynamic_default_fixture, single_thread)(benchmark::State& state) {


//   if (state.thread_index == 0) {
//     SetUp(state);
//   }

//   sqeazy::raster_reorder_scheme<std::uint16_t> local;
//   local.set_n_threads(1);


//   local.encode(sinus_.data(),
//                output_.data(),
//                shape_);

//   while (state.KeepRunning()) {
//     state.PauseTiming();
//     std::fill(output_.begin(), output_.end(),0);
//     state.ResumeTiming();

//     local.encode(sinus_.data(),
//                output_.data(),
//                shape_);
//   }

//   state.SetBytesProcessed(int64_t(state.iterations()) *
//                           int64_t(size_)*sizeof(sinus_.front()));
// }

// BENCHMARK_REGISTER_F(dynamic_default_fixture, single_thread)->Range(1 << 16,1 << 25);

// BENCHMARK_DEFINE_F(dynamic_default_fixture, two_threads)(benchmark::State& state) {


//   if (state.thread_index == 0) {
//     SetUp(state);
//   }

//   sqeazy::raster_reorder_scheme<std::uint16_t> local;
//   local.set_n_threads(2);


//   local.encode(sinus_.data(),
//                output_.data(),
//                shape_);

//   while (state.KeepRunning()) {
//     state.PauseTiming();
//     std::fill(output_.begin(), output_.end(),0);
//     state.ResumeTiming();

//     local.encode(sinus_.data(),
//                output_.data(),
//                shape_);
//   }

//   state.SetBytesProcessed(int64_t(state.iterations()) *
//                           int64_t(size_)*sizeof(sinus_.front()));
// }

// BENCHMARK_REGISTER_F(dynamic_default_fixture, two_threads)->UseRealTime()->Range(1 << 16,1 << 25);

// BENCHMARK_DEFINE_F(dynamic_default_fixture, max_threads)(benchmark::State& state) {


//   if (state.thread_index == 0) {
//     SetUp(state);
//   }

//   int nthreads = std::thread::hardware_concurrency();
//   sqeazy::raster_reorder_scheme<std::uint16_t> local;
//   local.set_n_threads(nthreads);


//   local.encode(sinus_.data(),
//                output_.data(),
//                shape_);

//   while (state.KeepRunning()) {
//     state.PauseTiming();
//     std::fill(output_.begin(), output_.end(),0);
//     state.ResumeTiming();

//     local.encode(sinus_.data(),
//                output_.data(),
//                shape_);
//   }

//   state.SetBytesProcessed(int64_t(state.iterations()) *
//                           int64_t(size_)*sizeof(sinus_.front()));
// }

// BENCHMARK_REGISTER_F(dynamic_default_fixture, max_threads)->UseRealTime()->Range(1 << 16,1 << 25);

BENCHMARK_MAIN();
