#define BOOST_TEST_MODULE TEST_STRING_PARSERS
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"
#include <numeric>
#include <functional>
#include <cmath>
#include <vector>
#include <map>
#include <chrono>
#include <cstdint>
#include <sstream>
#include "string_parsers.hpp"

namespace sqy = sqeazy;

static char global_serial_longs [] = "step1->step2(option=1)->step3(junk=<verbatim>\\\u000B\u0000\u0000\u0000\u0000\u0000\u0000]\u000B\u0000\u0000\u0000\u0000\u0000\u0000^\u000B\u0000\u0000\u0000\u0000\u0000\u0000_\u000B\u0000\u0000\u0000\u0000\u0000\u0000`</verbatim>,option2=false)->step4(option1=true,option2=false)";
static char global_junk_only [] = "junk=<verbatim>\\\u000B\u0000\u0000\u0000\u0000\u0000\u0000]\u000B\u0000\u0000\u0000\u0000\u0000\u0000^\u000B\u0000\u0000\u0000\u0000\u0000\u0000_\u000B\u0000\u0000\u0000\u0000\u0000\u0000`</verbatim>,option2=false";
static char global_longs_as_csv [] = "option=1,junk=<verbatim>\\\u000B\u0000\u0000\u0000\u0000\u0000\u0000]\u000B\u0000\u0000\u0000\u0000\u0000\u0000^\u000B\u0000\u0000\u0000\u0000\u0000\u0000_\u000B\u0000\u0000\u0000\u0000\u0000\u0000`</verbatim>";


struct string_fixture
{

  // std::string four_digits = "0->1->2->3";
  // std::vector<std::string> four_digits_as_strings = {"0","1","2","3"};
  // std::string four_commands = "ls(test=0)->cd(any=..1..)->cp(from=to2)->rm(what=4)";
  // std::vector<std::string> four_keys = {"ls","cd","cp","rm"};
  // std::vector<std::string> four_values = {"test=0","any=..1..","from=to2","what=4"};
  // std::string four_keywords = "ls->cd->cp->rm";

  // std::string multiple_args = "value=42,first=2,long_string=1-2-3-4";
  // std::map<std::string,std::string> multiple_kv = {
  //   {"value", "42"},
  //   {"first", "2"},
  //   {"long_string", "1-2-3-4"}
  // };

  // std::string one_key = "ls";
  // std::string one_value = "test=0";
  // std::string one_command = "ls(test=0)";
  // std::string quantiser_bug_20160520 = "quantiser(decode_lut_string=254:766:1278:1790:2302:2814:3326:3838:4350:4862:5374)->h264(option=1)";
  // std::string anything_goes_in_literals = "step1->step2(option=1)->step3(junk=<verbatim>;->,'.==/[],./</verbatim>)->step4(option1=true,option2=false)";
  // std::string verbatim_literal_last = "step_i->step_ii(option=1)->step_iii(option1=true,option2=false)->step_iv(junk=<verbatim>;->,'./[],./</verbatim>)";

  // std::string serialized_longs = "step1->step2(option=1)->step3(junk=<verbatim>\\\u000B\u0000\u0000\u0000\u0000\u0000\u0000]\u000B\u0000\u0000\u0000\u0000\u0000\u0000^\u000B\u0000\u0000\u0000\u0000\u0000\u0000_\u000B\u0000\u0000\u0000\u0000\u0000\u0000`</verbatim>,option2=false)->step4(option1=true,option2=false)";

  // std::string junk_only = "junk=<verbatim>\\\u000B\u0000\u0000\u0000\u0000\u0000\u0000]\u000B\u0000\u0000\u0000\u0000\u0000\u0000^\u000B\u0000\u0000\u0000\u0000\u0000\u0000_\u000B\u0000\u0000\u0000\u0000\u0000\u0000`</verbatim>,option2=false";

