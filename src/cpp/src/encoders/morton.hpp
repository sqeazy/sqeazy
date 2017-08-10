#ifndef _MORTON_H_
#define _MORTON_H_


#include <array>
#include <climits>

#include "traits.hpp"

namespace sqeazy {
	namespace detail {

		template <int stripe_size,
				  int bitplane_width,
				  std::uint32_t input,
				  int stripe_count
				  >
		struct morton_impl {

			static const std::uint32_t n_bits = sizeof(input)*CHAR_BIT;
			static const std::uint32_t value_mask = (~std::uint32_t(0)) >> (n_bits - bitplane_width);
			static const std::uint32_t n_bits_per_stripe = stripe_size*bitplane_width;
			static const std::uint32_t n_stripes = n_bits / n_bits_per_stripe;

			static const std::uint32_t value_of_interest = (input & value_mask);

			static const std::uint32_t current = value_of_interest << (stripe_size*bitplane_width*(n_stripes - stripe_count));


			static const std::uint32_t value = current | morton_impl<stripe_size,
																	 bitplane_width,
																	 (input >> bitplane_width),
																	  stripe_count - 1
																	  >::value;
		};

		template <int stripe_size,
				  int bitplane_width,
				  std::uint32_t input
				  >
		struct morton_impl<stripe_size, bitplane_width, input, 0> {

			static const std::uint32_t n_bits = sizeof(input)*CHAR_BIT;
			static const std::uint32_t value_mask = (~std::uint32_t(0)) >> (n_bits - bitplane_width);
			static const std::uint32_t n_bits_per_stripe = stripe_size*bitplane_width;
			static const std::uint32_t n_stripes = n_bits / n_bits_per_stripe;

			static const std::uint32_t value_of_interest = (input & value_mask);

			static const std::uint32_t current = value_of_interest << (stripe_size*bitplane_width*n_stripes);

			static const std::uint32_t value = current;

		};

		template <std::uint32_t input,
				  int stripe_size = 3,
				  int bitplane_width = 1>
		struct compile_time_morton {

			static const std::uint32_t index_n_bits = sizeof(input)*CHAR_BIT;
			static const std::uint32_t n_bits_per_stripe = stripe_size*bitplane_width;
			static const std::uint32_t n_stripes = index_n_bits / n_bits_per_stripe;

			static const std::uint32_t value = morton_impl<stripe_size,
														   bitplane_width,
														   input,
														   n_stripes
														   >::value;

		};


		static const std::array<std::uint32_t, 16> manual_morton_3x1 = {
			compile_time_morton<0>::value,
			compile_time_morton<1>::value,
			compile_time_morton<2>::value,
			compile_time_morton<3>::value,
			compile_time_morton<4 + 0>::value,
			compile_time_morton<4 + 1>::value,
			compile_time_morton<4 + 2>::value,
			compile_time_morton<4 + 3>::value,
			compile_time_morton<8 + 0>::value,
			compile_time_morton<8 + 1>::value,
			compile_time_morton<8 + 2>::value,
			compile_time_morton<8 + 3>::value,
			compile_time_morton<8 + 4 + 0>::value,
			compile_time_morton<8 + 4 + 1>::value,
			compile_time_morton<8 + 4 + 2>::value,
			compile_time_morton<8 + 4 + 3>::value
		};


		template <int stripe_size = 3, int bitplane_width = 1, int N, std::uint32_t ...Vals>
		static constexpr
		typename std::enable_if<N == sizeof...(Vals), std::array<std::uint32_t, N>>::type
		fill_array() {
			return std::array<std::uint32_t, N>{ {Vals...}};
		}

		template <int stripe_size = 3, int bitplane_width = 1, int N, std::uint32_t ...Vals>
		static constexpr
		typename std::enable_if<N != sizeof...(Vals), std::array<std::uint32_t, N>>::type
		fill_array() {
			return fill_array<stripe_size,
							  bitplane_width,
							  N,
							  Vals...,
							  compile_time_morton<(sizeof...(Vals)), stripe_size, bitplane_width>::value
							  >();
		}




