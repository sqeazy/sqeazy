#define __SQY_BENCH_CPP__
#include <iostream>
#include <functional>
#include <chrono>
#include <iomanip>
#include <numeric>
#include <vector>
#include <unordered_map>
#include "bench_fixtures.hpp"
#include "bench_utils.hpp"
#include "hist_impl.hpp"
#include "pipeline.hpp"
#include "sqeazy_impl.hpp"
#include "external_encoders.hpp"

typedef sqeazy::bmpl::vector< sqeazy::remove_estimated_background<unsigned short>,
        sqeazy::diff_scheme<unsigned short>,
        sqeazy::bitswap_scheme<unsigned short>,
        sqeazy::lz4_scheme<unsigned short> > rmbkg_diff_bswap1_lz4;
typedef sqeazy::pipeline<rmbkg_diff_bswap1_lz4> rmbkg_diff_bswap1_lz4_pipe;

typedef sqeazy::bmpl::vector< sqeazy::diff_scheme<unsigned short>,
        sqeazy::bitswap_scheme<unsigned short>,
        sqeazy::lz4_scheme<unsigned short> > diff_bswap1_lz4;
typedef sqeazy::pipeline<diff_bswap1_lz4> diff_bswap1_lz4_pipe;

typedef sqeazy::bmpl::vector< sqeazy::diff_scheme<unsigned short>,
        sqeazy::lz4_scheme<unsigned short> > diff_lz4;
typedef sqeazy::pipeline<diff_lz4> diff_lz4_pipe;

typedef sqeazy::bmpl::vector< sqeazy::bitswap_scheme<unsigned short>,
        sqeazy::lz4_scheme<unsigned short> > bswap1_lz4;
typedef sqeazy::pipeline<bswap1_lz4> bswap1_lz4_pipe;

typedef sqeazy::bmpl::vector< sqeazy::lz4_scheme<unsigned short> > lz4_;
typedef sqeazy::pipeline<lz4_> lz4_pipe;


template <typename MapT>
int print_help(const MapT& _av_targets) {



    std::string me = "bench";
    std::cout << "usage: " << me << " <flags> <target>\n"
              << "flags:\n\t -v \t\t print benchmark stats for every file\n"
              << "availble targets:\n"
              << "\t help\n";

    auto mbegin = _av_targets.cbegin();
    auto mend = _av_targets.cend();

    for(; mbegin!=mend; ++mbegin) {
        std::cout << "\t "<< mbegin->first <<" <files_to_encode>\n";
    }

    std::cout << "\n";

    return 1;
}



template <typename T, typename PipeType>
void fill_suite(const std::vector<std::string>& _args, sqeazy_bench::bsuite<T>& _suite) {

    typedef T value_type;
    typedef PipeType current_pipe;

    std::cout << "fill_suite :: " << PipeType::name() << "\n";
    unsigned num_files = _args.size();

    if(_suite.size() != num_files)
        _suite.cases.resize(num_files);


    for(unsigned i = 0; i < num_files; ++i) {

        tiff_fixture<value_type> reference(_args[i]);

        if(reference.empty())
            continue;

        sqeazy_bench::bcase<value_type> temp_case(_args[i], reference.data(), reference.axis_lengths);

        temp_case.return_code = current_pipe::encode(reference.data(),
                                reinterpret_cast<char*>(reference.output()),
                                reference.axis_lengths);

        temp_case.stop(current_pipe::last_num_encoded_bytes);

        _suite.at(i,temp_case);

    }



}

int main(int argc, char *argv[])
{

    typedef std::function<void(const std::vector<std::string>&, sqeazy_bench::bsuite<unsigned short>&) > func_t;
    std::unordered_map<std::string, func_t> prog_flow;

    prog_flow["rmbkg_diff_bswap1_lz4_compress"] = func_t(fill_suite<unsigned short, rmbkg_diff_bswap1_lz4_pipe>);
    prog_flow["diff_bswap1_lz4_compress"] = func_t(fill_suite<unsigned short, diff_bswap1_lz4_pipe>);
    prog_flow["bswap1_lz4_compress"] = func_t(fill_suite<unsigned short, bswap1_lz4_pipe>);
    prog_flow["diff_lz4_compress"] = func_t(fill_suite<unsigned short, diff_lz4_pipe>);
    prog_flow["lz4_compress"] = func_t(fill_suite<unsigned short, lz4_pipe>);

    prog_flow["rmbkg_diff_bswap1_lz4_speed"] = func_t(fill_suite<unsigned short, rmbkg_diff_bswap1_lz4_pipe>);
    prog_flow["diff_bswap1_lz4_speed"] = func_t(fill_suite<unsigned short, diff_bswap1_lz4_pipe>);
    prog_flow["bswap1_lz4_speed"] = func_t(fill_suite<unsigned short, bswap1_lz4_pipe>);
    prog_flow["diff_lz4_speed"] = func_t(fill_suite<unsigned short, diff_lz4_pipe>);
    prog_flow["lz4_speed"] = func_t(fill_suite<unsigned short, lz4_pipe>);

    int retcode = 0;
    if(argc>1) {
        unsigned int f_index = 1;

        std::unordered_map<std::string, func_t>::const_iterator f_func = prog_flow.end();

        std::vector<std::string> args(argv+1, argv+argc);
        bool verbose = sqeazy_bench::contains_and_pop(args, std::string("-v"));

        for(f_index = 0; f_index<args.size(); ++f_index) {
            if((f_func = prog_flow.find(args[f_index])) != prog_flow.end())
                break;
        }

        std::vector<std::string> filenames(args.begin()+1, args.end());
        sqeazy_bench::bsuite<unsigned short> suite(filenames.size());
        if(f_func!=prog_flow.end()) {
            (f_func->second)(filenames, suite);
            int prec = std::cout.precision();


            if(verbose)
                suite.print_cases();

            std::cout << f_func->first << "\t" << filenames.size() << " files ";

            if(f_func->first.find("compress")!=std::string::npos) {
                std::cout << std::setw(12) << "ratio (out/in)\t";
                std::cout.precision(5);
                suite.compression_summary();

            } else {
                std::cout << std::setw(12) << "speed (MB/s)\t";
                std::cout.precision(5);
                suite.speed_summary();

            }

            std::cout.precision(prec);
        } else {

            std::cerr << "unable to detect target in\n";
            for( const std::string& word : args ) {
                std::cerr << word << "\n";
            }
            std::cerr << "\navailable are\n";
	    for( auto& pair : prog_flow ) {
                std::cerr << pair.first << "\n";
            }

        }
    }
    else {
        retcode = print_help(prog_flow);
    }

    return retcode;
}
