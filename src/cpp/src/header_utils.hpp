#ifndef HEADER_UTILS_H
#define HEADER_UTILS_H

#include <string>
#include <cstdint>

namespace sqeazy
{
    namespace header
    {

        template <typename T>
        struct represent{
            static const std::string as_string() {return "";};
        };

        //template <> struct represent<unsigned char>{ static const std::string as_string(){ return "uint8"; } };
        template <> struct represent<std::uint8_t>{ static const std::string as_string(){ return "uint8"; } };
        template <> struct represent<std::int8_t>{ static const std::string as_string(){ return "int8"; } };
        template <> struct represent<char>{ static const std::string as_string(){ return "int8"; } };

        template <> struct represent<std::uint16_t>{ static const std::string as_string(){ return "uint16"; } };
        template <> struct represent<std::int16_t>{ static const std::string as_string(){ return "int16"; } };
//    template <> struct represent<unsigned short>{ static const std::string as_string(){ return "uint16"; } };
        //  template <> struct represent<short>{ static const std::string as_string(){ return "int16"; } };

        template <> struct represent<std::uint32_t>{ static const std::string as_string(){ return "uint32"; } };
        template <> struct represent<std::int32_t>{ static const std::string as_string(){ return "int32"; } };
        // template <> struct represent<unsigned int>{ static const std::string as_string(){ return "uint32"; } };
        // template <> struct represent<int>{ static const std::string as_string(){ return "int32"; } };

        template <> struct represent<std::uint64_t>{ static const std::string as_string(){ return "uint64"; } };
        template <> struct represent<std::int64_t>{ static const std::string as_string(){ return "int64"; } };
        //template <> struct represent<unsigned long>{ static const std::string as_string(){ return "uint64"; } };
        //template <> struct represent<long>{ static const std::string as_string(){ return "int64"; } };



        static std::uint32_t sizeof_typeinfo(const std::string& _lit) {

            static std::map<std::string,std::uint32_t> type_size_map;

            type_size_map[sqeazy::header::represent<std::int16_t>::as_string()] = sizeof(std::int16_t);
            type_size_map[sqeazy::header::represent<std::uint16_t>::as_string()] = sizeof(std::uint16_t);

            type_size_map[sqeazy::header::represent<std::int8_t>::as_string()] = sizeof(std::int8_t);
            type_size_map[sqeazy::header::represent<std::uint8_t>::as_string()] = sizeof(std::uint8_t);

            type_size_map[sqeazy::header::represent<std::int32_t>::as_string()] = sizeof(std::int32_t);
            type_size_map[sqeazy::header::represent<std::uint32_t>::as_string()] = sizeof(std::uint32_t);

            type_size_map[sqeazy::header::represent<std::int64_t>::as_string()] = sizeof(std::int64_t);
            type_size_map[sqeazy::header::represent<std::uint64_t>::as_string()] = sizeof(std::uint64_t);

            auto found = type_size_map.find(_lit);
            if(found!=type_size_map.end())
                return found->second;
            else
                return 0;
        }


    };

};


#endif /* HEADER_UTILS_H */
