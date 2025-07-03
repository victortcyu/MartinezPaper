#include "PhononSteppingAction.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4Track.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhononTransSlow.hh"
#include "G4PhononTransFast.hh"
#include "G4PhononLong.hh"

// Constructor: open file and write header line
PhononSteppingAction::PhononSteppingAction() {
    fout_.open("phonon_tracking.csv");
    fout_ << "trackID, stepNumber, x/nm, y/nm, z/nm, time_ns, energy_meV, volume\n";
}

// Destructor: close file
PhononSteppingAction::~PhononSteppingAction() {
    if (fout_.is_open()) fout_.close();
}

void PhononSteppingAction::UserSteppingAction(const G4Step* step) {
    auto track = step->GetTrack();
    if (track->GetDefinition() != G4PhononLong::Definition() && 
        track->GetDefinition() != G4PhononTransSlow::Definition() &&
        track->GetDefinition() != G4PhononTransFast::Definition())
        return;

    
    auto prePoint = step->GetPreStepPoint();
    auto postPoint = step->GetPostStepPoint();

    // stop tracking the phonon if it's below 2*Delta ~ 400 ueV
    if (prePoint->GetKineticEnergy() < 400 * eV * 1e-6) {
        auto pos = postPoint->GetPosition();
        auto time = postPoint->GetGlobalTime();
        auto energy = prePoint->GetKineticEnergy();
        fout_ << track->GetTrackID() << "," << track->GetCurrentStepNumber() << ","
            << pos.x() / nm << "," << pos.y() / nm << "," << pos.z() / nm << ","
            << time / ns << "," << energy / eV * 1e3 << ","
            << "BelowGap" << "\n";
        track->SetTrackStatus(fStopAndKill);
    }

    auto prevPhysVol = prePoint->GetPhysicalVolume();

    G4bool correctStatus = track->GetTrackStatus() == fStopAndKill &&
        postPoint->GetStepStatus() == fGeomBoundary &&
        step->GetNonIonizingEnergyDeposit() > 0.;

    if (!prevPhysVol // undefined pointer
        || (postPoint->GetPhysicalVolume()->GetName() != "KID" // not in the correct region
        && postPoint->GetPhysicalVolume()->GetName() != "Feedline"
        && postPoint->GetPhysicalVolume()->GetName() != "TeflonSupport0"
        && postPoint->GetPhysicalVolume()->GetName() != "TeflonSupport1"
        && postPoint->GetPhysicalVolume()->GetName() != "TeflonSupport2"
        && postPoint->GetPhysicalVolume()->GetName() != "TeflonSupport3")
        || !correctStatus  
        || prePoint->GetKineticEnergy() < 400 * eV * 1e-6
        )
        return;

    auto pos = postPoint->GetPosition();       
    auto time = postPoint->GetGlobalTime();    
    auto energy = prePoint->GetKineticEnergy();  

    // note that for such geometric crossings, our post-step point will always be on the boundary
    // thus the z value will not be interesting. However, the step will now be in the new volume
    fout_ << track->GetTrackID() << "," << track->GetCurrentStepNumber() << ","
        << pos.x() / nm << "," << pos.y() / nm << "," << pos.z() / nm << ","
        << time / ns << "," << energy / eV * 1e3 << "," 
        << postPoint->GetPhysicalVolume()->GetName() << "\n";
    
}
