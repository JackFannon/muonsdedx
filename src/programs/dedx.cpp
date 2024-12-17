#include <iostream>
#include <memory>
#include <numbers>
#include <vector>

#include "RtypesCore.h"
#include "TBranch.h"
#include "TCanvas.h"
#include "TClonesArray.h"
#include "TCollection.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2D.h"
#include "TKey.h"
#include "TMath.h"
#include "TObject.h"
#include "TTree.h"

#include "WCSimRootEvent.hh"
#include "WCSimRootGeom.hh"

#include "dedx.h"

int main(int argc, char **argv) {

    // Get arguments - input WCSim file and output file
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input WCSim file> <output file>" << std::endl;
        return 1;
    }

    // Load the WCSim file
    std::string WCSimFilePath = argv[1];
    std::unique_ptr<TFile> WCSimFile(TFile::Open(WCSimFilePath.c_str(), "READ"));
    if (!WCSimFile || WCSimFile->IsZombie()) {
        std::cerr << "Error: failed to open WCSim file" << std::endl;
        return 1;
    }

    // Example: Access a specific tree by name and cycle
    const char *treeName = "wcsimT"; // Replace with desired tree name
    int cycle = 1;                   // Replace with desired cycle number

    // Construct the name with cycle and fetch the tree
    std::string treeWithCycle = std::string(treeName) + ";" + std::to_string(cycle);
    TTree *wcSimTree = (TTree *)WCSimFile->Get(treeWithCycle.c_str());
    if (wcSimTree) {
        std::cout << "Successfully accessed tree '" << treeName << "' with cycle " << cycle << std::endl;
        std::cout << "Number of entries: " << wcSimTree->GetEntries() << std::endl;
    } else {
        std::cerr << "Tree '" << treeName << "' with cycle " << cycle << " not found!" << std::endl;
    }

    // Setup the WCSimRootEvent
    WCSimRootEvent *wcSimRootSuperEvent = new WCSimRootEvent();
    TBranch *wcSimRootEventBranch = wcSimTree->GetBranch("wcsimrootevent");
    wcSimRootEventBranch->SetAddress(&wcSimRootSuperEvent);
    if (!wcSimRootEventBranch) {
        std::cerr << "Error: failed to get WCSimRootEvent branch" << std::endl;
        return 1;
    }
    WCSimRootTrigger *wcSimRootEvent;

    TTree *wcSimGeoTree = (TTree *)WCSimFile->Get("wcsimGeoT");
    if (wcSimGeoTree) {
        std::cout << "Successfully accessed geometry tree" << std::endl;
    } else {
        std::cerr << "Geometry tree not found!" << std::endl;
    }
    WCSimRootGeom *geo = 0;
    wcSimGeoTree->SetBranchAddress("wcsimrootgeom", &geo);
    wcSimGeoTree->GetEntry(0);

    // Get the number of entries
    int nEntries = wcSimTree->GetEntries();
    nEntries = 1;
    int startEntry = 2;

    TH1D *hitTimeHist = new TH1D("hitTimeHist", "Hit Time Distribution", 3500, 0, 70000);
    TH2D *hitTimeVsZ = new TH2D("hitTimeVsZ", "Hit Time vs Z", 70, 0, 35, 35, 0, 2000);

    for (int entry = startEntry; entry < startEntry + nEntries; entry++) {
        std::cout << "Processing entry " << entry << std::endl;
        double muonEntry[3] = {0, 0, 0};
        double muonDir[3] = {0, 0, 0};
        double muonTime = 0.;
        wcSimTree->GetEntry(entry);
        wcSimRootEvent = wcSimRootSuperEvent->GetTrigger(0);

        int nTracks = wcSimRootEvent->GetNtrack();

        for (int trackN = 0; trackN < nTracks; trackN++) {
            TObject *element = wcSimRootEvent->GetTracks()->At(trackN);
            WCSimRootTrack *wcSimRootTrack = (WCSimRootTrack *)(element);
            if (wcSimRootTrack->GetFlag() == 0 && wcSimRootTrack->GetParenttype() == 0 &&
                wcSimRootTrack->GetIpnu() == 13) {
                muonDir[0] = wcSimRootTrack->GetDir(0);
                muonDir[1] = wcSimRootTrack->GetDir(1);
                muonDir[2] = wcSimRootTrack->GetDir(2);
                muonEntry[0] = wcSimRootTrack->GetStart(0);
                muonEntry[1] = wcSimRootTrack->GetStart(1);
                muonEntry[2] = wcSimRootTrack->GetStart(2);
                muonTime = wcSimRootTrack->GetTime();
            }
        }

        int nHits = wcSimRootEvent->GetNcherenkovdigihits();
        int nDigitHitSlots = wcSimRootEvent->GetNcherenkovdigihits_slots();
        if (nHits) {
            for (int NdigiHitEvent = 0; NdigiHitEvent < nDigitHitSlots; NdigiHitEvent++) {
                std::vector<double> zValues;
                std::vector<double> tValues;
                TObject *digitHit = wcSimRootEvent->GetCherenkovDigiHits()->At(NdigiHitEvent);
                if (!digitHit) {
                    continue;
                }
                WCSimRootCherenkovDigiHit *wcSimRootCherenkovDigiHit =
                    dynamic_cast<WCSimRootCherenkovDigiHit *>(digitHit);
                int tubeNumber = wcSimRootCherenkovDigiHit->GetTubeId();
                double hitTime = wcSimRootCherenkovDigiHit->GetT();
                WCSimRootPMT pmt = geo->GetPMT(tubeNumber - 1);
                double hitPos[3] = {pmt.GetPosition(0), pmt.GetPosition(1), pmt.GetPosition(2)};
                double muonToHit[3] = {hitPos[0] - muonEntry[0], hitPos[1] - muonEntry[1],
                                       hitPos[2] - muonEntry[2]};
                double magnitudeR = std::sqrt(pow(muonToHit[0], 2) + pow(muonToHit[1], 2) + pow(muonToHit[2], 2));
                double a = calculateA();
                double b = calculateB(muonToHit, muonDir, hitTime, muonTime);
                double c = calculateC(magnitudeR, hitTime, muonTime);
                std::vector<double> roots = quadraticFormula(a, b, c);
                double discriminant = getDiscriminant(a, b, c);
                if (discriminant < 100 && discriminant > 0) {
                    std::cout << "There are: " << roots.size()
                              << " roots to the equation. The discriminant is: " << discriminant << std::endl;
                }
                if (roots.size()) {
                    hitTimeHist->Fill(roots[0]);
                    hitTimeVsZ->Fill(roots[0] / 1000, hitTime);
                    if (roots.size() == 2) {
                        hitTimeHist->Fill(roots[1]);
                        hitTimeVsZ->Fill(roots[1] / 1000, hitTime);
                    }
                }
                double dotProduct =
                    muonToHit[0] * muonDir[0] + muonToHit[1] * muonDir[1] + muonToHit[2] * muonDir[2];
            }
        }
        // if (nHits) {
        //     // Print the hit time of the first hit
        //     TClonesArray *cherenkovHitTimes = wcSimRootEvent->GetCherenkovHitTimes();
        //     TClonesArray *cherenkovDigitHitTimes = wcSimRootEvent->GetCherenkovDigiHits();
        //     for (int i = 0; i < nHits; i++) {
        //         WCSimRootCherenkovHitTime *cherenkovHitTime =
        //             (WCSimRootCherenkovHitTime *)cherenkovHitTimes->At(i);

        //         double hitPos[3] = {cherenkovHitTime->GetPhotonEndPos(0), cherenkovHitTime->GetPhotonEndPos(1),
        //                             cherenkovHitTime->GetPhotonEndPos(2)};
        //         double hitTime = cherenkovHitTime->GetTruetime();
        //         double muonToHit[3] = {hitPos[0] - muonEntry[0], hitPos[1] - muonEntry[1],
        //                                hitPos[2] - muonEntry[2]};
        //         double magnitudeR = std::sqrt(pow(muonToHit[0], 2) + pow(muonToHit[1], 2) + pow(muonToHit[2],
        //         2)); double a = calculateA(); double b = calculateB(muonToHit, muonDir, hitTime, muonTime);
        //         double c = calculateC(magnitudeR, hitTime, muonTime);
        //         std::vector<double> roots = quadraticFormula(a, b, c);
        //         double discriminant = getDiscriminant(a, b, c);
        //         if (discriminant < 100 && discriminant > 0) {
        //             std::cout << "There are: " << roots.size()
        //                       << " roots to the equation. The discriminant is: " << discriminant << std::endl;
        //         }
        //         if (roots.size()) {
        //             hitTimeHist->Fill(roots[0] / 10);
        //             if (roots.size() == 2) {
        //                 hitTimeHist->Fill(roots[1] / 10);
        //             }
        //         }
        //         // hitTimeHist->Fill(hitTime);
        //     }
        // }
    }

    TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
    hitTimeHist->Draw();
    c1->SaveAs("hitTimeHist.C");
    c1->Clear();
    hitTimeVsZ->Draw("colz");
    c1->SaveAs("hitTimeVsZ.C");

    return 0;
}