  // std::string longs_as_csv = "option=1,junk=<verbatim>\\\u000B\u0000\u0000\u0000\u0000\u0000\u0000]\u000B\u0000\u0000\u0000\u0000\u0000\u0000^\u000B\u0000\u0000\u0000\u0000\u0000\u0000_\u000B\u0000\u0000\u0000\u0000\u0000\u0000`</verbatim>";
  // std::string parsing_0_githubissue_2 = "remove_background(threshold=0";

  std::string four_digits ;
  std::vector<std::string> four_digits_as_strings ;
  std::string four_commands ;
  std::vector<std::string> four_keys ;
  std::vector<std::string> four_values ;
  std::string four_keywords ;

  std::string multiple_args ;
  std::map<std::string,std::string> multiple_kv = {
    {"value", "42"},
    {"first", "2"},
    {"long_string", "1-2-3-4"}
  };

  std::string one_key ;
  std::string one_value ;
  std::string one_command ;
  std::string quantiser_bug_20160520 ;
  std::string anything_goes_in_literals ;
  std::string verbatim_literal_last ;
  std::string parsing_0_githubissue_2 ;

  std::string serialized_longs ;
  std::string junk_only ;
  std::string longs_as_csv ;

  string_fixture():
    four_digits("0->1->2->3"),
    four_digits_as_strings({"0","1","2","3"}),
    four_commands("ls(test=0)->cd(any=..1..)->cp(from=to2)->rm(what=4)"),
    four_keys({"ls","cd","cp","rm"}),
    four_values({"test=0","any=..1..","from=to2","what=4"}),
    four_keywords("ls->cd->cp->rm"),
    multiple_args("value=42,first=2,long_string=1-2-3-4"),
    multiple_kv({
        {"value", "42"},
        {"first", "2"},
        {"long_string", "1-2-3-4"}
      }),
    one_key("ls"),
    one_value("test=0"),
    one_command("ls(test=0)"),
    quantiser_bug_20160520("quantiser(decode_lut_string=254:766:1278:1790:2302:2814:3326:3838:4350:4862:5374)->h264(option=1)"),
    anything_goes_in_literals("step1->step2(option=1)->step3(junk=<verbatim>),->,'.==/[],./</verbatim>)->step4(option1=true,option2=false)" ),
    verbatim_literal_last("step_i->step_ii(option=1)->step_iii(option1=true,option2=false)->step_iv(junk=<verbatim>),->,'./[],./</verbatim>)"),
    parsing_0_githubissue_2("remove_background(threshold=0"),

    //difficult, string literals contain null character (hence are chopped off at assignment)
    serialized_longs(),
    junk_only(),
    longs_as_csv()

    {

      serialized_longs = std::string(&global_serial_longs[0],  sizeof(global_serial_longs)-1);//-1 for the null terminator
      junk_only = std::string(&global_junk_only[0], sizeof(global_junk_only)-1);//-1 for the null terminator
      longs_as_csv = std::string(&global_longs_as_csv[0],  sizeof(global_longs_as_csv)-1);//-1 for the null terminator

    }
};


BOOST_FIXTURE_TEST_SUITE( splitters, string_fixture )

BOOST_AUTO_TEST_CASE (fixture_picked_correctly) {

  BOOST_CHECK_EQUAL(serialized_longs.size(),sizeof(global_serial_longs)-1);
  BOOST_CHECK_EQUAL(serialized_longs.capacity(),serialized_longs.size());

  BOOST_REQUIRE_EQUAL_COLLECTIONS(serialized_longs.begin(),serialized_longs.begin()+20,
                                  &global_serial_longs[0],&global_serial_longs[0] + 20);

  std::string serialized_longs_end_expected = "false)";

  BOOST_CHECK_EQUAL(serialized_longs.substr(serialized_longs.size()-serialized_longs_end_expected.size(),
                                            serialized_longs_end_expected.size()
                      ),
                    serialized_longs_end_expected);
}

