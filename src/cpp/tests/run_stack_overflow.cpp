

#include <algorithm>
#include <climits>
#include <numeric>
#include <iostream>
#include <sstream>
#include <iterator>
#include <map>
#include <random>
#include <vector>
#include <array>

//#include "encoders/quantiser_scheme_impl.hpp"
namespace sqeazy
{
  namespace detail
  {

    template <typename T> struct dense_histo
    {

      static_assert(std::is_integral<T>::value, "[dense_histo] input type is not integral");
      static const std::size_t n_possible_values = (1 << (sizeof(T) * CHAR_BIT));
      typedef std::array<std::uint32_t, n_possible_values> type;
    };

    template <typename iter_type> auto serial_fill_histogram(iter_type _begin, iter_type _end) -> typename dense_histo<typename std::iterator_traits<iter_type>::value_type>::type
    {
      using value_t = typename std::iterator_traits<iter_type>::value_type;
      using histo_t = typename dense_histo<value_t>::type;

      histo_t histo;
      histo.fill(std::uint32_t(0));

      for(; _begin != _end; ++_begin)
      {
        histo[*_begin]++;
      }

      return histo;
    }

    template <typename iter_type> auto parallel_fill_histogram(iter_type _begin, iter_type _end, int nthreads = 1) -> typename dense_histo<typename std::iterator_traits<iter_type>::value_type>::type
    {
      using histo_t = typename dense_histo<typename std::iterator_traits<iter_type>::value_type>::type;

      histo_t histo;
      histo.fill(0u);

      std::vector<histo_t> histo_clones(nthreads, histo);
      auto histo_clones_itr = histo_clones.data();
      const auto len = std::distance(_begin, _end);
      const omp_size_type chunk_size = (len + nthreads - 1) / nthreads;


      // mapping the partitions of [_begin, _end) to histo_clones
#pragma omp parallel shared(histo_clones_itr) firstprivate(_begin, _end, chunk_size) num_threads(nthreads)
      {
        auto tid = omp_get_thread_num();
        auto chunk_begin = _begin + tid * chunk_size;
        auto chunk_end = chunk_begin + chunk_size >= _end ? _end : chunk_begin + chunk_size;
        auto my_histo_itr = &(*(histo_clones_itr + tid))[0];

        for(; chunk_begin != chunk_end; ++chunk_begin)
          my_histo_itr[*chunk_begin]++;
      }

      auto histo_itr = histo.data();
      const omp_size_type histo_len = histo.size();
      const omp_size_type n_clones = histo_clones.size();

#pragma omp parallel for shared(histo_itr) firstprivate(histo_clones_itr, n_clones) num_threads(nthreads)
      for(omp_size_type idx = 0; idx < histo_len; idx++)
      {
        for(omp_size_type clone = 0; clone < n_clones; ++clone)
        {
          *(histo_itr + idx) += (histo_clones_itr + clone)->data()[idx];
        }
      }

      return histo;
    }

    };

  namespace weighters
  {
    struct none
    {

      std::string name()
      {
        std::ostringstream msg;
        msg << "none_weight";
        return msg.str();
      }

      template <typename weights_iter_type, typename ref_iter_type> void transform(weights_iter_type _out, weights_iter_type _out_end, ref_iter_type _ref, int _nthreads = 1) const
      {
        this->transform(_out, _out_end, _nthreads);
        return;
      }

      template <typename weights_iter_type> void transform(weights_iter_type _out, weights_iter_type _out_end, int _nthreads = 1) const { return; }
    };
  };



template <typename raw_type, typename compressed_type> 
struct quantiser
{
  static_assert(std::is_integral<raw_type>::value, "[quantiser] raw_type is not integral");
  static_assert(std::is_integral<compressed_type>::value, "[quantiser] compressed_type is not integral");
  static_assert(sizeof(size_t) > sizeof(raw_type), "[quantiser] raw_type is not smaller than size_t");

  static const size_t max_raw_ = 1 << (sizeof(raw_type) * CHAR_BIT);
  static const size_t max_compressed_ = 1 << (sizeof(compressed_type) * CHAR_BIT);

  static const raw_type max_value_raw_ = (std::numeric_limits<raw_type>::max)();
  static const compressed_type max_value_compressed_ = (std::numeric_limits<compressed_type>::max)();

  static const char lut_field_separator = ':';

  long sum_;
  std::array<std::uint32_t, max_raw_> histo_;

  typedef compressed_type compressed_t;
  typedef raw_type raw_t;

  int nthreads_;
  std::array<float, max_raw_> weights_;

  template <typename weight_functor_t = weighters::none>
  quantiser(const raw_type *_begin = nullptr, const raw_type *_end = nullptr, int _nt = 1, weight_functor_t _weight_functor = weighters::none())
      : sum_(0)
      , histo_()
      , weights_()
      , nthreads_(_nt)
  {
    std::cout << "constructor: " << histo_.size() << ", " << weights_.size() << ", sum " << sum_ << "\n";
    reset();

    //setup_com(_begin, _end, _weight_functor);
  };


  void reset()
  {


    std::cout << "reset [0]: " << (histo_.data() + histo_.size()) << ", " << (weights_.data() + weights_.size()) << ","
              << "\n";
    std::cout << "reset [1]: " << histo_.size() << ", " << weights_.size() << ", sum " << sum_ << "\n";
    std::cout << "reset [2]: " << histo_[0] << ", " << weights_[0] << "\t" << *(histo_.data() + histo_.size() - 1) << ", " << *(weights_.data() + weights_.size() - 1) << "\n";

    histo_.fill(0.f);
    sum_ = 0;
    weights_.fill(1.f);

    std::cout << "reset [3]: " << histo_[0] << ", " << weights_[0] << "\t" << *(histo_.data() + histo_.size() - 1) << ", " << *(weights_.data() + weights_.size() - 1) << "\n";
    

  }

  template <typename weight_functor_t = weighters::none> 
  void setup_com(const raw_type *_begin, const raw_type *_end, weight_functor_t _weight_functor = weighters::none())
  {

    // safe-guard
    if(!_begin || !(std::distance(_begin,_end)!=0))
    {
      std::cout << "[setup_com] recevied invalid range!\n";
      return;
    }


    std::cout << "computeHistogram::serial_fill to temp histo\n";
    auto th = detail::serial_fill_histogram(_begin, _end);
    std::cout << "computeHistogram copy " << th.size() << " to histo_ " << this->histo_.size() << " \n";

    std::copy(th.cbegin(), th.cend(), histo_.begin());
    this->sum_ = std::accumulate(histo_.begin(), histo_.end(), 0);

  }


  void set_n_threads(int nt) { nthreads_ = nt; }

  const int n_threads() const { return nthreads_; }

};
};


int main(int argc,char **argv)
{
  std::vector<std::uint16_t> embryo_(1 << 10, 0);
  sqeazy::quantiser<uint16_t, uint8_t> shrinker;
  try
  {
    shrinker.setup_com(0, 0);
  }
  catch(std::exception &e)
  {
    std::cerr << "shrinker.setup_com(0, 0) threw" << e.what() << "\n";
  }
  
  try
  {
    shrinker.setup_com(embryo_.data(), embryo_.data() + embryo_.size());
  }
  catch(std::exception &e)
  {
    std::cerr << "shrinker.setup_com(embryo_.data(), embryo_.data() + embryo_.size()) threw" << e.what() << "\n";
  }
}