// Returns the roots of the quadratic equation ax^2 + bx + c = 0 if they exist
std::vector<double> quadraticFormula(double a, double b, double c) {
    double discriminant = getDiscriminant(a, b, c);
    std::vector<double> roots;
    if (discriminant < 0) {
        return roots;
    } else if (discriminant == 0) {
        roots.push_back(-b / (2 * a));
        return roots;
    } else {
        // std::cout << "Discriminant: " << discriminant << std::endl;
        // std::cout << "Upper fraction + : " << (-b + std::sqrt(discriminant)) << std::endl;
        // std::cout << "Upper fraction - : " << (-b - std::sqrt(discriminant)) << std::endl;
        // std::cout << "Lower fraction: " << (2 * a) << std::endl;

        roots.push_back((-b + std::sqrt(discriminant)) / (2 * a));
        roots.push_back((-b - std::sqrt(discriminant)) / (2 * a));
        return roots;
    }
}

// Returns the discriminant of the quadratic equation ax^2 + bx + c = 0
double getDiscriminant(double a, double b, double c) { return pow(b, 2) - 4 * a * c; }

double calculateA() {
    double a = (1 / pow(ng, 2)) - 1;
    // std::cout << "A is: " << a << std::endl;
    return a;
}

double calculateB(double muonToHit[3], double muonEntry[3], double hitTime, double muonEntryTime) {
    double dotProduct = muonToHit[0] * muonEntry[0] + muonToHit[1] * muonEntry[1] + muonToHit[2] * muonEntry[2];
    double b = 2 * (dotProduct - (cVac * (hitTime - muonEntryTime)) / pow(ng, 2));
    // std::cout << "B is: " << b << std::endl;
    return b;
}

