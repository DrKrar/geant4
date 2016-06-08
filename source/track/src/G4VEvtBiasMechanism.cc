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
// $Id: G4VEvtBiasMechanism.cc,v 1.3 2001/07/11 10:08:40 gunter Exp $
// GEANT4 tag $Name: geant4-04-00 $
//
// 
// --------------------------------------------------------------
//	GEANT 4 class implementation file 
//
//	
//	
// ------------------------------------------------------------
//   Implemented for the new scheme                 13 Apr. 1999  H.Kurahige

#include "G4VEvtBiasMechanism.hh"

G4VEvtBiasMechanism::G4VEvtBiasMechanism(const G4String& name):
  theEBName(name)
{
}

G4VEvtBiasMechanism::G4VEvtBiasMechanism(const G4VEvtBiasMechanism& right)
{
  theEBName = right.theEBName;
}

G4VEvtBiasMechanism::~G4VEvtBiasMechanism()
{
}












