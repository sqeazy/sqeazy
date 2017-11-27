#ifndef _SQEAZY_ALGORITHMS_H_
#define _SQEAZY_ALGORITHMS_H_

#include "sqeazy_common.hpp"


#include <cmath>
#include <iterator>
#include <numeric>

namespace sqeazy {

  static inline int clean_number_of_threads(int nthreads_to_use){

    static int max_threads = std::thread::hardware_concurrency();

    if(nthreads_to_use > max_threads || nthreads_to_use <= 0)
      nthreads_to_use = max_threads;

    return nthreads_to_use;
  }

  template <typename element_t>
  int lowest_set_bit(element_t _data, element_t _data_end){

    int value = sizeof(element_t)*CHAR_BIT - 1;
    for(;_data!=_data_end;++_data){

      int lowest = 0;
      for(;lowest<value;++lowest)
      {
        if((*_data >> lowest) & 1)
          value = lowest;
      }

      if(lowest<value)
        value = lowest;

      if(value == 0)
        break;
    }

    return value;
  }

  template <typename element_t>
  int highest_set_bit(element_t _data, element_t _data_end){

    auto max = *std::max_element(_data, _data_end);
    float value = 0;

    if(max)
      value = std::log(max)/std::log(2);

    return std::ceil(value);
  }

  template <typename it_type, typename it2_type>
  static inline double l1norm(it_type _lbegin, it_type _lend,
                              it2_type _rbegin){

    double dsum = 0;

    for(;_lbegin!=_lend;++_lbegin,++_rbegin){
      double diff = *_lbegin - *_rbegin;
      dsum += std::abs(diff);
    }

    double value = std::sqrt(dsum);

    return value;
  }

  template <typename it_type, typename it2_type>
  static inline double l2norm(it_type _lbegin, it_type _lend,
                              it2_type _rbegin){

    double dsum = 0;

    for(;_lbegin!=_lend;++_lbegin,++_rbegin){
      double diff = *_lbegin - *_rbegin;
      dsum += diff*diff;
    }

    double value = std::sqrt(dsum);

    return value;
  }

  //MSE = 1/N sum[(I-I')^2]
  template <typename it_type, typename it2_type>
  static inline double mse(it_type _lbegin, it_type _lend,
                           it2_type _rbegin){

    typedef decltype(*_lbegin) item_t;
    typedef decltype(*_rbegin) item2_t;

    static_assert(sizeof(item_t)==sizeof(item2_t),"[sqeazy::mse] types of different width received");

    size_t left_size = _lend - _lbegin;

    std::vector<float> diff(_lend - _lbegin,0);
    std::transform(_lbegin,_lend,
                   _rbegin,
                   diff.begin(),
                   std::minus<float>());

    double mse = std::inner_product(diff.begin(), diff.end(),
                                    diff.begin(),
                                    0.);
    mse /= left_size;

    return mse;
  }

  template <typename it_type>
  static inline double mse(it_type _lbegin, it_type _lend,
                           double _scalar){


    const auto len = std::distance(_lbegin,_lend);

    std::vector<double> diff(len,_scalar);
    std::transform(_lbegin,_lend,
                   diff.begin(),
                   diff.begin(),
                   std::minus<double>());

    double mse = std::inner_product(diff.begin(), diff.end(),
                                    diff.begin(),
                                    0.);
    mse /= len;

    return mse;
  }

  //RMS = sqrt(1/N sum[(I-I')^2])
  template <typename it_type, typename it2_type>
  static inline double rms(it_type _lbegin, it_type _lend,
                           it2_type _rbegin){

    typedef decltype(*_lbegin) item_t;
    typedef decltype(*_rbegin) item2_t;

    static_assert(sizeof(item_t)==sizeof(item2_t),"[sqeazy::mse] types of different width received");

    double value = mse(_lbegin, _lend,_rbegin);

    return std::sqrt(value);
  }

  //NRMSE = sqrt{1/N sum (I-I')^2}/[max(I) - min(I)]
  template <typename it_type, typename it2_type>
  static inline double nrmse(it_type _lbegin, it_type _lend,
                             it2_type _rbegin){

    typedef decltype(*_lbegin) item_t;
    typedef decltype(*_rbegin) item2_t;

    static_assert(sizeof(item_t)==sizeof(item2_t),"[sqeazy::nrmse] types of different width received");

    size_t left_size = _lend - _lbegin;

    std::vector<float> diff(_lend - _lbegin,0);
    std::transform(_lbegin,_lend,
                   _rbegin,
                   diff.begin(),
                   std::minus<float>());

    double rnmse = std::sqrt(std::inner_product(diff.begin(), diff.end(),
                                                diff.begin(),
                                                0.));
    rnmse /= (left_size);

    item_t max_value = *std::max_element(_lbegin, _lend);
    item_t min_value = *std::min_element(_lbegin, _lend);

    rnmse = std::sqrt(rnmse)/std::abs(max_value - min_value);

    return rnmse;
  }




  template <typename it_type>
  static inline double mean(it_type _begin, it_type _end){



    const auto len = std::distance(_begin,_end);

    double value = std::accumulate(_begin, _end,std::size_t(0),std::plus<std::size_t>())/double(len);

    return value;
  }

  template <typename it_type>
  static inline double variance(it_type _begin, it_type _end){

    const auto len = std::distance(_begin,_end);

    double mn = mean(_begin, _end);
    double variance = mse(_begin, _end, mn);

    return variance;
  }