double calculateC(double magnitudeR, double hitTime, double muonEntryTime) {
    std::cout << "magnitudeR: " << magnitudeR << std::endl;
    double c = ((pow(cVac, 2) * pow((hitTime - muonEntryTime), 2)) / pow(ng, 2)) - pow(magnitudeR, 2);
    // std::cout << "C is: " << c << std::endl;
    return c;
}

double calculateZ(double longDist, double muonToHit[3], double muonDir[3]) {
    double magnitudeR = std::sqrt(pow(muonToHit[0], 2) + pow(muonToHit[1], 2) + pow(muonToHit[2], 2));
    double dotProduct = muonToHit[0] * muonDir[0] + muonToHit[1] * muonDir[1] + muonToHit[2] * muonDir[2];
    double z = longDist + 34000 * sin(acos(dotProduct / magnitudeR)) * (1 / tan(42 * TMath::Pi() / 180));
    return z;
}

double calculateT(double longDist, double muonToHit[3], double muonDir[3]) {
    double magnitudeR = std::sqrt(pow(muonToHit[0], 2) + pow(muonToHit[1], 2) + pow(muonToHit[2], 2));
    double dotProduct = muonToHit[0] * muonDir[0] + muonToHit[1] * muonDir[1] + muonToHit[2] * muonDir[2];
    double t =
        (1 / cVac) * (longDist + 34000 * sin(acos(dotProduct / magnitudeR))) * (ng / sin(42 * TMath::Pi() / 180));
    return t;
}
