#ifndef HEADER_TAG_H
#define HEADER_TAG_H

#include <vector>
#include <string>
#include <cstdint>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>


namespace sqeazy {

    namespace header {

        struct tag {

            std::string pipename_;
            std::string raw_type_id_;

            std::vector<std::size_t> raw_shape_;
            std::intmax_t encoded_bytes_;

            friend class boost::serialization::access;

            template<class Archive>
            void serialize(Archive & ar, const unsigned int version)
                {
                    ar & pipename_;
                    ar & raw_type_id_;
                    ar & raw_shape_;
                    ar & encoded_bytes_;
                }

            tag():
                pipename_(""),
                raw_type_id_(""),
                raw_shape_(0),
                encoded_bytes_(0){
            }

            tag(const std::string& _pipename,
                 const std::string& _raw_type_id,
                 const std::vector<std::size_t>& _raw_shape,
                 const std::intmax_t& _encoded_bytes
                ):
                pipename_(_pipename),
                raw_type_id_(_raw_type_id),
                raw_shape_(_raw_shape),
                encoded_bytes_(_encoded_bytes){
            }

            bool operator==(const tag& _rhs){

                bool value = true;
                value = value && pipename_ == _rhs.pipename_;
                value = value && raw_type_id_ == _rhs.raw_type_id_;
                value = value && std::equal(raw_shape_.begin(), raw_shape_.end(),
                                            _rhs.raw_shape_.begin());
                value = value && encoded_bytes_ == _rhs.encoded_bytes_;

                return value;
            }





        };


        static std::string to_string(tag& _tag){

            std::stringstream ofs("");
            boost::archive::text_oarchive oa(ofs);
            oa << _tag;
            return ofs.str();
        }

        static tag from_string(const std::string& _data){

            std::stringstream ifs(_data);
            tag value;

            boost::archive::text_iarchive ia(ifs);
            ia >> value;

            return value;
        }
    }


}

#endif /* HEADER_TAG_H */