BOOST_AUTO_TEST_CASE (split_string_ref) {

  boost::string_ref local_ref(four_digits);
  std::string sep = "->";
  boost::string_ref sep_ref(sep);
  auto splitted = sqy::split_string_ref_by(local_ref,sep_ref);
  BOOST_CHECK_EQUAL(splitted.empty(),
            false);

  BOOST_CHECK_EQUAL_COLLECTIONS(splitted.begin(), splitted.end(),
                four_digits_as_strings.begin(), four_digits_as_strings.end());

  boost::string_ref junk_ref(junk_only);
  sep = ",";
  sep_ref = (sep);
  splitted = sqy::split_string_ref_by(junk_ref,sep_ref);
  BOOST_CHECK_EQUAL(splitted.empty(),
                    false);
  BOOST_CHECK_EQUAL(splitted.size(),
                    2u);

  auto second_key = splitted[1].substr(0,7);
  auto second_value = splitted[1].substr(8);
  BOOST_CHECK_EQUAL(second_key,"option2");
  BOOST_CHECK_EQUAL(second_value,"false");
}


BOOST_AUTO_TEST_SUITE_END()


// BOOST_FIXTURE_TEST_SUITE( boost_parsing, string_fixture )

// BOOST_AUTO_TEST_CASE (parse_digits) {
//   auto parsed_to_map = sqy::unordered_parse_by(four_keywords.begin(),
//                         four_keywords.end(),
//                         "->");
//   BOOST_CHECK_EQUAL(parsed_to_map.empty(),
//          false);
//   BOOST_CHECK_EQUAL(parsed_to_map.size(),
//          four_keys.size());
// }

// BOOST_AUTO_TEST_CASE (parse_correctly) {
//   auto parsed_to_map = sqy::unordered_parse_by(four_keywords.begin(),
//                   four_keywords.end(),
//                   "->");
//   BOOST_REQUIRE_EQUAL(parsed_to_map.empty(),
//          false);

//   int count = 0;
//   for( auto & key : four_keys ){
//     if(parsed_to_map.find(key) != parsed_to_map.end())
//       count++;
//   }

//   BOOST_CHECK_EQUAL(count,
//          four_keys.size());
// }

// BOOST_AUTO_TEST_CASE (parse_preserves_order) {
//   auto parsed_to_vec = sqy::parse_by(four_keywords.begin(),
//                   four_keywords.end(),
//                   "->");
//   BOOST_REQUIRE_EQUAL(parsed_to_vec.empty(),
//            false);


//   for( std::size_t i = 0;i<parsed_to_vec.size();++i ){
//     BOOST_CHECK_EQUAL(parsed_to_vec[i].first,four_keys[i]) ;
//   }


// }

// BOOST_AUTO_TEST_CASE (parse_preserves_order_and_value) {
//   auto parsed_to_vec = sqy::parse_by(four_commands.begin(),
//                   four_commands.end(),
//                   "->");
//   BOOST_REQUIRE_EQUAL(parsed_to_vec.empty(),
//            false);
//   BOOST_REQUIRE_EQUAL(parsed_to_vec.size(),
//            4);


//   for( std::size_t i = 0;i<parsed_to_vec.size();++i ){
//     BOOST_CHECK_EQUAL(parsed_to_vec[i].first,four_keys[i]) ;
//     BOOST_CHECK_EQUAL(parsed_to_vec[i].second,four_values[i]) ;
//   }


// }

// BOOST_AUTO_TEST_CASE (parse_without_token) {
//   auto parsed_to_vec = sqy::parse_by(one_command.begin(),
//                   one_command.end(),
//                   "->");
//   BOOST_REQUIRE_EQUAL(parsed_to_vec.empty(),
//            false);

//   BOOST_REQUIRE_EQUAL(parsed_to_vec.size(),
//            1);

//   BOOST_REQUIRE_EQUAL(parsed_to_vec.at(0).first,
//            one_key);

//   BOOST_REQUIRE_EQUAL(parsed_to_vec.at(0).second,
//            one_value);
// }

// BOOST_AUTO_TEST_CASE (speed_split_by) {

//   std::ostringstream msg;
//   size_t n_items = (1 << 12);
//   for( size_t i = 0; i < n_items;++i){
//     msg << "item" << i << "=" << i;
//     if(i!=(n_items-1))
//       msg << ",";
//   }

