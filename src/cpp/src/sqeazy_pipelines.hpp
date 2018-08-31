#ifndef _SQEAZY_PIPELINES_H_
#define _SQEAZY_PIPELINES_H_

//import native filters/sinks
#include "encoders/sqeazy_impl.hpp"
#include "encoders/quantiser_scheme_impl.hpp"
#include "encoders/raster_reorder_scheme_impl.hpp"
#include "encoders/zcurve_reorder_scheme_impl.hpp"
#include "encoders/tile_shuffle_scheme_impl.hpp"
#include "encoders/frame_shuffle_scheme_impl.hpp"

//import external filters/sinks
#include "encoders/lz4.hpp"

#ifdef SQY_WITH_FFMPEG
#include "encoders/h264.hpp"
#include "encoders/hevc.hpp"
#endif

#ifdef SQY_WITH_BITSHUFFLE
#include "encoders/bitshuffle_scheme_impl.hpp"
#endif

#include "dynamic_pipeline.hpp"
#include "dynamic_stage_factory.hpp"


namespace sqeazy {


  template <typename T>
  using filters_factory = stage_factory<
    diff_scheme<T>,
    bitswap_scheme<T>,
#ifdef SQY_WITH_BITSHUFFLE
    bitshuffle_scheme<T>,
#endif
      remove_background_scheme<T>,
    flatten_to_neighborhood_scheme<T>,
    remove_estimated_background_scheme<T>,
    raster_reorder_scheme<T>,
    tile_shuffle_scheme<T>,
    frame_shuffle_scheme<T>,
    zcurve_reorder_scheme<T>
    >;

  template <typename T>
  using encoders_factory = stage_factory<
    pass_through<T>,
    quantiser_scheme<T>,
    #ifdef SQY_WITH_FFMPEG
    hevc_scheme<T>,
    h264_scheme<T>,
    #endif
    lz4_scheme<T>
    >;

  template <typename T>
  using tail_filters_factory = stage_factory<
    diff_scheme<T>,
    bitswap_scheme<T>,
#ifdef SQY_WITH_BITSHUFFLE
    bitshuffle_scheme<T>,
#endif
#ifdef SQY_WITH_FFMPEG
    h264_scheme<T>,
    hevc_scheme<T>,
#endif
    lz4_scheme<T>,
    raster_reorder_scheme<T>,
    tile_shuffle_scheme<T>,
    frame_shuffle_scheme<T>,
    zcurve_reorder_scheme<T>
    >;

  template <typename T>
  using dypeline = dynamic_pipeline<T, filters_factory, encoders_factory<T>, tail_filters_factory<char> >;


  //FIXME: required as quantiser will emit compilation error if incoming_type == outcoming_type
  #ifdef SQY_WITH_FFMPEG
  using dypeline_from_uint8 = dynamic_pipeline<std::uint8_t,
                                               filters_factory,
                                               stage_factory<
                                                 lz4_scheme<std::uint8_t>,
                                                 hevc_scheme<std::uint8_t>,
                                                 h264_scheme<std::uint8_t>
                                                 >,
                                               stage_factory<
                                                 lz4_scheme<char>,
                                                 hevc_scheme<char>,
                                                 h264_scheme<char>
                                                 >
                                               >;
#else
  using dypeline_from_uint8 = dynamic_pipeline<std::uint8_t,
                                               filters_factory,
                                               stage_factory<
                                                 lz4_scheme<std::uint8_t>
                                                 >,
                                               stage_factory<
                                                 lz4_scheme<char>
                                                 >
                                               >;
  #endif
}

#endif /* _SQEAZY_PIPELINES_H_ */
