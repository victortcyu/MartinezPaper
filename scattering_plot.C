#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "TH1D.h"
#include "TCanvas.h"
#include "TString.h"

void scattering_plot(const TString& fileName = "phonon_hits.txt")
{
    const int    nBins      = 188;
    const double tMin_us    = 0.0;
    const double tMax_us    = 150.4;
    const double eta_pb     = 0.57;
    const double ph_count   = 1;
    const double ph_energy  = 0.0620; 	// eV
    const double E_input_J  = ph_count * ph_energy * 1.602176634e-19;       // J
    const double xi_tr      = 1;
    const double meV_to_J   = 1.602176634e-22;    // conversion

    TH1D* h = new TH1D("h","Phonon Time-Energy;Time (#mus);Efficiency (%/0.8#mus)",
                       nBins,tMin_us,tMax_us);

    std::ifstream fin(fileName.Data());
    if (!fin) { std::cout<<"Cannot open "<<fileName<<'\n'; return; }

    std::string line;
    std::getline(fin,line);                       // discard header

    double eTot=0, eKID=0, eFeed=0, eTef=0, eGap=0;

    while (std::getline(fin,line))
    {
        std::vector<std::string> tok;
        std::size_t pos=0, last=0;
        while (tok.size()<8 && (pos=line.find(',',last))!=std::string::npos) {
            tok.emplace_back(line.substr(last,pos-last));
            last=pos+1;
        }
        tok.emplace_back(line.substr(last));      // last field (volume)

        if (tok.size()<8) continue;               // malformed

        double time_ns    = std::stod(tok[5]);
        double energy_meV = std::stod(tok[6]);
        const std::string& vol = tok[7];

        if (energy_meV<=0.) continue;

        double eJ = energy_meV*meV_to_J;
        eTot += eJ;

        if      (vol=="KID")      { eKID  += eJ; h->Fill(time_ns*1e-3,eJ); }
        else if (vol=="Feedline") { eFeed += eJ; }
        else if (vol.rfind("TeflonSupport",0)==0) eTef += eJ;
	else if (vol=="BelowGap") { eGap += eJ; }
    }
    fin.close();

    if (eTot==0) { std::cout<<"No valid rows read.\n"; return; }

    double norm = eta_pb / (E_input_J * xi_tr) * 100.0;
    h->Scale(norm);
    h->GetYaxis()->SetTitle("Efficiency (% / 0.8 #mus)");

    new TCanvas("c","Arrival",800,600);
    h->SetLineColor(kBlue+2);  h->Draw("hist");

    std::cout.setf(std::ios::fixed); std::cout.precision(3);
    std::cout<<"Total deposited (eV): "<<eTot / (1.60217e-19)<<'\n'
             <<"Total / E_input  : "<<eTot/E_input_J*100.0<<" %\n"
             <<"KID              : "<<eKID /eTot*100.0<<" %\n"
             <<"Feedline         : "<<eFeed/eTot*100.0<<" %\n"
             <<"Teflon           : "<<eTef /eTot*100.0<<" %\n"
	     <<"BelowGap	 : "<<eGap /eTot*100.0<< "%\n";
    double eta_percent = h->Integral();
    std::cout << "Integral % (η) : " << eta_percent << std::endl;
    int    peakBin = h->GetMaximumBin();
    double yPeak   = h->GetBinContent(peakBin);
    double tPeak   = h->GetBinCenter (peakBin);
    double t90 = -1, t10 = -1;
    for (int b = peakBin; b <= h->GetNbinsX(); ++b) {
         double y = h->GetBinContent(b);
        if (t90 < 0 && y <= 0.9*yPeak) t90 = h->GetBinCenter(b);
        if (t10 < 0 && y <= 0.1*yPeak) { t10 = h->GetBinCenter(b); break; }
    }

    double tau_ph = (t10 > 0 && t90 > 0) ? (t10 - t90)/2.2 : -1;

    std::cout << "t_peak  (us): " << tPeak << std::endl;
    std::cout << "t_90%   (us): " << t90   << std::endl;
    std::cout << "t_10%   (us): " << t10   << std::endl;
    std::cout << "tau_ph  (us): " << tau_ph<< std::endl;
}
