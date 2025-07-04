/***********************************************************************\
 * This software is licensed under the terms of the GNU General Public *
 * License version 3 or later. See G4CMP/LICENSE for the full license. *
\***********************************************************************/

/// \file exoticphysics/phonon/include/PhononDetectorConstruction.hh
/// \brief Definition of the PhononDetectorConstruction class
//
// $Id: 4c06153e9ea08f2a90b22c53e5c39bde4b847c07 $
//
// 20221006  M. Kelsey -- Remove "IsField" flag, unnecessary with phonons.
//		Add material properties for aluminum phonon sensors

#ifndef PhononDetectorConstruction_h
#define PhononDetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"

class G4Material;
class G4VPhysicalVolume;
class G4CMPSurfaceProperty;
class G4CMPElectrodeSensitivity;


class PhononDetectorConstruction : public G4VUserDetectorConstruction {
public:
  PhononDetectorConstruction();
  virtual ~PhononDetectorConstruction();
  
public:
  virtual G4VPhysicalVolume* Construct();
  
private:
  void DefineMaterials();
  void SetupGeometry();
  void AttachPhononSensor(G4CMPSurfaceProperty* surfProp);

private:
	G4Material* fGalactic;
	G4Material* fSi;
	G4Material* fAl;
	G4Material* fTeflon;

	G4VPhysicalVolume* fWorldPhys;
	G4VPhysicalVolume* fSiSlab;
	G4VPhysicalVolume* fKID;
	G4VPhysicalVolume* fFeedline;
	G4VPhysicalVolume* fTeflon0;
	G4VPhysicalVolume* fTeflon1;
	G4VPhysicalVolume* fTeflon2;
	G4VPhysicalVolume* fTeflon3;

	G4CMPSurfaceProperty* siVacuum;
	G4CMPSurfaceProperty* siAl;
	G4CMPSurfaceProperty* siTeflon;
	G4CMPElectrodeSensitivity* electrodeSensitivity;

	G4bool fConstructed;
};

#endif

