#define __BENCHMARK_HISTOGRAM_UTILS_CPP__

#include <thread>

#include "hist_impl.hpp"
#include "benchmark_fixtures.hpp"

typedef sqeazy::benchmark::dynamic_synthetic_data<> uint16_fixture;
typedef sqeazy::benchmark::dynamic_synthetic_data<std::uint8_t> uint8_fixture;

BENCHMARK_DEFINE_F(uint8_fixture, serial_histogram_fill_uint8)(benchmark::State& state) {

  if (state.thread_index == 0) {
    SetUp(state);
  }

  typedef typename std::iterator_traits<decltype(sinus_.begin())>::value_type value_t;

  sqeazy::histogram<value_t> local;
  local.set_n_threads(1);

  local.fill_from_image(sinus_.begin(),sinus_.end());

  while (state.KeepRunning()) {
    state.PauseTiming();
    local.clear();
    state.ResumeTiming();

    local.fill_from_image(sinus_.begin(),sinus_.end());
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(size_)*sizeof(sinus_.front()));
}


BENCHMARK_DEFINE_F(uint16_fixture, serial_histogram_fill_uint16)(benchmark::State& state) {

  if (state.thread_index == 0) {
    SetUp(state);
  }

  typedef typename std::iterator_traits<decltype(sinus_.begin())>::value_type value_t;

  sqeazy::histogram<value_t> local;
  local.set_n_threads(1);

  local.fill_from_image(sinus_.begin(),sinus_.end());

  while (state.KeepRunning()) {
    state.PauseTiming();
    local.clear();
    state.ResumeTiming();

    local.fill_from_image(sinus_.begin(),sinus_.end());
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(size_)*sizeof(sinus_.front()));
}

BENCHMARK_REGISTER_F(uint16_fixture, serial_histogram_fill_uint16)

->Args({4 << 10})//L1 cache
->Args({(48) << 10})//L2 cache
->Args({(480) << 10})//L3 cache
->Args({48 << 20})//RAM cache
;

BENCHMARK_REGISTER_F(uint8_fixture, serial_histogram_fill_uint8)

->Args({4 << 10})//L1 cache
->Args({(48) << 10})//L2 cache
->Args({(480) << 10})//L3 cache
->Args({48 << 20})//RAM cache
;
//->Range(1 << 16,1 << 25);


BENCHMARK_DEFINE_F(uint8_fixture, parallel_histogram_fill_uint8)(benchmark::State& state) {

  if (state.thread_index == 0) {
    SetUp(state);
  }

  typedef typename std::iterator_traits<decltype(sinus_.begin())>::value_type value_t;

  sqeazy::histogram<value_t> local;
  local.set_n_threads(std::thread::hardware_concurrency());

  local.fill_from_image(sinus_.begin(),sinus_.end());

  while (state.KeepRunning()) {
    state.PauseTiming();
    local.clear();
    state.ResumeTiming();

    local.fill_from_image(sinus_.begin(),sinus_.end());
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(size_)*sizeof(sinus_.front()));
}


BENCHMARK_DEFINE_F(uint16_fixture, parallel_histogram_fill_uint16)(benchmark::State& state) {

  if (state.thread_index == 0) {
    SetUp(state);
  }

  typedef typename std::iterator_traits<decltype(sinus_.begin())>::value_type value_t;

  sqeazy::histogram<value_t> local;
  local.set_n_threads(std::thread::hardware_concurrency());

  local.fill_from_image(sinus_.begin(),sinus_.end());

  while (state.KeepRunning()) {
    state.PauseTiming();
    local.clear();
    state.ResumeTiming();

    local.fill_from_image(sinus_.begin(),sinus_.end());
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(size_)*sizeof(sinus_.front()));
}

BENCHMARK_REGISTER_F(uint16_fixture, parallel_histogram_fill_uint16)
->Args({4 << 10})//L1 cache
->Args({(48) << 10})//L2 cache
->Args({(480) << 10})//L3 cache
->Args({48 << 20})//RAM cache
;

BENCHMARK_REGISTER_F(uint8_fixture, parallel_histogram_fill_uint8)
->Args({4 << 10})//L1 cache
->Args({(48) << 10})//L2 cache
->Args({(480) << 10})//L3 cache
->Args({48 << 20})//RAM cache
;
//->Range(1 << 16,1 << 25);

template <typename iter_t>
void fill_beginend(iter_t begin, iter_t end){

  typedef typename std::iterator_traits<iter_t>::value_type value_t;

  const std::size_t len = std::distance(begin,end);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::normal_distribution<> wide_normal(std::numeric_limits<value_t>::max()/2,std::numeric_limits<value_t>::max()*.1);
  if(len < 2*64){
    std::generate(begin, end,
                  [&]{
                    return std::round(wide_normal(gen));
                  });
  }
  else {
  std::generate(begin, begin+64,
        [&]{
          return std::round(wide_normal(gen));
        });

  std::generate(begin + len -64, end,
                [&]{
                  return std::round(wide_normal(gen));
                });
  }
}


template <typename value_t>
static void BM_serial_sum(benchmark::State& state) {

  std::vector<value_t> src(state.range(0),43u);

  fill_beginend(src.begin(), src.end());

  float sum = sqeazy::detail::sum(src.begin(), src.end(), 0.f);
  while (state.KeepRunning()){
    benchmark::DoNotOptimize(sum = std::accumulate(src.begin(), src.end(), 0.f));
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(state.range(0)*sizeof(value_t)));
}