//   const std::string payload = msg.str();
//   std::vector<std::string> my_pairs;
//   std::vector<std::string> karma_pairs;

//   auto start_t = std::chrono::high_resolution_clock::now();
//   for(int r = 0;r<10;++r)
//     my_pairs = sqy::split_string_by(payload);
//   auto end_t = std::chrono::high_resolution_clock::now();
//   std::chrono::duration<double, std::micro> my_split_mus = (end_t-start_t);

//   start_t = std::chrono::high_resolution_clock::now();
//   for(int r = 0;r<10;++r)
//     karma_pairs = sqy::split_by(payload.begin(), payload.end());
//   end_t = std::chrono::high_resolution_clock::now();
//   std::chrono::duration<double, std::micro> karma_split_mus = (end_t-start_t);

//   BOOST_TEST_MESSAGE("my_split " << my_split_mus.count() << " mus, karma_split " << karma_split_mus.count() << " mus\n");

//   //on ivy bridge laptop (under release conditions with gcc 5.3.1)
//   //y_split 1711.59 mus, karma_split 4713.37 mus
//   BOOST_REQUIRE_LT(my_split_mus.count(),karma_split_mus.count());
//   BOOST_CHECK_EQUAL_COLLECTIONS(my_pairs.begin(), my_pairs.end(),
//              karma_pairs.begin(), karma_pairs.end());
// }

// BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( custom_parsing,
                          string_fixture )

BOOST_AUTO_TEST_CASE (parse_digits) {
  auto parsed_to_map = sqy::parse_string_by(four_keywords,
                        "->");
  BOOST_CHECK_EQUAL(parsed_to_map.empty(),
            false);
  BOOST_CHECK_EQUAL(parsed_to_map.size(),
            four_keys.size());

}

BOOST_AUTO_TEST_CASE (parse_correctly) {
  auto parsed_to_map = sqy::parse_string_by(four_keywords,
                     "->");
  BOOST_REQUIRE_EQUAL(parsed_to_map.empty(),
            false);

  int count = 0;
  for( auto & key : four_keys ){
    if(parsed_to_map.find(key) != parsed_to_map.end())
      count++;
  }

  BOOST_CHECK_EQUAL((std::size_t)count,
                    four_keys.size());
}


BOOST_AUTO_TEST_CASE (parse_to_map) {


  std::vector<std::string> pairs = sqy::split_string_by(multiple_args,",");
  BOOST_CHECK_EQUAL(pairs.size(),3u);
  BOOST_TEST_MESSAGE("split_by \'" << multiple_args << "\'");
  for(auto p : pairs)
    BOOST_TEST_MESSAGE(">> " << p);

  sqy::parsed_map_t value = sqy::parse_string_by(multiple_args);

  BOOST_CHECK_EQUAL(value.size(),3);

  BOOST_TEST_MESSAGE("parse_string_by \'" << multiple_args << "\'");
  for(auto v : value)
    BOOST_CHECK_MESSAGE(v.second == multiple_kv[v.first],
            "values for " << v.first << " do not match: "
            << v.second << " != " << multiple_kv[v.first]);

}

BOOST_AUTO_TEST_CASE (parse_to_map_with_parser) {


  std::vector<std::string> pairs = sqy::split_string_by(multiple_args,",");
  BOOST_CHECK_EQUAL(pairs.size(),3);
  BOOST_TEST_MESSAGE("split_by \'" << multiple_args << "\'");
  for(auto p : pairs)
    BOOST_TEST_MESSAGE(">> " << p);

  sqy::pipeline_parser p;

  sqy::parsed_map_t value = p.minors(multiple_args.begin(), multiple_args.end());

  BOOST_CHECK_EQUAL(value.size(),3);

  BOOST_TEST_MESSAGE("parse_string_by \'" << multiple_args << "\'");
  for(auto v : value)
    BOOST_CHECK_MESSAGE(v.second == multiple_kv[v.first],
            "values for " << v.first << " do not match: "
            << v.second << " != " << multiple_kv[v.first]);

}