  template <typename it_type, typename it2_type>
  static inline double psnr(it_type _lbegin, it_type _lend,
                            it2_type _rbegin){


    typedef decltype(*_lbegin) item_t;
    typedef decltype(*_rbegin) item2_t;

    static_assert(sizeof(item_t)==sizeof(item2_t),"[sqeazy::psnr] types of different width received");

    double mse = sqeazy::mse(_lbegin, _lend, _rbegin);

    const size_t max = ~0;
    const double offset = 20*std::log((item_t)max);
    double psnr = offset - 10*std::log(mse);

    return psnr;
  }

  //MNMSE = sqrt{1/N sum (I-I')^2}/[mean(I)]
  template <typename it_type, typename it2_type>
  static inline double mnmse(it_type _lbegin, it_type _lend,
                             it2_type _rbegin){

    typedef decltype(*_lbegin) item_t;
    typedef decltype(*_rbegin) item2_t;

    static_assert(sizeof(item_t)==sizeof(item2_t),"[sqeazy::nrmse] types of different width received");

    size_t left_size = _lend - _lbegin;

    std::vector<float> diff(_lend - _lbegin,0);
    std::transform(_lbegin,_lend,
                   _rbegin,
                   diff.begin(),
                   std::minus<float>());

    double rnmse = std::sqrt(std::inner_product(diff.begin(), diff.end(),
                                                diff.begin(),
                                                0.));
    rnmse /= (left_size);

    const double mn = mean(_lbegin,_lend);
    rnmse = std::sqrt(rnmse)/mn;

    return rnmse;
  }

  template <typename iter_t>
  size_t dyn_range(iter_t _begin, iter_t _end){

    typedef typename std::iterator_traits<iter_t>::value_type value_t;
    static_assert(std::is_integral<value_t>::value,"[sqeazy::dyn_range] only works with integer types");


    auto max = *std::max_element(_begin, _end);
    auto min = *std::min_element(_begin, _end);

    size_t value = (max-min) +1;

    return value;
  }

  template <typename iter_t, typename out_iter_t>
  out_iter_t prefix_sum(iter_t _begin, iter_t _end,
                        out_iter_t _out,
                        int _nthreads = 1){

    typedef typename std::iterator_traits<iter_t>::pointer in_ptr_t;
    // typedef typename std::iterator_traits<iter_t>::value_type in_value_t;
    typedef typename std::iterator_traits<out_iter_t>::value_type out_value_t;
    typedef typename std::iterator_traits<out_iter_t>::pointer out_ptr_t;

    if(_nthreads <= 0)
      _nthreads = std::thread::hardware_concurrency();

    const omp_size_type len = std::distance(_begin,_end);

    if(len < _nthreads)
      _nthreads = 1;

    if(_nthreads == 1){
      out_value_t sum = 0;
      for(omp_size_type i = 0;i<len;++i){
        *(_out++) = sum;
        sum += *(_begin + i);
      }

      return _out;
    }

    in_ptr_t pdata = &(*_begin);
    out_ptr_t psum = &(*_out);

    out_value_t *suma = nullptr;
#pragma omp parallel
    {
      const int ithread = omp_get_thread_num();
      const int nthreads = omp_get_num_threads();
#pragma omp single
      {
        suma = new out_value_t[nthreads+1];
        suma[0] = 0;
      }

      out_value_t sum = 0;
#pragma omp for schedule(static)
      for (omp_size_type i=0; i<len; i++) {
        psum[i] = sum;
        sum += pdata[i];

      }
      suma[ithread+1] = sum;
#pragma omp barrier
      out_value_t offset = 0;
      for(omp_size_type i=0; i<(ithread+1); i++) {
        offset += suma[i];
      }
#pragma omp for schedule(static)
      for (omp_size_type i=0; i<len; i++) {
        psum[i] += offset;
      }
    }
    delete[] suma;

    return _out + len;
  }

  template <typename iter_t, typename out_iter_t, typename functor_t>
  out_iter_t prefix_sum_of(iter_t _begin, iter_t _end,
                           out_iter_t _out,
                           functor_t&& _my_functor,
                           int _nthreads = 1){

    typedef typename std::iterator_traits<iter_t>::pointer in_ptr_t;
    // typedef typename std::iterator_traits<iter_t>::value_type in_value_t;
    typedef typename std::iterator_traits<out_iter_t>::value_type out_value_t;
    typedef typename std::iterator_traits<out_iter_t>::pointer out_ptr_t;

    if(_nthreads <= 0)
      _nthreads = std::thread::hardware_concurrency();

    const omp_size_type len = std::distance(_begin,_end);

    if(len < _nthreads)
      _nthreads = 1;

    if(_nthreads == 1){
      out_value_t sum = 0;
      for(omp_size_type i = 0;i<len;++i){
        *(_out++) = sum;
        sum += _my_functor(*(_begin + i));
      }

      return _out;
    }

    in_ptr_t pdata = &(*_begin);
    out_ptr_t psum = &(*_out);

    out_value_t *suma = nullptr;
#pragma omp parallel
    {
      const int ithread = omp_get_thread_num();
      const int nthreads = omp_get_num_threads();
#pragma omp single
      {
        suma = new out_value_t[nthreads+1];
        suma[0] = 0;
      }

      out_value_t sum = 0;
#pragma omp for schedule(static)
      for (omp_size_type i=0; i<len; i++) {
        psum[i] = sum;
        sum += _my_functor(pdata[i]);

      }
      suma[ithread+1] = sum;
#pragma omp barrier
      out_value_t offset = 0;
      for(omp_size_type i=0; i<(ithread+1); i++) {
        offset += suma[i];
      }
#pragma omp for schedule(static)
      for (omp_size_type i=0; i<len; i++) {
        psum[i] += offset;
      }
    }
    delete[] suma;

    return _out + len;
  }


};

#endif /* _SQEAZY_ALGORITHMS_H_ */
