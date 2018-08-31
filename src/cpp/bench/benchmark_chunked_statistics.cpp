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
  value = sqeazy::mean<float>(noisy_embryo_.data(),noisy_embryo_.data()+noisy_embryo_.size());
  value = 0.;

  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(value = sqeazy::mean<float>(noisy_embryo_.data(),noisy_embryo_.data()+noisy_embryo_.size()));
    benchmark::ClobberMemory();
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(size_)*sizeof(noisy_embryo_.front()));
}

BENCHMARK_REGISTER_F(dynamic_default_fixture, single_thread)->Range(1 << 16,1 << 25);


BENCHMARK_DEFINE_F(dynamic_default_fixture, max_threads)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  int nthreads = std::thread::hardware_concurrency();

  float end = 0;
  float value = 0.f;
  value = sqeazy::mean<float>(noisy_embryo_.data(),noisy_embryo_.data()+noisy_embryo_.size(), nthreads);

  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(value = sqeazy::mean<float>(noisy_embryo_.data(),noisy_embryo_.data()+noisy_embryo_.size(), nthreads));
    benchmark::ClobberMemory();
    state.PauseTiming();
    end += value;
    state.ResumeTiming();

  }

  assert(end != 0.);
  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(size_)*sizeof(noisy_embryo_.front()));
}

BENCHMARK_REGISTER_F(dynamic_default_fixture, max_threads)->UseRealTime()->Range(1 << 16,1 << 25);

template <typename in_t, typename mean_t>
void chunked_mean(in_t _begin, in_t _end,
                  std::vector<mean_t>& _values,
                  std::size_t _chunksize,
                  int _nthreads = 1){

  const auto len = std::distance(_begin,_end);
  const std::size_t nchunks = (len + _chunksize -1)/_chunksize;

  if(_nthreads < 1)
    _nthreads = std::thread::hardware_concurrency();

  if(_nthreads == 1){
    for(std::size_t c = 0;c < nchunks;++c){
      auto beg = _begin+(c*_chunksize);
      auto lend = ((c+1)*_chunksize > (std::size_t)len) ? _end : (_begin+(c+1)*_chunksize);

      _values[c] = std::accumulate(beg,
                                   lend,
                                   mean_t(0),
                                   std::plus<mean_t>())/(len);
      _values[c] /= std::distance(beg,lend);
    }
  }
  else{

    auto results = _values.data();

#pragma omp parallel for schedule(static) private(_begin,results) num_threads(_nthreads)
    for(omp_size_type c=0; c<(omp_size_type)nchunks; c++){
        auto beg = _begin+(c*_chunksize);
        auto lend = ((c+1)*_chunksize > (std::size_t)len) ? _end : (_begin+(c+1)*_chunksize);
        results[c] = std::accumulate(beg,
                                     lend,
                                     mean_t(0),
                                     std::plus<mean_t>())/(len);
        results[c] /= std::distance(beg,lend);
      }


    }


}

BENCHMARK_DEFINE_F(dynamic_default_fixture, chunked_one_thread)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  int nthreads = 1;
  std::vector<float> means(shape_[sqeazy::row_major::z],0);
  std::size_t chunk_size = shape_[sqeazy::row_major::y]*shape_[sqeazy::row_major::x];

  while (state.KeepRunning()) {

    state.PauseTiming();
    std::fill(means.begin(),means.end(),0.);
    state.ResumeTiming();
    chunked_mean(noisy_embryo_.data(),noisy_embryo_.data()+noisy_embryo_.size(),
                                          means,
                                          chunk_size,
                                          nthreads);

    benchmark::ClobberMemory();
  }

  assert(means.front() != 0.);
  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(size_)*sizeof(noisy_embryo_.front()));
}
BENCHMARK_REGISTER_F(dynamic_default_fixture, chunked_one_thread)->UseRealTime()->Range(1 << 16,1 << 25);

BENCHMARK_DEFINE_F(dynamic_default_fixture, chunked_max_threads)(benchmark::State& state) {


  if (state.thread_index == 0) {
    SetUp(state);
  }

  int nthreads = std::thread::hardware_concurrency();
  std::vector<float> means(shape_[sqeazy::row_major::z],0);
  std::size_t chunk_size = shape_[sqeazy::row_major::y]*shape_[sqeazy::row_major::x];

  while (state.KeepRunning()) {

    state.PauseTiming();
    std::fill(means.begin(),means.end(),0.);
    state.ResumeTiming();
    chunked_mean(noisy_embryo_.data(),
                 noisy_embryo_.data()+noisy_embryo_.size(),
                 means,
                 chunk_size,
                 nthreads);
    benchmark::ClobberMemory();
  }

  assert(means.front() != 0.);
  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(size_)*sizeof(noisy_embryo_.front()));
}
BENCHMARK_REGISTER_F(dynamic_default_fixture, chunked_max_threads)->UseRealTime()->Range(1 << 16,1 << 25);

BENCHMARK_MAIN();
