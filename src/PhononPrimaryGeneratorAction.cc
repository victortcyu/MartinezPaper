#include "PhononPrimaryGeneratorAction.hh"

#include "G4Event.hh"
#include "G4Geantino.hh"
#include "G4ParticleGun.hh"
#include "G4RandomDirection.hh"
#include "G4PhononTransFast.hh"
#include "G4PhononTransSlow.hh"
#include "G4PhononLong.hh"
#include "G4SystemOfUnits.hh"
#include <cmath>

PhononPrimaryGeneratorAction::PhononPrimaryGeneratorAction() {
    G4int n_particle = 1;
    fParticleGun = new G4ParticleGun(n_particle);
    fParticleGun->SetParticleDefinition(G4Geantino::Definition());
    fParticleGun->SetParticleMomentumDirection(G4RandomDirection());
    fParticleGun->SetParticleEnergy(0.0620 * eV);
}

PhononPrimaryGeneratorAction::~PhononPrimaryGeneratorAction() {
    delete fParticleGun;
}

void PhononPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
    if (fParticleGun->GetParticleDefinition() == G4Geantino::Definition()) {
        G4double selector = G4UniformRand();
        if (selector < 0.531) {
            fParticleGun->SetParticleDefinition(G4PhononTransSlow::Definition());
        }
        else if (selector < 0.907) {
            fParticleGun->SetParticleDefinition(G4PhononTransFast::Definition());
        }
        else {
            fParticleGun->SetParticleDefinition(G4PhononLong::Definition());
        }
    }

    const G4double RInjection = 2.33 * mm;
    // 1 micron inside, as in the paper
    const G4double zBack = -0.189 * mm;

    G4double r = RInjection * std::sqrt(G4UniformRand());
    G4double phi = 2. * CLHEP::pi * G4UniformRand();
    G4double x = r * std::cos(phi);
    G4double y = r * std::sin(phi);

    fParticleGun->SetParticlePosition(G4ThreeVector(x, y - 6. * mm, zBack));
    fParticleGun->SetParticleMomentumDirection(G4RandomDirection());
    fParticleGun->GeneratePrimaryVertex(anEvent);
}
