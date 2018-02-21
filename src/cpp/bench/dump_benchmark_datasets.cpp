#include <string>
#include <sstream>

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "benchmark_fixtures.hpp"
#include "tiff_utils.hpp"

namespace opts = boost::program_options;
typedef sqeazy::benchmark::dynamic_synthetic_data<> dynamic_default_fixture;
namespace bfs = boost::filesystem;

int main(int argc, char ** argv)
{
    opts::options_description po;
    po.add_options()
        ("help,h", "produce help message")
        ("verbose,v", "enable verbose output")
        ("nitems,n", opts::value<int>()->default_value(32 << 20), "number of items to use")
        ("output,o", opts::value<std::string>()->default_value("synthetic.tif"), "file name of the output")
    ;

    opts::variables_map po_vm;
    opts::parsed_options po_parsed = opts::command_line_parser(argc, argv)
        .options(po)
        .run();
    opts::store(po_parsed, po_vm);
    opts::notify(po_vm);

    if(po_vm.count("help")) {
        std::cout << po << '\n';
        return 1;
    }

    std::cout << "generating 3 datasets of size " << po_vm["nitems"].as<int>() << " 16-bit elements\n";

    dynamic_default_fixture fix;
    fix.setup(po_vm["nitems"].as<int>());

    bfs::path output = po_vm["output"].as<std::string>();

    sqeazy::tiff_facet sinus(fix.sinus_.data(),fix.sinus_.data()+fix.sinus_.size(),fix.shape_);
    sqeazy::tiff_facet embryo(fix.embryo_.data(),fix.embryo_.data()+fix.sinus_.size(),fix.shape_);
    sqeazy::tiff_facet noisy_embryo(fix.noisy_embryo_.data(),fix.noisy_embryo_.data()+fix.sinus_.size(),fix.shape_);

    bfs::path sinusf = output.parent_path();
    sinusf += "sinus-";
    sinusf += output.filename();

    sinus.write(sinusf.generic_string(),16);

    bfs::path embryof = output.parent_path();
    embryof += "embryo-";
    embryof += output.filename();
    embryo.write(embryof.generic_string(),16);

    bfs::path noisy_embryof = output.parent_path();
    noisy_embryof += "noisy_embryo-";
    noisy_embryof += output.filename();
    noisy_embryo.write(noisy_embryof.generic_string(),16);

    return 0;
}
