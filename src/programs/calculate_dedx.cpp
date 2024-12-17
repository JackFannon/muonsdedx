#include <iostream>

#include "TBranch.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TH1F.h"
#include "TObject.h"
#include "TStyle.h"
#include "TTree.h"

#include "WCSimRootEvent.hh"

int main(int argc, char **argv) {
    // Check that we have a file in the arguments
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input file>" << std::endl;
        return 1;
    }

    // Open the file
    TFile *wcsim_file = new TFile(argv[1], "READ");

    // Check that the file is open
    if (!wcsim_file->IsOpen()) {
        std::cerr << "Failed to open file: " << argv[1] << std::endl;
        return 1;
    }

    // Get the WCSim tree from the file
    TTree *wcsim_tree = (TTree *)wcsim_file->Get("wcsimT");
    TBranch *event_branch = wcsim_tree->GetBranch("wcsimrootevent");
    WCSimRootEvent *wcsimrootsuperevent = new WCSimRootEvent();
    event_branch->SetAddress(&wcsimrootsuperevent);

    WCSimRootTrigger *trigger;

    // Set number of events to loop over here
    int nentries = wcsim_tree->GetEntries();
    // Print in green
    std::cout << "\033[1;32m" << "Number of entries: " << nentries << "\033[0m" << std::endl;
    // nentries = 2;

    TH1F *h1 = new TH1F("h1", "h1", 300, 0.5, 0.8);

    // Create a map that maps all of the PDG values to the names of the particles
    std::map<int, std::string> pdgToParticleName = {{11, "Electron"},
                                                    {-11, "Positron"},
                                                    {12, "Electron Neutrino"},
                                                    {-12, "Electron Anti-Neutrino"},
                                                    {13, "Muon"},
                                                    {-13, "Anti-Muon"},
                                                    {14, "Muon Neutrino"},
                                                    {-14, "Muon Anti-Neutrino"},
                                                    {15, "Tau"},
                                                    {-15, "Anti-Tau"},
                                                    {16, "Tau Neutrino"},
                                                    {-16, "Tau Anti-Neutrino"},
                                                    {22, "Photon"},
                                                    {111, "Pi0"},
                                                    {211, "Pi+"},
                                                    {-211, "Pi-"},
                                                    {130, "K0 Long"},
                                                    {310, "K0 Short"},
                                                    {311, "K0"},
                                                    {321, "K+"},
                                                    {-321, "K-"},
                                                    {2212, "Proton"},
                                                    {-2212, "Anti-Proton"},
                                                    {2112, "Neutron"},
                                                    {-2112, "Anti-Neutron"},
                                                    {-311, "Anti-K0"},
                                                    {0, "Geantino"},
                                                    {3112, "Sigma-"},
                                                    {3122, "Lambda"},
                                                    {3222, "Sigma+"},
                                                    {1000010020, "Deuteron"},
                                                    {1000010030, "Triton"},
                                                    {1000020030, "Helium-3"},
                                                    {1000020040, "Alpha"}};

    // Create a map for strings to ints
    std::map<std::string, int> process_map;

    std::map<int, int> particle_type_map;

    // Loop over the events in the tree
    for (int event = 0; event < nentries; event++) {
        // Get the entry in the tree
        wcsim_tree->GetEntry(event);
        // Get the trigger
        trigger = wcsimrootsuperevent->GetTrigger(0);

        std::cout << "There are " << trigger->GetNvtxs() << " vertices in this event" << std::endl;
        for (int i = 0; i < trigger->GetNvtxs(); i++) {
            // std::cout << "Vertex " << i << " is at " << trigger->GetVtxs(i, 0) << ", " << trigger->GetVtxs(i, 1)
            //           << ", " << trigger->GetVtxs(i, 2) << std::endl;
        }
        WCSimRootTrigger *wcsimrootevent = wcsimrootsuperevent->GetTrigger(0);

        int ntrack = wcsimrootevent->GetNtrack();

        for (int j = 0; j < ntrack; j++) {
            TObject *element = (wcsimrootevent->GetTracks()->At(j));

            WCSimRootTrack *wcsimroottrack = (WCSimRootTrack *)(element);

            h1->Fill(wcsimroottrack->GetE());

            if (!(wcsimroottrack->GetIpnu() == 11 && wcsimroottrack->GetE() < 0.8)) {

                if (particle_type_map.find(wcsimroottrack->GetIpnu()) == particle_type_map.end()) {
                    particle_type_map[wcsimroottrack->GetIpnu()] = 1;
                } else {
                    particle_type_map[wcsimroottrack->GetIpnu()]++;
                }
            }

            if (wcsimroottrack->GetIpnu() == 11 && wcsimroottrack->GetE() < 0.8) {
                std::string process = wcsimroottrack->GetCreatorProcessName();
                if (process_map.find(process) == process_map.end()) {
                    process_map[process] = 1;
                } else {
                    process_map[process]++;
                }
            }

            if ((wcsimroottrack->GetParenttype() == 0)) {
                std::cout << " ======== TRACK " << j << " ========" << std::endl;
                std::cout << "Parent type (PDG):  " << wcsimroottrack->GetParenttype() << std::endl;

                std::cout << "Flag:               " << wcsimroottrack->GetFlag() << std::endl;

                std::cout << "PDG:                " << wcsimroottrack->GetIpnu() << std::endl;

                std::cout << "Mass:               " << wcsimroottrack->GetM() << std::endl;

                std::cout << "Position start:    " << wcsimroottrack->GetStart(0) << " "
                          << wcsimroottrack->GetStart(1) << " " << wcsimroottrack->GetStart(2) << std::endl;
                std::cout << "Position end:      " << wcsimroottrack->GetStop(0) << " "
                          << wcsimroottrack->GetStop(1) << " " << wcsimroottrack->GetStop(2) << std::endl;
                std::cout << "Momentum:           " << wcsimroottrack->GetP() << std::endl;

                std::cout << "Energy:             " << wcsimroottrack->GetE() << std::endl;

                std::cout << "Direction:          " << wcsimroottrack->GetDir(0) << " "
                          << wcsimroottrack->GetDir(1) << " " << wcsimroottrack->GetDir(2) << std::endl;

                std::cout << "Direction bef div : " << wcsimroottrack->GetPdir(0) << " "
                          << wcsimroottrack->GetPdir(1) << " " << wcsimroottrack->GetPdir(2) << std::endl;

                std::cout << "ENERGY: " << wcsimroottrack->GetE() << std::endl;
                std::cout << "MASS: " << wcsimroottrack->GetM() << std::endl;
                float diff = wcsimroottrack->GetE() - wcsimroottrack->GetM();
                std::cout << "Difference between energy and mass " << diff << std::endl;
                std::cout << "Process producing the track " << wcsimroottrack->GetCreatorProcess() << std::endl;

                std::cout << " ===== END OF TRACK " << j << " =====" << std::endl << std::endl;
            }
        }
    }

    // Create a th1f and fill it with the process map values with the process names on the x axis
    TH1F *h2 = new TH1F("h2", "h2", process_map.size(), 0, process_map.size());
    int i = 1;
    for (auto it = process_map.begin(); it != process_map.end(); it++) {
        h2->GetXaxis()->SetBinLabel(i, it->first.c_str());
        h2->SetBinContent(i, it->second);
        i++;
    }

    i = 1;
    TH1F *id_hist = new TH1F("id_hist", "id_hist", particle_type_map.size(), 0, particle_type_map.size());
    for (auto it = particle_type_map.begin(); it != particle_type_map.end(); it++) {
        std::cout << "Setting bin " << it->first << " to " << pdgToParticleName[it->first] << std::endl;
        if (pdgToParticleName.find(it->first) == pdgToParticleName.end()) {
            std::cout << "Particle not found in map with id " << it->first << std::endl;
        }
        id_hist->GetXaxis()->SetBinLabel(i, pdgToParticleName[it->first].c_str());
        id_hist->SetBinContent(i, it->second);
        i++;
    }

    // don't tilt the x-axis labels
    id_hist->LabelsOption("v");
    // increase bottom padding
    gStyle->SetPadBottomMargin(0.35);

    TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
    // make the y-axis log
    c1->SetLogy();
    h1->Draw();
    c1->SaveAs("h1.C");
    c1->Clear();

    id_hist->Draw();
    c1->SetLogy(false);
    c1->SaveAs("id_hist.pdf");
    c1->Clear();

    c1->SetLogy(0);
    h2->Draw();
    c1->SaveAs("h2.C");

    return 0;
}
