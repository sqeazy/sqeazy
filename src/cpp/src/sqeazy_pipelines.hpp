#ifndef _SQEAZY_PIPELINES_H_
#define _SQEAZY_PIPELINES_H_

//import native filters/sinks
#include "encoders/sqeazy_impl.hpp"
#include "encoders/quantiser_scheme_impl.hpp"

//import external filters/sinks
#include "encoders/lz4.hpp"
#include "encoders/h264.hpp"
#include "encoders/hevc.hpp"

#include "dynamic_pipeline.hpp"
#include "dynamic_stage_factory.hpp"


namespace sqeazy {

  
  template <typename T>
  using filters_factory = stage_factory<
    diff_scheme<T>,
    bitswap_scheme<T>,
    remove_background_scheme<T>,
    flatten_to_neighborhood_scheme<T>,
    remove_estimated_background_scheme<T>
    >;

  template <typename T>
  using encoders_factory = stage_factory<
    pass_through<T>,
    quantiser_scheme<T>,
    lz4_scheme<T>
    >;

  template <typename T>
  using tail_filters_factory = stage_factory<
    diff_scheme<T>,
    bitswap_scheme<T>,
    // remove_background_scheme<T>,
    // flatten_to_neighborhood_scheme<T>,
    // remove_estimated_background_scheme<T>,
    h264_scheme<T>,
    hevc_scheme<T>
    >;
  
  template <typename T>
  using dypeline = dynamic_pipeline<T, filters_factory, encoders_factory<T>, tail_filters_factory<char> >;

  //meant for lz4 only pipeline
  using dypeline_from_char = dynamic_pipeline<char,
					    filters_factory,
					    stage_factory<
					      lz4_scheme<char>,
					      hevc_scheme<char>,
					      h264_scheme<char>
					      >
					    >;
  
  //FIXME: required as quantiser will emit compilation error if incoming_type == outcoming_type
  using dypeline_from_uint8 = dynamic_pipeline<std::uint8_t,
					      filters_factory,
					      stage_factory<
						lz4_scheme<std::uint8_t>,
						hevc_scheme<std::uint8_t>,
						h264_scheme<std::uint8_t>
						>
					      >;

}

#endif /* _SQEAZY_PIPELINES_H_ */