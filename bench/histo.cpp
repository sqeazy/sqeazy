#define __SQY_BENCH_CPP__
#include <iostream>
#include <functional>
#include <string>
#include <iomanip>
#include <sstream>
#include <numeric>
#include <vector>
#include <unordered_map>
#include "bench_fixtures.hpp"
#include "hist_impl.hpp"
#include "sqeazy_impl.hpp"

#include "TH1F.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TFile.h"
#include "TPaveText.h"

int print_help() {

    std::string me = "histo";
    std::cout << "usage: " << me << " <target>\n"
              << "availble targets:\n"
              << "\t help\n"
              << "\t plot <file(s)_to_encode>\t plot intensity histogram to .svg\n"
              << "\t darkest_face <fil(s)_to_encode>\t plot intensity histogram of darkest face to .svg\n"
              << "\t info <file(s)_to_encode>\t print statistics information to command line\n"
              << "\n";
    return 1;
}

int help(const std::vector<std::string>& _args) {
    std::cerr << "unknown argument provided:\n";
    if(_args.empty())
        std::cerr << "<none>";
    else
        for( const std::string& el : _args )
            std::cerr << el << " ";
    std::cerr << "\n\n";

    return print_help();
}

template <typename T>
void write_as_svg(const std::vector<sqeazy::histogram<T>* >& _histos,
                  const std::vector<std::string>& _hnames,
                  const std::vector<std::string>& _htitle,
                  const std::string& _basepath
                 ) {

    std::ostringstream rfilename;
    rfilename << _basepath << "/" << _hnames[0] << ".root";
    TFile rfile(rfilename.str().c_str(),"RECREATE");
    rfile.WriteFree();
    gStyle->SetOptStat("");

    unsigned hindex = 0;
    std::string currentname;
    std::string currenttitle;
    std::ostringstream filename;
    std::ostringstream text;
    std::vector<TText*> left(2);
    std::vector<TText*> right(2);
    for(auto sqy_h : _histos) {

        currentname = _hnames[hindex];
        currenttitle = _htitle[hindex];
        
        TH1F hist(currentname.c_str(), currenttitle.c_str(), 256, 0 , sqy_h->largest_populated_bin());
        for(unsigned b = 0; b<sqy_h->largest_populated_bin()+1; ++b) {
            hist.Fill(b,sqy_h->bins[b]);
        }

        TCanvas to_be_written(currentname.c_str(),"",400,300);
        to_be_written.Clear();
        hist.SetTickLength(0.01,"Y");
        to_be_written.SetTopMargin(.2);
        hist.SetStats(0);

        hist.Draw();
        gPad->SetLogy();

        to_be_written.cd();

        TPaveText* pt_left = new TPaveText(0.1,0.8,0.6,0.92,"NBNDC" );
        pt_left->SetTextAlign(11);
        pt_left->SetBorderSize(0);
        pt_left->SetShadowColor(kWhite);
        pt_left->SetFillColor(kWhite);
        text << "median = " << std::setw(10) << sqy_h->median() << " +/- " << std::setw(10) << sqy_h->median_variation();

        left[0] = pt_left->AddText(text.str().c_str());
        text.str("");
        text << "mean   = " << std::setw(10) << sqy_h->mean() << " +/- " << std::setw(10) << sqy_h->mean_variation();

        left[1] = pt_left->AddText(text.str().c_str());
        pt_left->Draw();


        TPaveText* pt_right = new TPaveText(0.6,0.8,0.9,0.92,"NBNDC" );
        pt_right->SetTextAlign(11);
        pt_right->SetBorderSize(0);
        pt_right->SetShadowColor(kWhite);
        pt_right->SetFillColor(kWhite);
        text.str("");
        text << "integral= " << std::setw(10) << sqy_h->integral();
	
        right[0] = pt_right->AddText(text.str().c_str());
	right[0]->SetTextSize(left[0]->GetTextSize());
	right[0]->SetTextFont(left[0]->GetTextFont());
	text.str("");
        text << "mode    = " << std::setw(10) << sqy_h->mode();
	right[1] = pt_right->AddText(text.str().c_str());
	right[1]->SetTextSize(left[1]->GetTextSize());
	right[1]->SetTextFont(left[1]->GetTextFont());
        pt_right->Draw();


        to_be_written.Update();

        std::ostringstream filename;
        filename << _basepath << "/" << currentname << ".svg";
        to_be_written.Print(filename.str().c_str());

        hist.Write();

        delete pt_left;
        delete pt_right;
    }
    rfile.Close();
}

void write_as_svg(const std::vector<TH1F*>& _histos, const std::string& _basename) {

    std::ostringstream rfilename;
    rfilename << _basename << ".root";
    TFile rfile(rfilename.str().c_str(),"RECREATE");
    rfile.WriteFree();
    gStyle->SetOptStat("im");
    for(auto h : _histos) {

        TCanvas to_be_written(h->GetName(),"",400,300);
        to_be_written.Clear();
        h->SetTickLength(0.01,"Y");
        h->Draw();
        gPad->SetLogy();

        to_be_written.Update();
        std::ostringstream filename;
        filename << _basename << h->GetName() << ".svg";
        to_be_written.Print(filename.str().c_str());
        h->Write();
    }
    rfile.Close();
}