		template <int bitplane_width = 1, int stripe_size = 3>
		struct morton_at_ct {

			static const std::uint32_t size = 256;
			static const std::uint32_t span_bits = 8;
			static const std::uint32_t bits_per_dim = sizeof(std::uint64_t)*CHAR_BIT / stripe_size;
			static const std::uint32_t n_full_spans = bits_per_dim / span_bits;
			static const std::uint32_t span_mask = (1 << span_bits) - 1;

			typedef std::array<std::uint32_t, size> array_t;

#ifdef WIN32
			static const array_t values;
#else
			static constexpr array_t values  = fill_array<stripe_size, bitplane_width, size>();
#endif

			static inline std::uint64_t from(std::uint32_t z,
											 std::uint32_t y,
											 std::uint32_t x) {
				std::uint64_t value = 0;
				// static const std::uint32_t n_remaining_bits	= bits_per_dim % span_bits	;
				// static const std::uint32_t mask_remaining_bits	= (1 << n_remaining_bits) -1 ;

				for (std::int32_t span_shift = (n_full_spans);
					 span_shift >= 0;
					 --span_shift) {

					// auto current_mask = span_shift == n_full_spans ? mask_remaining_bits : span_mask;
					value |= (values[(z >> (span_shift*span_bits)) & span_mask] << (2 * bitplane_width) | // shifting second byte
							  values[(y >> (span_shift*span_bits)) & span_mask] << (bitplane_width) |
							  values[(x >> (span_shift*span_bits)) & span_mask]) << (span_shift * 3 * span_bits);
				}

				return value;
			}

			static inline std::uint64_t from_(const std::array<std::uint32_t, stripe_size>& _coords) {
				return from_(_coords[row_major::z],
							 _coords[row_major::y],
							 _coords[row_major::x]);
			}


			static inline void to_(std::uint64_t index, std::array<std::uint32_t, stripe_size>& _coords) {

				static const std::uint32_t bits_per_dim = sizeof(index)*CHAR_BIT / 3;
				// static const std::uint32_t n_full_spans		= bits_per_dim/span_bits  	;
				// static const std::uint32_t n_remaining_bits	= bits_per_dim % span_bits	;
				// static const std::uint32_t mask_remaining_bits	= (1 << n_remaining_bits) -1 ;
				static const std::uint32_t mask_bits_of_interest = (1 << bitplane_width) - 1;

				_coords.fill(0);

				std::uint64_t index_of_interest = index;
				for (std::uint32_t b = 0; b < bits_per_dim && index_of_interest != 0; b += bitplane_width, index_of_interest >>= stripe_size*bitplane_width) {
					for (std::uint32_t d = 0; d < _coords.size(); ++d) {
						const auto in_stripe_offset = (_coords.size() - d - 1)*bitplane_width;
						auto temp = ((index_of_interest >> in_stripe_offset) & mask_bits_of_interest);
						_coords[d] |= temp << b;
					}
				}


				return;

			}

			static inline void to(std::uint64_t index, std::uint32_t& z, std::uint32_t& y, std::uint32_t& x) {
				std::array<std::uint32_t, stripe_size> coords;
				coords[row_major::z] = z;
				coords[row_major::y] = y;
				coords[row_major::x] = x;
				to_(index, coords);
				z = coords[row_major::z];
				y = coords[row_major::y];
				x = coords[row_major::x];
			}


		};

#ifdef WIN32
		template<int stripe_size, int bitplane_width>
		const std::array<std::uint32_t, 256> morton_at_ct< stripe_size, bitplane_width>::values = fill_array<stripe_size, bitplane_width, 256>();
#else
#if __clang__
		template<int stripe_size, int bitplane_width>
		constexpr typename morton_at_ct< stripe_size, bitplane_width>::array_t morton_at_ct< stripe_size, bitplane_width>::values;
#else
		template<int stripe_size, int bitplane_width>
		constexpr std::array<std::uint32_t, 256> morton_at_ct< stripe_size, bitplane_width>::values;
#endif

#endif
	};
};

#endif /* _MORTON_H_ */
