#ifndef QUANTISER_WEIGHTERS_H
#define QUANTISER_WEIGHTERS_H

#include "sqeazy_common.hpp"

#include <string>
#include <limits>
#include <cstdint>

#if defined(__clang__) || defined (__GNUC__)
# define ATTRIBUTE_NO_SANITIZE_ADDRESS __attribute__((no_sanitize_address))
#else
# define ATTRIBUTE_NO_SANITIZE_ADDRESS
#endif

namespace sqeazy {

    namespace weighters {

        struct offset_power_of{

            std::size_t first_nonzero_index = (std::numeric_limits<std::size_t>::max)();

            int exp_enum;
            int exp_denom;
            const float exponent;

            offset_power_of(int _enum = 1, int _denom = 1):
                first_nonzero_index((std::numeric_limits<std::size_t>::max)()),
                exp_enum(_enum),
                exp_denom(_denom),
                exponent(float(exp_enum)/exp_denom)
                {}

            std::string name () const {
                std::ostringstream msg;
                msg << "offset_power_of_" << exp_enum << "_" << exp_denom;
                return msg.str();
            }


            template <typename weights_iter_type>
            ATTRIBUTE_NO_SANITIZE_ADDRESS void transform(weights_iter_type _out,
                           weights_iter_type _out_end ,
                           omp_size_type _offset = 0,
                           int _nthreads = 1
                ) const {
                //REMOVED from address sanitizer as gcc 7.3's asan triggers on these functions while llvm/clang 5.0.0 doesn't

                const omp_size_type len = std::distance(_out, _out_end);


#pragma omp parallel for                        \
    shared(_out )                               \
    firstprivate( _offset )                     \
    num_threads(_nthreads)
                for(omp_size_type i = _offset;i<len;++i){
                    *(_out + i) = std::pow(i-_offset,exponent);
                }

                return ;
            }

            template <typename weights_iter_type, typename ref_iter_type>
            void transform(weights_iter_type _out,
                           weights_iter_type _out_end ,
                           ref_iter_type _ref,
                           int _nthreads = 1
                ) const {

                const omp_size_type len = std::distance(_out, _out_end);
                omp_size_type offset = 0;

                for(;offset<len;++offset){
                    if(*(_ref + offset))
                        break;
                }

                this->transform(_out,_out_end,offset,_nthreads);

                return ;
            }

            template <typename weights_iter_type>
            void transform(weights_iter_type _out,
                           weights_iter_type _out_end ,
                           int _nthreads = 1
                ) const {

                this->transform(_out,_out_end,0,_nthreads);

                return ;
            }

        };


        struct power_of
        {

            int exp_enum = 1;
            int exp_denom = 1;
            const float exponent;

            power_of(int _enum = 1, int _denom = 1):
                exp_enum(_enum),
                exp_denom(_denom),
                exponent(float(exp_enum)/exp_denom){}

            std::string name () const {
                std::ostringstream msg;
                msg << "power_of_" << exp_enum << "_" << exp_denom;
                return msg.str();
            }


            /* perform output = output.index(current) ** exponent */
            template <typename weights_iter_type>
            ATTRIBUTE_NO_SANITIZE_ADDRESS void transform(weights_iter_type _out,
                           weights_iter_type _out_end ,
                           int _nthreads = 1
                ) const {
                //REMOVED from address sanitizer as gcc 7.3's asan triggers on these functions while llvm/clang 5.0.0 doesn't

                const omp_size_type len = std::distance(_out, _out_end);

#pragma omp parallel for                        \
    shared(_out )                               \
    num_threads(_nthreads)
                for(omp_size_type i = 0;i<len;++i){
                    *(_out + i) = std::pow(i,exponent);
                }

                return ;
            }

            /* perform output = ref ** exponent ?? */
            template <typename weights_iter_type,
                      typename ref_iter_type>
            void transform(weights_iter_type _out,
                           weights_iter_type _out_end ,
                           ref_iter_type _ref,
                           int _nthreads = 1
                ) const {

                this->transform(_out, _out_end, _nthreads);

                /*
                const omp_size_type len = std::distance(_out, _out_end);

#pragma omp parallel for                        \
     shared(_out )                               \
     num_threads(_nthreads)
                for(omp_size_type i = 0;i<len;++i){
                    *(_out + i) = std::pow(*(_ref + i),exponent);
                }
                */
                return ;
            }
        };



        struct set_to_one
        {

            std::string name (){
                std::ostringstream msg;
                msg << "set_to_one_weight";
                return msg.str();
            }

            template <typename weights_iter_type,
                      typename ref_iter_type>
            void transform(weights_iter_type _out,
                           weights_iter_type _out_end ,
                           ref_iter_type _ref,
                           int _nthreads = 1
                ) const {

                this->transform(_out,_out_end,_nthreads);

                return ;
            }

            template <typename weights_iter_type>
            void transform(weights_iter_type _out,
                           weights_iter_type _out_end ,
                           int _nthreads = 1
                ) const {

                const omp_size_type len = std::distance(_out, _out_end);

#pragma omp parallel for                        \
    shared(_out )                               \
    num_threads(_nthreads)
                for(omp_size_type i = 0;i<len;++i){
                    *(_out + i) = 1;
                }

                return ;
            }
        };

        struct none
        {

            std::string name (){
                std::ostringstream msg;
                msg << "none_weight";
                return msg.str();
            }

            template <typename weights_iter_type,
                      typename ref_iter_type>
            void transform(weights_iter_type _out,
                           weights_iter_type _out_end ,
                           ref_iter_type _ref,
                           int _nthreads = 1
                ) const {
                this->transform(_out,_out_end,_nthreads);
                return ;
            }

            template <typename weights_iter_type>
            void transform(weights_iter_type _out,
                           weights_iter_type _out_end ,
                           int _nthreads = 1
                ) const
                {

                    return ;
                }


        };



    };//weighters

};//sqeazy

#endif /* QUANTISER_WEIGHTERS_H */