template <typename value_type>
int info(const std::vector<std::string>& _args) {

    auto widest_el_itr = std::max_element(_args.begin(), _args.end(), [](const std::string& _a, const std::string& _b) {
        return _a.size() < _b.size();
    });
    std::cout << std::setw(widest_el_itr->size()+2) << "file path"
              << sqeazy::histogram<value_type>::print_header() << "\n";

    for(unsigned fid = 1; fid < _args.size(); ++fid) {


        tiff_fixture<value_type, false> reference(_args[fid]);

        if(reference.empty())
            continue;

        sqeazy::histogram<value_type> hist(reference.data(), reference.size());

        std::cout << std::setw(widest_el_itr->size()+2) << _args[fid] << hist;

    }

    return 0;
}

int plot(const std::vector<std::string>& _args) {

    std::vector<TH1F*> histos(2);



    for(unsigned fid = 1; fid < _args.size(); ++fid) {
        tiff_fixture<unsigned short> reference(_args[fid]);

        if(reference.empty())
            continue;

        sqeazy::histogram<unsigned short> ref_hist(&reference.tiff_data[0],reference.tiff_data.size());

        std::string no_suffix(_args[fid],0,_args[fid].find_last_of("."));
        size_t dash_pos = _args[fid].find_last_of("/");
        std::string basename(_args[fid],dash_pos+1, no_suffix.size() - dash_pos-1);
        basename += "_histo";
        /////////////////////////////////////////////////////////////////////////////////////////////////////
        // histogram for the entire intensity range
        std::ostringstream hname("");
        hname <<  "_fullrange_" << sizeof(unsigned short)*8 << "bit_intensities";
        std::ostringstream htitle("");
        htitle << basename << ";intensity;frequency";
        TH1F full_range(hname.str().c_str(),htitle.str().c_str(),256,0,1 << (sizeof(unsigned short)*8));

        for(unsigned long i = 0; i<ref_hist.num_bins; ++i) {
            full_range.Fill(i,ref_hist.bins[i]);
        }

        full_range.GetXaxis()->SetRangeUser(-20.f*float(full_range.GetXaxis()->GetBinWidth(2)),1 << (sizeof(unsigned short)*8));

        /////////////////////////////////////////////////////////////////////////////////////////////////////
        // histogram from 0 to 1.1*(largest intensity)
        hname.str("");
        hname << "_populatedrange_" << sizeof(unsigned short)*8 << "bit_intensities";

        TH1F populated_range(hname.str().c_str(),htitle.str().c_str(),128,0.9*ref_hist.smallest_populated_bin(),1.1*ref_hist.largest_populated_bin());
        for(unsigned long i = 0; i<ref_hist.num_bins; ++i) {
            populated_range.Fill(i,ref_hist.bins[i]);
        }
        populated_range.GetXaxis()->SetRangeUser(-20.f*populated_range.GetXaxis()->GetBinWidth(2),1.1*ref_hist.largest_populated_bin());

        //   histos.reserve(2);
        histos[0] = (&full_range);
        histos[1] = (&populated_range);
        write_as_svg(histos, basename);
    }



    return 0;

}


int darkest_face(const std::vector<std::string>& _args) {

    typedef unsigned short raw_type;
    std::vector<raw_type> darkest_face;
    //std::vector<TH1F*> histos(1);
    std::vector<std::string> hnames(1);
    std::vector<std::string> htitles(1);
    std::vector<sqeazy::histogram<raw_type>* > histos(1);

    for(unsigned fid = 1; fid < _args.size(); ++fid) {
        tiff_fixture<raw_type> reference(_args[fid]);

        if(reference.empty())
            continue;

        darkest_face.clear();

        std::string no_suffix(_args[fid],0,_args[fid].find_last_of("."));
        size_t dash_pos = _args[fid].find_last_of("/");
        std::string basename(_args[fid],dash_pos+1, no_suffix.size() - dash_pos-1);
        basename += "_darkest_face";

        sqeazy::remove_estimated_background<unsigned short>::extract_darkest_face(
            &reference.tiff_data[0], reference.axis_lengths, darkest_face);
        sqeazy::histogram<unsigned short> ref_hist(darkest_face.begin(), darkest_face.end());

        std::ostringstream hname("");
        hname << basename << "_" << sizeof(unsigned short)*8 << "bit_intensities";
        hnames[0] = hname.str();
        hname << ";intensity I;N";
        htitles[0] = hname.str();


        histos[0] = (&ref_hist);

        write_as_svg(histos, hnames, htitles, "./");
    }

    return 0;
}

int main(int argc, char *argv[])
{

    typedef std::function<int(std::vector<std::string>) > func_t;
    std::unordered_map<std::string, func_t> prog_flow;
    prog_flow["help"] = func_t(help);
    prog_flow["-h"] = func_t(help);
    prog_flow["plot"] = func_t(plot);
    prog_flow["darkest_face"] = func_t(darkest_face);
    prog_flow["info"] = func_t(info<unsigned short>);

    std::vector<std::string> arguments(argv+1,argv + argc);

    int retcode = 0;
    if(argc>1 && prog_flow.find(argv[1])!=prog_flow.end()) {
        retcode = prog_flow[argv[1]](arguments);
    }
    else {
        retcode = help(arguments);
    }

    return retcode;
}