BOOST_AUTO_TEST_CASE (anything_goes_test) {

  std::string sep = "->";

  sqy::pipeline_parser p;
  sqy::vec_of_pairs_t value = p.to_pairs(anything_goes_in_literals.begin(),anything_goes_in_literals.end());
  BOOST_REQUIRE_EQUAL(value.size(),4);
  BOOST_CHECK_EQUAL(value[0].first, "step1");
  BOOST_CHECK_EQUAL(value[1].first, "step2");
  BOOST_CHECK_EQUAL(value[2].first, "step3");
  BOOST_CHECK_EQUAL(value[3].first, "step4");
  BOOST_CHECK_EQUAL(value.front().second, "");
  BOOST_CHECK_EQUAL(value[1].second, "option=1");

  std::string expected("junk=<verbatim>),->,'.==/[],./</verbatim>");
  BOOST_CHECK_EQUAL(value[2].second.size(), expected.size());
  BOOST_CHECK_EQUAL(value[2].second, expected);
  BOOST_CHECK_EQUAL(value.back().second, "option1=true,option2=false");

}

BOOST_AUTO_TEST_CASE (anything_goes_major_keys) {


  sqy::pipeline_parser p;
  auto value = p.major_keys(anything_goes_in_literals.begin(),anything_goes_in_literals.end());

  BOOST_REQUIRE_EQUAL(value.size(),4);
  BOOST_CHECK_EQUAL(value[0], "step1");
  BOOST_CHECK_EQUAL(value[1], "step2");
  BOOST_CHECK_EQUAL(value[2], "step3");
  BOOST_CHECK_EQUAL(value[3], "step4");

}



BOOST_AUTO_TEST_CASE (anything_goes_in_literals_ref) {

  std::string sep = "->";

  boost::string_ref ref(anything_goes_in_literals);

  boost::string_ref sep_ref(sep);

  auto value = sqy::split_string_ref_by(ref,sep_ref);

  BOOST_REQUIRE_EQUAL(value.size(),4);
  BOOST_CHECK_EQUAL(value[0].substr(0,5), "step1");
  BOOST_CHECK_EQUAL(value[1].substr(0,5), "step2");
  BOOST_CHECK_EQUAL(value[2].substr(0,5), "step3");
  BOOST_CHECK_EQUAL(value[3].substr(0,5), "step4");

  sqy::pipeline_parser parser;
  auto parsed_map = parser(ref.begin(), ref.end());

  BOOST_CHECK_EQUAL(parsed_map["step1"].empty(), true);
  BOOST_CHECK_EQUAL(parsed_map["step2"].size(), 1);

  BOOST_CHECK_EQUAL(parsed_map["step3"].size(), 1);
  BOOST_CHECK(parsed_map["step3"].find("junk") != parsed_map["step3"].end());

  BOOST_CHECK_EQUAL(parsed_map["step4"].size(), 2);

}


BOOST_AUTO_TEST_CASE (anything_goes_in_literals_to_pairs) {

  std::string sep = "->";

  boost::string_ref ref(anything_goes_in_literals);

  boost::string_ref sep_ref(sep);

  auto value = sqy::split_string_ref_by(ref,sep_ref);

  BOOST_REQUIRE_EQUAL(value.size(),4);
  BOOST_CHECK_EQUAL(value[0].substr(0,5), "step1");
  BOOST_CHECK_EQUAL(value[1].substr(0,5), "step2");
  BOOST_CHECK_EQUAL(value[2].substr(0,5), "step3");
  BOOST_CHECK_EQUAL(value[3].substr(0,5), "step4");

  sqy::pipeline_parser parser;
  auto parsed_pairs = parser.to_pairs(ref.begin(), ref.end());

  BOOST_REQUIRE_EQUAL(parsed_pairs.size(), 4);
  BOOST_CHECK_EQUAL(parsed_pairs.front().first, "step1");
  BOOST_CHECK_EQUAL(parsed_pairs[1].first, "step2");
  BOOST_CHECK_EQUAL(parsed_pairs[2].first, "step3");
  BOOST_CHECK_EQUAL(parsed_pairs.back().first, "step4");

  // BOOST_CHECK_EQUAL(parsed_pairs["step3"].size(), 1);
  // BOOST_CHECK(parsed_pairs["step3"].find("junk") != parsed_pairs["step3"].end());

  // BOOST_CHECK_EQUAL(parsed_pairs["step4"].size(), 2);

}



