#ifndef _SQEAZY_IMPL_H_
#define _SQEAZY_IMPL_H_
#include <functional>
#include <algorithm>
#include <sstream>
#include <climits>
#include <numeric>
#include "sqeazy_common.hpp"
#include "traits.hpp"
#include "dynamic_stage.hpp"

#include "hist_impl.hpp"
#include "huffman_utils.hpp"

//filter schemes
#include "pass_through_scheme_impl.hpp"
#include "diff_scheme_impl.hpp"
#include "bitswap_scheme_impl.hpp"
#include "remove_background_scheme_impl.hpp"
#include "flatten_to_neighborhood_scheme_impl.hpp"
#include "remove_estimated_background_scheme_impl.hpp"


#endif /* _SQEAZY_IMPL_H_ */
