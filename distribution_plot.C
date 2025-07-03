#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "TString.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TStyle.h"
#include <sstream>

std::vector<std::string> getCSVTokens(const std::string& line) {
    std::vector<std::string> tokens;
    std::string current_token;
    std::stringstream ss(line);
    while (std::getline(ss, current_token, ',')) {
        tokens.push_back(current_token);
    }
    return tokens;
}

void distribution_plot(const TString& fileName = "phonon_tracking.csv")
{
    // --- Plotting Style ---
    gStyle->SetOptStat(0); // Turn off statistics box

    // --- Constants and Bins ---
    const int nBins = 200;
    const double freqMin_THz = 0.0;
    const double freqMax_THz = 5.0; // Set max to 5 THz, adjust if needed

    // Conversion factor from phonon energy in meV to frequency in THz
    // E = h*nu  => nu = E/h
    // h = 4.1357e-12 meV*s
    // nu (Hz) = E (meV) / 4.1357e-12
    // nu (THz) = E (meV) / 4.1357
    const double meV_to_THz = 1.0 / 4.1357;

    std::map<TString, TH1D*> histos;
    histos["KID"] = new TH1D("h_kid", "Phonon Frequency Spectrum;Phonon frequency (THz);Counts (arb. units)", nBins, freqMin_THz, freqMax_THz);
    histos["Feedline"] = new TH1D("h_feedline", "", nBins, freqMin_THz, freqMax_THz); // No title for subsequent plots
    histos["Teflon"] = new TH1D("h_teflon", "", nBins, freqMin_THz, freqMax_THz);

    // --- Style the Histograms to match the example ---
    histos["KID"]->SetLineColor(kRed);
    histos["KID"]->SetLineWidth(2);

    histos["Teflon"]->SetLineColor(kBlue);
    histos["Teflon"]->SetLineStyle(kDashed);
    histos["Teflon"]->SetLineWidth(2);

    histos["Feedline"]->SetLineColor(kBlack);
    histos["Feedline"]->SetLineStyle(kDashed);
    histos["Feedline"]->SetLineWidth(2);

    // --- Read the data file ---
    std::ifstream fin(fileName.Data());
    if (!fin) {
        std::cout << "Error: Cannot open input file " << fileName << std::endl;
        return;
    }

    std::string line;
    std::getline(fin, line); // Discard the header row

    int lineCount = 0;
    while (std::getline(fin, line)) {
        lineCount++;
        std::vector<std::string> tokens = getCSVTokens(line);

        // Expecting format: trackID,stepNum,x,y,z,time,energy_meV,volume
        if (tokens.size() < 8) {
            std::cout << "Warning: Malformed line #" << lineCount << ": " << line << std::endl;
            continue;
        }

        try {
            double energy_meV = std::stod(tokens[6]);
            TString volumeName = tokens[7];

            if (energy_meV <= 0.) continue;

            double frequency_thz = energy_meV * meV_to_THz;

            // Fill the correct histogram based on volume name
            if (volumeName == "KID") {
                histos["KID"]->Fill(frequency_thz);
            } else if (volumeName == "Feedline") {
                histos["Feedline"]->Fill(frequency_thz);
            } else if (volumeName.BeginsWith("TeflonSupport")) {
                // Group all Teflon supports into one histogram
                histos["Teflon"]->Fill(frequency_thz);
            }
        } catch (const std::invalid_argument& e) {
            std::cout << "Warning: Could not parse number on line #" << lineCount << std::endl;
        }
    }
    fin.close();

    std::cout << "Finished reading " << lineCount << " data lines." << std::endl;

    // --- Create Canvas and Draw ---
    TCanvas* c1 = new TCanvas("c1", "Phonon Frequency Spectra", 800, 600);
    c1->SetLeftMargin(0.12);

    // Find the maximum bin height to set the Y-axis range nicely
    double max_y = 0;
    for (auto const& [key, val] : histos) {
        if (val->GetMaximum() > max_y) {
            max_y = val->GetMaximum();
        }
    }
    if (max_y > 0) {
        // Set the range on the first histogram that will be drawn
        histos["KID"]->GetYaxis()->SetRangeUser(0, max_y * 1.15);
    }
    
    // Draw the histograms on the same canvas
    histos["KID"]->Draw("HIST");
    histos["Teflon"]->Draw("HIST SAME");
    histos["Feedline"]->Draw("HIST SAME");

    // --- Create Legend ---
    TLegend* legend = new TLegend(0.65, 0.70, 0.88, 0.88);
    legend->SetHeader("Component", "C"); // "C" for centered
    legend->AddEntry(histos["KID"], "KID", "l");
    legend->AddEntry(histos["Teflon"], "Teflon", "l");
    legend->AddEntry(histos["Feedline"], "Feedline", "l");
    legend->SetBorderSize(1);
    legend->Draw();

    c1->Update();
}