BOOST_AUTO_TEST_CASE (verbatim_literal_last_ref) {

  std::string sep = "->";

  boost::string_ref ref(verbatim_literal_last);

  boost::string_ref sep_ref(sep);

  auto value = sqy::split_string_ref_by(ref,sep_ref);

  BOOST_REQUIRE_EQUAL(value.size(),4);
  BOOST_CHECK_EQUAL(value[0].substr(0,6), "step_i");
  BOOST_CHECK_EQUAL(value[1].substr(0,7), "step_ii");
  BOOST_CHECK_EQUAL(value[2].substr(0,8), "step_iii");
  BOOST_CHECK_EQUAL(value[3].substr(0,7), "step_iv");

  sqy::pipeline_parser parser;
  auto parsed_map = parser(ref.begin(), ref.end());

  BOOST_CHECK_EQUAL(parsed_map["step_i"].empty(), true);
  BOOST_CHECK_EQUAL(parsed_map["step_ii"].size(), 1);

  BOOST_CHECK_EQUAL(parsed_map["step_iii"].size(), 2);
  BOOST_CHECK_EQUAL(parsed_map["step_iii"]["option1"], "true");
  BOOST_CHECK_EQUAL(parsed_map["step_iii"]["option2"], "false");

  BOOST_CHECK_EQUAL(parsed_map["step_iv"].size(), 1);
  BOOST_CHECK(parsed_map["step_iv"].find("junk") != parsed_map["step_iii"].end());

}



BOOST_AUTO_TEST_CASE (serialized_longs_test) {

  std::string sep = "->";
  auto splitted = sqy::split_string_ref_by(serialized_longs,
                       sep);
  BOOST_CHECK_EQUAL(splitted.size(),4);

  sqy::pipeline_parser parser;

  auto parsed_map = parser(serialized_longs.begin(),serialized_longs.end());

  BOOST_REQUIRE_EQUAL(parsed_map.size(),4);
  BOOST_REQUIRE(parsed_map.find("step1") != parsed_map.end());
  BOOST_REQUIRE(parsed_map.find("step2") != parsed_map.end());
  BOOST_REQUIRE(parsed_map.find("step3") != parsed_map.end());
  BOOST_REQUIRE(parsed_map.find("step4") != parsed_map.end());
  BOOST_CHECK_EQUAL(parsed_map["step1"].empty(), true);
  BOOST_CHECK_EQUAL(parsed_map["step2"].empty(), false);

  BOOST_REQUIRE_EQUAL(parsed_map["step2"].size(), 1);
  BOOST_REQUIRE(parsed_map["step2"].find("option") != parsed_map["step2"].end());
  BOOST_CHECK_EQUAL(parsed_map["step2"]["option"], "1");

  BOOST_REQUIRE_EQUAL(parsed_map["step3"].size(), 2);
  BOOST_REQUIRE(parsed_map["step3"].find("junk") != parsed_map["step2"].end());
  BOOST_REQUIRE(parsed_map["step3"].find("option2") != parsed_map["step2"].end());

  boost::string_ref step3_junk = parsed_map["step3"]["junk"];
  BOOST_CHECK_GT(step3_junk.size(), 0u);
  BOOST_CHECK_EQUAL(step3_junk.starts_with("<verbatim>"), true);
  BOOST_CHECK_EQUAL(step3_junk.ends_with("</verbatim>"), true);

  BOOST_REQUIRE_EQUAL(parsed_map["step4"].size(), 2);
}


BOOST_AUTO_TEST_SUITE_END()


struct verbatim_fixture
{

  const std::size_t len;
  std::vector<std::uint8_t >  range8;
  std::vector<std::uint32_t> range32;

  std::size_t bytes_range8 ;
  std::size_t bytes_range32;

