// HallGeometry.hh
// --------------------------------------------------------------
// History:
//
// Created by Emanuele Leonardi (emanuele.leonardi@roma1.infn.it) 2016-05-31
// --------------------------------------------------------------

#ifndef HallGeometry_H
#define HallGeometry_H 1

#include "G4ThreeVector.hh"

#define HALLGEOMETRY_NUMBER_OF_BLOCKS 16

class HallGeometry
{

public:

  ~HallGeometry();
  static HallGeometry* GetInstance();

private:

  static HallGeometry* fInstance;

protected:

  HallGeometry();

public:

  // Size of gap between concrete blocks of Hall walls
  G4double GetBlockGap() { return fBlockGap; };

  // Length of side of a single concrete block
  G4double GetBlockSide() { return fBlockNominalSide-fBlockGap; }
  
  // Get number of blocks composing wall
  G4int GetNumberOfBlocks() { return HALLGEOMETRY_NUMBER_OF_BLOCKS; }

  // Get position of a block
  G4ThreeVector GetBlockPosition(G4int i) { return fBlockPosition[i]; }

  G4double GetBlockPosX(G4int i) { return fBlockPosition[i].x(); }
  G4double GetBlockPosY(G4int i) { return fBlockPosition[i].y(); }
  G4double GetBlockPosZ(G4int i) { return fBlockPosition[i].z(); }

private:

  G4double fBlockGap;
  G4double fBlockNominalSide;

  G4ThreeVector fBlockPosition[HALLGEOMETRY_NUMBER_OF_BLOCKS];

};

#endif