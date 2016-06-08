//
// ********************************************************************
// * DISCLAIMER                                                       *
// *                                                                  *
// * The following disclaimer summarizes all the specific disclaimers *
// * of contributors to this software. The specific disclaimers,which *
// * govern, are listed with their locations in:                      *
// *   http://cern.ch/geant4/license                                  *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.                                                             *
// *                                                                  *
// * This  code  implementation is the  intellectual property  of the *
// * GEANT4 collaboration.                                            *
// * By copying,  distributing  or modifying the Program (or any work *
// * based  on  the Program)  you indicate  your  acceptance of  this *
// * statement, and all its terms.                                    *
// ********************************************************************
//
//
// $Id: ExN01DetectorConstruction.cc,v 1.5 2002/01/09 17:23:48 ranjard Exp $
// GEANT4 tag $Name: geant4-04-00-patch-01 $
//

#include "ExN01DetectorConstruction.hh"

#include "G4Material.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4PVPlacement.hh"
#include "globals.hh"

ExN01DetectorConstruction::ExN01DetectorConstruction()
 :  experimentalHall_log(0), tracker_log(0),
    calorimeterBlock_log(0), calorimeterLayer_log(0),
    experimentalHall_phys(0), calorimeterLayer_phys(0),
    calorimeterBlock_phys(0), tracker_phys(0)
{;}

ExN01DetectorConstruction::~ExN01DetectorConstruction()
{
  delete calorimeterLayer_log;
  delete calorimeterBlock_log;
  delete tracker_log;
  delete experimentalHall_log;
  delete tracker_phys;
  delete calorimeterBlock_phys;
  delete calorimeterLayer_phys;
  delete experimentalHall_phys;
}

G4VPhysicalVolume* ExN01DetectorConstruction::Construct()
{

  //------------------------------------------------------ materials

  G4double a;  // atomic mass
  G4double z;  // atomic number
  G4double density;
  G4String name;

  a = 39.95*g/mole;
  density = 1.782e-03*g/cm3;
  G4Material* Ar = new G4Material(name="ArgonGas", z=18., a, density);

  a = 26.98*g/mole;
  density = 2.7*g/cm3;
  G4Material* Al = new G4Material(name="Aluminum", z=13., a, density);

  a = 207.19*g/mole;
  density = 11.35*g/cm3;
  G4Material* Pb = new G4Material(name="Lead", z=82., a, density);

  //------------------------------------------------------ volumes

  //------------------------------ experimental hall (world volume)
  //------------------------------ beam line along x axis

  G4double expHall_x = 3.0*m;
  G4double expHall_y = 1.0*m;
  G4double expHall_z = 1.0*m;
  G4Box* experimentalHall_box
    = new G4Box("expHall_box",expHall_x,expHall_y,expHall_z);
  experimentalHall_log = new G4LogicalVolume(experimentalHall_box,
                                             Ar,"expHall_log",0,0,0);
  experimentalHall_phys = new G4PVPlacement(0,G4ThreeVector(),"expHall",
                                            experimentalHall_log,0,false,0);

  //------------------------------ a tracker tube

  G4double innerRadiusOfTheTube = 0.*cm;
  G4double outerRadiusOfTheTube = 60.*cm;
  G4double hightOfTheTube = 50.*cm;
  G4double startAngleOfTheTube = 0.*deg;
  G4double spanningAngleOfTheTube = 360.*deg;
  G4Tubs* tracker_tube = new G4Tubs("tracker_tube",innerRadiusOfTheTube,
                                    outerRadiusOfTheTube,hightOfTheTube,
                                    startAngleOfTheTube,spanningAngleOfTheTube);
  tracker_log = new G4LogicalVolume(tracker_tube,Al,"tracker_log",0,0,0);
  G4double trackerPos_x = -1.0*m;
  G4double trackerPos_y = 0.*m;
  G4double trackerPos_z = 0.*m;
  tracker_phys = new G4PVPlacement(0,
             G4ThreeVector(trackerPos_x,trackerPos_y,trackerPos_z),
             tracker_log,"tracker",experimentalHall_log,false,0);

  //------------------------------ a calorimeter block

  G4double block_x = 1.0*m;
  G4double block_y = 50.0*cm;
  G4double block_z = 50.0*cm;
  G4Box* calorimeterBlock_box = new G4Box("calBlock_box",block_x,
                                          block_y,block_z);
  calorimeterBlock_log = new G4LogicalVolume(calorimeterBlock_box,
                                             Pb,"caloBlock_log",0,0,0);
  G4double blockPos_x = 1.0*m;
  G4double blockPos_y = 0.0*m;
  G4double blockPos_z = 0.0*m;
  calorimeterBlock_phys = new G4PVPlacement(0,
             G4ThreeVector(blockPos_x,blockPos_y,blockPos_z),
             calorimeterBlock_log,"caloBlock",experimentalHall_log,false,0);

  //------------------------------ calorimeter layers

  G4double calo_x = 1.*cm;
  G4double calo_y = 40.*cm;
  G4double calo_z = 40.*cm;
  G4Box* calorimeterLayer_box = new G4Box("caloLayer_box",
                                          calo_x,calo_y,calo_z);
  calorimeterLayer_log = new G4LogicalVolume(calorimeterLayer_box,
                                             Al,"caloLayer_log",0,0,0);
  for(G4int i=0;i<19;i++) // loop for 19 layers
  {
    G4double caloPos_x = (i-9)*10.*cm;
    G4double caloPos_y = 0.0*m;
    G4double caloPos_z = 0.0*m;
    calorimeterLayer_phys = new G4PVPlacement(0,
               G4ThreeVector(caloPos_x,caloPos_y,caloPos_z),
               calorimeterLayer_log,"caloLayer",calorimeterBlock_log,false,i);
  }

  //------------------------------------------------------------------

  return experimentalHall_phys;
}
