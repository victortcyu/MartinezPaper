#include "PhononDetectorConstruction.hh"
#include "PhononSensitivity.hh"
#include "G4CMPLogicalBorderSurface.hh"
#include "G4CMPPhononElectrode.hh"
#include "G4CMPSurfaceProperty.hh"
#include "G4Box.hh"
#include "G4Colour.hh"
#include "G4GeometryManager.hh"
#include "G4LatticeLogical.hh"
#include "G4LatticeManager.hh"
#include "G4LatticePhysical.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Material.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
#include "G4SolidStore.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4TransportationManager.hh"
#include "G4Tubs.hh"
#include "G4UserLimits.hh"
#include "G4VisAttributes.hh"


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

PhononDetectorConstruction::PhononDetectorConstruction()
    : fGalactic(0), fSi(0), fAl(0), fTeflon(0),
    fWorldPhys(0), fSiSlab(0), fKID(0), fFeedline(0), 
    fTeflon0(0), fTeflon1(0), fTeflon2(0), fTeflon3(0),
    siVacuum(0), siAl(0), siTeflon(0), electrodeSensitivity(0),
    fConstructed(false) {
    ;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

PhononDetectorConstruction::~PhononDetectorConstruction() {
    delete siVacuum;
    delete siAl;
    delete siTeflon;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

// deals with the physical geometry, clearing it if necessary, then setting up
G4VPhysicalVolume* PhononDetectorConstruction::Construct()
{
    if (fConstructed) {
        if (!G4RunManager::IfGeometryHasBeenDestroyed()) {
            // Run manager hasn't cleaned volume stores. This code shouldn't execute
            G4GeometryManager::GetInstance()->OpenGeometry();
            G4PhysicalVolumeStore::GetInstance()->Clean();
            G4LogicalVolumeStore::GetInstance()->Clean();
            G4SolidStore::GetInstance()->Clean();
        }
        // Have to completely remove all lattices to avoid warning on reconstruction
        G4LatticeManager::GetLatticeManager()->Reset();
        // Clear all LogicalSurfaces
        // NOTE: No need to redefine the G4CMPSurfaceProperties
        G4CMPLogicalBorderSurface::CleanSurfaceTable();
    }

    DefineMaterials();
    SetupGeometry();
    fConstructed = true;

    return fWorldPhys;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void PhononDetectorConstruction::DefineMaterials()
{
    G4NistManager* nist = G4NistManager::Instance();

    fGalactic = nist->FindOrBuildMaterial("G4_Galactic");
    fSi = nist->FindOrBuildMaterial("G4_Si");
    fAl = nist->FindOrBuildMaterial("G4_Al");
    fTeflon = nist->FindOrBuildMaterial("G4_TEFLON");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void PhononDetectorConstruction::SetupGeometry()
{
    auto SetColour = [&](G4LogicalVolume* lv, G4double p_r, G4double p_g, G4double p_b) {
        auto vis = new G4VisAttributes(G4Colour(p_r, p_g, p_b));
        vis->SetVisibility(true);
        lv->SetVisAttributes(vis);
        };

    // G4bool checkOverlaps = true;

    G4double world_size = 30.0 * mm;
    G4Box* solidWorld = new G4Box("World", 0.5 * world_size, 0.5 * world_size, 0.5 * world_size);
    G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, fGalactic, "World");
    fWorldPhys = new G4PVPlacement(0, G4ThreeVector(), logicWorld, "World", 0, false, 0 /*, checkOverlaps*/);

    G4double si_x = 20.0 * mm;
    G4double si_y = 20.0 * mm;
    G4double si_z = 0.380 * mm;
    G4Box* solidSi = new G4Box("SiliconSubstrate", 0.5 * si_x, 0.5 * si_y, 0.5 * si_z);
    G4LogicalVolume* logicSi = new G4LogicalVolume(solidSi, fSi, "SiliconSubstrate");
    fSiSlab = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicSi, "SiliconSubstrate", logicWorld, false, 0);
    SetColour(logicSi, 0.2, 0.2, 0.8);

    G4double top_surface_z = 0.5 * si_z;
    G4double kid_x = 2.0 * mm;
    G4double kid_y = 2.0 * mm;
    G4double al_thickness_z = 60.0 * nm;

    G4double feedline_x = 20.0 * mm;
    G4double feedline_y = 72.0 * um;

    G4Box* solidKID = new G4Box("KID", 0.5 * kid_x, 0.5 * kid_y, 0.5 * al_thickness_z);
    G4LogicalVolume* logicKID = new G4LogicalVolume(solidKID, fAl, "KID");
    G4double kid_z_pos = top_surface_z + 0.5 * al_thickness_z;
    fKID = new G4PVPlacement(0, G4ThreeVector(0, 0.5 * kid_y + 0.5 * feedline_y, kid_z_pos), logicKID, "KID", logicWorld, false, 0);
    SetColour(logicKID, 0.8, 0.2, 0.8);


    G4Box* solidFeedline = new G4Box("Feedline", 0.5 * feedline_x, 0.5 * feedline_y, 0.5 * al_thickness_z);
    G4LogicalVolume* logicFeedline = new G4LogicalVolume(solidFeedline, fAl, "Feedline");
    fFeedline = new G4PVPlacement(0, G4ThreeVector(0, 0, kid_z_pos), logicFeedline, "Feedline", logicWorld, false, 1);
    SetColour(logicFeedline, 0.8, 0.2, 0.2);

    G4double teflon_radius = 3 * mm;
    G4double teflon_height = 200 * um; // Assumed height for visualization
    G4Tubs* solidTeflon = new G4Tubs("TeflonSupport", 0, teflon_radius, 0.5 * teflon_height, 0, 360 * deg);
    G4LogicalVolume* logicTeflon = new G4LogicalVolume(solidTeflon, fTeflon, "TeflonSupport");
    SetColour(logicTeflon, 0.2, 0.2, 0.2);

    G4double teflon_xy_offset = 11 * mm;
    G4double teflon_z_pos = top_surface_z + 0.5 * teflon_height;

    // so the overlap area is 2.18mm, which is less than 3mm, as in the paper
    fTeflon0 = new G4PVPlacement(0, G4ThreeVector(teflon_xy_offset, teflon_xy_offset, teflon_z_pos), logicTeflon, "TeflonSupport0", logicWorld, false, 0);
    fTeflon1 = new G4PVPlacement(0, G4ThreeVector(teflon_xy_offset, -teflon_xy_offset, teflon_z_pos), logicTeflon, "TeflonSupport1", logicWorld, false, 1);
    fTeflon2 = new G4PVPlacement(0, G4ThreeVector(-teflon_xy_offset, teflon_xy_offset, teflon_z_pos), logicTeflon, "TeflonSupport2", logicWorld, false, 2);
    fTeflon3 = new G4PVPlacement(0, G4ThreeVector(-teflon_xy_offset, -teflon_xy_offset, teflon_z_pos), logicTeflon, "TeflonSupport3", logicWorld, false, 3);

    G4LatticeManager* LM = G4LatticeManager::GetLatticeManager();
    G4LatticeLogical* SiLogical = LM->LoadLattice(fSi, "Si");

    G4LatticePhysical* SiPhysical = new G4LatticePhysical(SiLogical);
    SiPhysical->SetMillerOrientation(1, 0, 0);
    LM->RegisterLattice(fSiSlab, SiPhysical);


    if (!fConstructed) {
        const G4double GHz = 1e9 * hertz;
        
        const std::vector<G4double> noAnhCoeff = {0};
        const std::vector<G4double> fullSpecCoeff = { 1 };
        const std::vector<G4double> noDiffuseCoeff = { 0 };
        const G4double AnhCutoff = 15000., ReflCutoff = 15000;
        
        siVacuum = new G4CMPSurfaceProperty("siVacuum", 
            1.0, 0.0, 0.0, 0.0,   // q absorption, q refl, e min k (to absorb), hole min k
            0.0, 1.0, 1.0, 0.0);  // phonon abs, phonon refl (implying transmission), ph specular, p min k
        siVacuum->AddScatteringProperties(AnhCutoff, ReflCutoff, noAnhCoeff,
            noDiffuseCoeff, fullSpecCoeff, GHz, GHz, GHz);

        siAl = new G4CMPSurfaceProperty("siAl",
            1.0, 0.0, 0.0, 0.0,   // q absorption, q refl, e min k (to absorb), hole min k
            1.0, 1.0, 1.0, 0.0);  // phonon abs, phonon refl (implying transmission), ph specular, p min k
        siAl->AddScatteringProperties(AnhCutoff, ReflCutoff, noAnhCoeff,
            noDiffuseCoeff, fullSpecCoeff, GHz, GHz, GHz);

        siTeflon = new G4CMPSurfaceProperty("siTeflon",
            1.0, 0.0, 0.0, 0.0,   // q absorption, q refl, e min k (to absorb), hole min k
            0.0, 1.0, 1.0, 0.0);  // phonon abs, phonon refl (implying transmission), ph specular, p min k
        siTeflon->AddScatteringProperties(AnhCutoff, ReflCutoff, noAnhCoeff,
            noDiffuseCoeff, fullSpecCoeff, GHz, GHz, GHz);
    }

    new G4CMPLogicalBorderSurface("siVacuum", fSiSlab, fWorldPhys,
        siVacuum);
    new G4CMPLogicalBorderSurface("siKID", fSiSlab, fKID,
        siAl);
    new G4CMPLogicalBorderSurface("siFeedline", fSiSlab, fFeedline,
        siAl);
    new G4CMPLogicalBorderSurface("siTeflon0", fSiSlab, fTeflon0,
        siTeflon);
    new G4CMPLogicalBorderSurface("siTeflon1", fSiSlab, fTeflon1,
        siTeflon);
    new G4CMPLogicalBorderSurface("siTeflon2", fSiSlab, fTeflon2,
        siTeflon);
    new G4CMPLogicalBorderSurface("siTeflon3", fSiSlab, fTeflon3,
        siTeflon);

    logicWorld->SetVisAttributes(G4VisAttributes::Invisible);
    G4VisAttributes* simpleBoxVisAtt = new G4VisAttributes(G4Colour(1.0, 1.0, 1.0));
    simpleBoxVisAtt->SetVisibility(true);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

// Attach material properties and electrode/sensor handler to surface

void PhononDetectorConstruction::
AttachPhononSensor(G4CMPSurfaceProperty* surfProp) {
    if (!surfProp) return;		// No surface, nothing to do

    // Specify properties of aluminum sensor, same on both detector faces
    // See G4CMPPhononElectrode.hh or README.md for property keys

    // Properties must be added to existing surface-property table
    auto sensorProp = surfProp->GetPhononMaterialPropertiesTablePointer();
    sensorProp->AddConstProperty("filmAbsorption", 0.20);    // True sensor area
    sensorProp->AddConstProperty("filmThickness", 600. * nm);
    sensorProp->AddConstProperty("gapEnergy", 173.715e-6 * eV);
    sensorProp->AddConstProperty("lowQPLimit", 3.);
    sensorProp->AddConstProperty("phononLifetime", 242. * ps);
    sensorProp->AddConstProperty("phononLifetimeSlope", 0.29);
    sensorProp->AddConstProperty("vSound", 3.26 * km / s);
    sensorProp->AddConstProperty("subgapAbsorption", 0.1);

    // Attach electrode object to handle KaplanQP interface
    surfProp->SetPhononElectrode(new G4CMPPhononElectrode);
}