BENCHMARK_TEMPLATE(BM_serial_sum,std::uint32_t)
->Args({256    })
->Args({std::numeric_limits<std::uint16_t>::max()})
->Args({48 << 20})
;


template <typename value_t>
static void BM_parallel_sum(benchmark::State& state) {

  std::vector<value_t> src(state.range(0),43u);

  fill_beginend(src.begin(), src.end());

  float sum = sqeazy::detail::sum(src.begin(), src.end(), 0.f);
  while (state.KeepRunning()){
    benchmark::DoNotOptimize(sum = sqeazy::detail::sum(src.begin(), src.end(), 0.f));
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(state.range(0))*sizeof(value_t));
}

BENCHMARK_TEMPLATE(BM_parallel_sum,std::uint32_t)
->Args({256    })
->Args({std::numeric_limits<std::uint16_t>::max()})
->Args({48 << 20})

;

template <typename value_t>
static void BM_serial_max_element(benchmark::State& state) {

  std::vector<value_t> src(state.range(0),43u);

  fill_beginend(src.begin(), src.end());


  std::size_t index = sqeazy::detail::max_element_distance(src.begin(), src.end(), 1);
  while (state.KeepRunning()){
    benchmark::DoNotOptimize(index = sqeazy::detail::max_element_distance(src.begin(), src.end(), 1));
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(state.range(0)*sizeof(value_t)));
}

BENCHMARK_TEMPLATE(BM_serial_max_element,std::uint32_t)
->Args({256    })
->Args({std::numeric_limits<std::uint16_t>::max()})
->Args({48 << 20})
;


template <typename value_t>
static void BM_parallel_max_element(benchmark::State& state) {

  std::vector<value_t> src(state.range(0),43u);

  fill_beginend(src.begin(), src.end());

  std::size_t index = sqeazy::detail::max_element_distance(src.begin(), src.end());
  while (state.KeepRunning()){
    benchmark::DoNotOptimize(index = sqeazy::detail::max_element_distance(src.begin(), src.end()));
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(state.range(0))*sizeof(value_t));
}

BENCHMARK_TEMPLATE(BM_parallel_max_element,std::uint32_t)
->Args({256    })
->Args({std::numeric_limits<std::uint16_t>::max()})
->Args({48 << 20})

;


template <typename value_t>
static void BM_serial_index_weighted_sum(benchmark::State& state) {

  std::vector<value_t> src(state.range(0),43u);

  fill_beginend(src.begin(), src.end());

  float index_weighted_sum = sqeazy::detail::index_weighted_sum(src.begin(), src.end(), 0.f,0,1);
  while (state.KeepRunning()){
    benchmark::DoNotOptimize(index_weighted_sum = sqeazy::detail::index_weighted_sum(src.begin(), src.end(), 0.f,0,1));
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(state.range(0)*sizeof(value_t)));
}

BENCHMARK_TEMPLATE(BM_serial_index_weighted_sum,std::uint32_t)
->Args({256    })
->Args({std::numeric_limits<std::uint16_t>::max()})
->Args({48 << 20})
;


template <typename value_t>
static void BM_parallel_index_weighted_sum(benchmark::State& state) {

  std::vector<value_t> src(state.range(0),43u);

  fill_beginend(src.begin(), src.end());

  float index_weighted_sum = sqeazy::detail::index_weighted_sum(src.begin(), src.end(), 0.f, 0);
  while (state.KeepRunning()){
    benchmark::DoNotOptimize(index_weighted_sum = sqeazy::detail::index_weighted_sum(src.begin(), src.end(), 0.f,0));
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(state.range(0))*sizeof(value_t));
}

BENCHMARK_TEMPLATE(BM_parallel_index_weighted_sum,std::uint32_t)
->Args({256    })
->Args({std::numeric_limits<std::uint16_t>::max()})
->Args({48 << 20})

;

template <typename value_t>
static void BM_serial_unnormalized_variation(benchmark::State& state) {

  std::vector<value_t> src(state.range(0),43u);

  fill_beginend(src.begin(), src.end());

  float unnormalized_variation = sqeazy::detail::unnormalized_variation(src.begin(), src.end(), 0.f,43.f,0,1);
  while (state.KeepRunning()){
    benchmark::DoNotOptimize(unnormalized_variation = sqeazy::detail::unnormalized_variation(src.begin(), src.end(), 0.f,43.f,0,1));
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(state.range(0)*sizeof(value_t)));
}

BENCHMARK_TEMPLATE(BM_serial_unnormalized_variation,std::uint32_t)
->Args({256    })
->Args({std::numeric_limits<std::uint16_t>::max()})
->Args({48 << 20})
;


template <typename value_t>
static void BM_parallel_unnormalized_variation(benchmark::State& state) {

  std::vector<value_t> src(state.range(0),43u);

  fill_beginend(src.begin(), src.end());

  float unnormalized_variation = sqeazy::detail::unnormalized_variation(src.begin(), src.end(), 0.f,43.f, 0);
  while (state.KeepRunning()){
    benchmark::DoNotOptimize(unnormalized_variation = sqeazy::detail::unnormalized_variation(src.begin(), src.end(), 0.f,43.f,0));
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(state.range(0))*sizeof(value_t));
}

BENCHMARK_TEMPLATE(BM_parallel_unnormalized_variation,std::uint32_t)
->Args({256    })
->Args({std::numeric_limits<std::uint16_t>::max()})
->Args({48 << 20})

;
BENCHMARK_MAIN();
