#define __BENCHMARK_HISTOGRAM_HEAP_VS_STACK_CPP__

#include <thread>

#include "benchmark_fixtures.hpp"

namespace sqeazy {

    namespace detail {

      template <typename T>
        struct dense_histo
        {

            static_assert(std::is_integral<T>::value,"[dense_histo] input type is not integral");
            static const std::size_t n_possible_values = (1 << (sizeof(T)*CHAR_BIT));
            typedef std::array<std::uint32_t,n_possible_values> type;


        };

        template <typename iter_type>
        auto serial_fill_histogram(iter_type _begin,
                                   iter_type _end
            ) -> typename dense_histo<typename std::iterator_traits<iter_type>::value_type>::type
        {

            using histo_t = typename dense_histo<typename std::iterator_traits<iter_type>::value_type>::type;

            histo_t histo;
            histo.fill(0u);

            for(;_begin!=_end;++_begin){
                histo[*_begin]++;
            }

            return histo;
        }

      template <typename iter_type, typename out_iter_type>
        out_iter_type serial_fill_histogram_byref(iter_type _begin,
                                                  iter_type _end,
                                                  out_iter_type _out

            )
        {

            using value_t = typename std::iterator_traits<iter_type>::value_type;
            static const std::size_t n_possible_values = (1 << (sizeof(value_t)*CHAR_BIT));

            std::fill(_out, _out + n_possible_values,0);

            for(;_begin!=_end;++_begin){
              *(_out + *_begin) += 1;
            }

            return _out + n_possible_values;
        }

    }
}

typedef sqeazy::benchmark::dynamic_synthetic_data<> uint16_fixture;
typedef sqeazy::benchmark::dynamic_synthetic_data<std::uint8_t> uint8_fixture;

BENCHMARK_DEFINE_F(uint8_fixture, stack_fill_histogram_uint8)(benchmark::State& state) {

  if (state.thread_index == 0) {
    SetUp(state);
  }

  typedef typename sqeazy::detail::dense_histo<std::uint8_t>::type container_t;

  container_t dh = sqeazy::detail::serial_fill_histogram(sinus_.cbegin(), sinus_.cend());


  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(dh.begin(),dh.end(),0);
    state.ResumeTiming();

    dh = sqeazy::detail::serial_fill_histogram(sinus_.cbegin(), sinus_.cend());
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(size_)*sizeof(sinus_.front()));
}

BENCHMARK_DEFINE_F(uint16_fixture, stack_fill_histogram_uint16)(benchmark::State& state) {

  if (state.thread_index == 0) {
    SetUp(state);
  }

  auto dh = sqeazy::detail::serial_fill_histogram(sinus_.cbegin(), sinus_.cend());


  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(dh.begin(),dh.end(),0);
    state.ResumeTiming();

    dh = sqeazy::detail::serial_fill_histogram(sinus_.cbegin(), sinus_.cend());
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(size_)*sizeof(sinus_.front()));
}



BENCHMARK_REGISTER_F(uint8_fixture, stack_fill_histogram_uint8)

->Args({4 << 10})//L1 cache
->Args({(48) << 10})//L2 cache
->Args({(480) << 10})//L3 cache
->Args({48 << 20})//RAM cache
;

BENCHMARK_REGISTER_F(uint16_fixture, stack_fill_histogram_uint16)

->Args({2 << 10})//L1 cache
->Args({(24) << 10})//L2 cache
->Args({(240) << 10})//L3 cache
->Args({24 << 20})//RAM cache
;

//->Range(1 << 16,1 << 25);
BENCHMARK_DEFINE_F(uint8_fixture, heap_fill_histogram_uint8)(benchmark::State& state) {

  if (state.thread_index == 0) {
    SetUp(state);
  }

  typedef std::vector<std::uint32_t> container_t;
  static const std::size_t hlen = (1 << (sizeof(std::uint8_t)*CHAR_BIT));

  container_t dh(hlen,0);
  sqeazy::detail::serial_fill_histogram_byref(sinus_.cbegin(), sinus_.cend(),dh.begin());


  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(dh.begin(),dh.end(),0);
    state.ResumeTiming();

    sqeazy::detail::serial_fill_histogram_byref(sinus_.cbegin(), sinus_.cend(),dh.begin());
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(size_)*sizeof(sinus_.front()));
}


BENCHMARK_DEFINE_F(uint16_fixture, heap_fill_histogram_uint16)(benchmark::State& state) {

  if (state.thread_index == 0) {
    SetUp(state);
  }

  typedef std::vector<std::uint32_t> container_t;
  static const std::size_t hlen = (1 << (sizeof(std::uint16_t)*CHAR_BIT));

  container_t dh(hlen,0);
  sqeazy::detail::serial_fill_histogram_byref(sinus_.cbegin(), sinus_.cend(),dh.begin());


  while (state.KeepRunning()) {
    state.PauseTiming();
    std::fill(dh.begin(),dh.end(),0);
    state.ResumeTiming();

    sqeazy::detail::serial_fill_histogram_byref(sinus_.cbegin(), sinus_.cend(),dh.begin());
  }

  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(size_)*sizeof(sinus_.front()));
}

BENCHMARK_REGISTER_F(uint8_fixture, heap_fill_histogram_uint8)

->Args({4 << 10})//L1 cache
->Args({(48) << 10})//L2 cache
->Args({(480) << 10})//L3 cache
->Args({48 << 20})//RAM cache
;

BENCHMARK_REGISTER_F(uint16_fixture, heap_fill_histogram_uint16)

->Args({2 << 10})//L1 cache
->Args({(24) << 10})//L2 cache
->Args({(240) << 10})//L3 cache
->Args({24 << 20})//RAM cache
;



BENCHMARK_MAIN();