  verbatim_fixture():
    len(32),
    range8(len),
    range32(len),
    bytes_range8 (0),
    bytes_range32(0)
  {

    std::uint32_t counter =0;
    for( auto& el : range8)
      el = counter++;

    counter =0;
    for( auto& el : range32)
      el = counter++;

    bytes_range8  = len*sizeof(std::uint8_t );
    bytes_range32 = len*sizeof(std::uint32_t);
  }
};

BOOST_FIXTURE_TEST_SUITE( make_verbatim, verbatim_fixture )

BOOST_AUTO_TEST_CASE (encode_range8) {


  auto verbatim = sqy::parsing::range_to_verbatim(range8.begin(),
                           range8.end());
  BOOST_CHECK_EQUAL(verbatim.empty(),
            false);

  BOOST_CHECK_GT(verbatim.size(),
         bytes_range8);

  BOOST_CHECK_EQUAL(verbatim.size(),sqy::parsing::verbatim_bytes(range8.begin(),
                           range8.end()));

  std::string last_entry(verbatim.end()-1-sqeazy::ignore_this_delimiters.second.size(),
                         verbatim.end()-sqeazy::ignore_this_delimiters.second.size());

  const std::uint8_t* last_ptr = reinterpret_cast<const std::uint8_t*>(last_entry.data());
  BOOST_CHECK(*last_ptr == range8.back() || *last_ptr == '=');

}

BOOST_AUTO_TEST_CASE (encode_range32) {


  auto verbatim = sqy::parsing::range_to_verbatim(range32.begin(),
                                                  range32.end());
  BOOST_CHECK_EQUAL(verbatim.empty(),
                    false);

  BOOST_CHECK_GT(verbatim.size(),
                 bytes_range32);

  BOOST_CHECK_EQUAL(verbatim.size(),sqy::parsing::verbatim_bytes(range32.begin(),
                                                                 range32.end()));



}


BOOST_AUTO_TEST_CASE (rt_range8) {

  auto decoded_range8 = range8;
  std::fill(decoded_range8.begin(), decoded_range8.end(),0);

  auto verbatim = sqy::parsing::range_to_verbatim(range8.begin(),
                                                  range8.end());

  BOOST_CHECK_EQUAL(verbatim.empty(),
                    false);

  auto res = sqy::parsing::verbatim_to_range(verbatim,
                                             decoded_range8.begin(),
                                             decoded_range8.end());

  BOOST_CHECK(res == decoded_range8.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(range8.begin(), range8.end(),
                decoded_range8.begin(), decoded_range8.end());
}

BOOST_AUTO_TEST_CASE (rt_range32) {

  auto decoded_range32 = range32;
  std::fill(decoded_range32.begin(), decoded_range32.end(),0);

  auto verbatim = sqy::parsing::range_to_verbatim(range32.begin(),
                          range32.end());
  BOOST_CHECK_EQUAL(verbatim.empty(),
            false);

  auto res = sqy::parsing::verbatim_to_range(verbatim,
                         decoded_range32.begin(),
                         decoded_range32.end());

  BOOST_CHECK(res == decoded_range32.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(range32.begin(), range32.end(),
                decoded_range32.begin(), decoded_range32.end());
}

BOOST_AUTO_TEST_CASE (quantiser_bug) {

  std::vector<std::uint16_t> buffer = {1, 2, 4, 8, 16, 32, 64, 128, 65535};
  auto decoded_buffer = buffer;
  std::fill(decoded_buffer.begin(), decoded_buffer.end(), 0);

  auto verbatim = sqy::parsing::range_to_verbatim(buffer.begin(),
                                                  buffer.end());
  BOOST_CHECK_EQUAL(verbatim.empty(),
                    false);

  auto res = sqy::parsing::verbatim_to_range(verbatim,
                                             decoded_buffer.begin(),
                                             decoded_buffer.end());

  BOOST_CHECK(res == decoded_buffer.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(buffer.begin(), buffer.end(),
                                decoded_buffer.begin(), decoded_buffer.end());
}

BOOST_AUTO_TEST_SUITE_END()
