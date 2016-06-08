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
// $Id: G4VVisCommand.cc,v 1.10 2001/09/10 10:49:51 johna Exp $
// GEANT4 tag $Name: geant4-04-00 $

// Base class for visualization commands - John Allison  9th August 1998
// It is really a messenger - we have one command per messenger.

#include "G4VVisCommand.hh"

#include "G4UImanager.hh"
#include "G4UnitsTable.hh"
#include "g4std/strstream"

G4VVisCommand::~G4VVisCommand () {}

G4VisManager* G4VVisCommand::fpVisManager = 0;

G4std::vector <G4UIcommand*> G4VVisCommand::sceneNameCommands;

G4std::vector <G4UIcommand*> G4VVisCommand::sceneHandlerNameCommands;

G4std::vector <G4UIcommand*> G4VVisCommand::viewerNameCommands;

G4double G4VVisCommand::ValueOf(G4String unitName) {
   return G4UnitDefinition::GetValueOf(unitName);
}

G4String G4VVisCommand::ConvertToString(G4bool blValue)
{
  G4String vl = "false";
  if(blValue) vl = "true";
  return vl;
}

G4String G4VVisCommand::ConvertToString
(G4double x, G4double y, const char * unitName)
{
  G4double uv = ValueOf(unitName);
  
  char st[50];
  G4std::ostrstream os(st,50);
  os << x/uv << " " << y/uv << " " << unitName << G4std::ends;
  G4String vl = st;
  return vl;
}

G4String G4VVisCommand::ConvertToString(const G4ThreeVector& vec)
{
  char st[100];
  G4std::ostrstream os(st,100);
  os << vec.x() << " " << vec.y() << " " << vec.z() << G4std::ends;
  G4String vl = st;
  return vl;
}

G4bool G4VVisCommand::GetNewBoolValue(const G4String& paramString)
{
  G4String v = paramString;
  v.toUpper();
  G4bool vl = false;
  if( v=="Y" || v=="YES" || v=="1" || v=="T" || v=="TRUE" )
  { vl = true; }
  return vl;
}

G4int G4VVisCommand::GetNewIntValue(const G4String& paramString)
{
  G4int vl;
  const char* t = paramString;
  G4std::istrstream is((char*)t);
  is >> vl;
  return vl;
}

G4double G4VVisCommand::GetNewDoubleValue(const G4String& paramString)
{
  G4double vl;
  char unts[30];
  
  const char* t = paramString;
  G4std::istrstream is((char*)t);
  is >> vl >> unts;
  G4String unt = unts;
  
  return (vl*ValueOf(unt));
}

G4ThreeVector G4VVisCommand::GetNew3VectorValue(const G4String& paramString)
{
  G4double vx;
  G4double vy;
  G4double vz;
  const char* t = paramString;
  G4std::istrstream is((char*)t);
  is >> vx >> vy >> vz;
  return G4ThreeVector(vx,vy,vz);
}

void G4VVisCommand::GetNewDoublePairValue(const G4String& paramString,
					  G4double& xval,
					  G4double& yval)
{
  G4double x, y;
  char unts[30];
  
  const char* t = paramString;
  G4std::istrstream is((char*)t);
  is >> x >> y >> unts;
  G4String unt = unts;

  xval = x*ValueOf(unt);
  yval = y*ValueOf(unt);

  return;
}

void G4VVisCommand::UpdateVisManagerScene
(const G4String& sceneName) {

  G4VisManager::Verbosity verbosity = fpVisManager->GetVerbosity();

  G4SceneList& sceneList = fpVisManager -> SetSceneList ();

  G4int iScene, nScenes = sceneList.size ();
  for (iScene = 0; iScene < nScenes; iScene++) {
    if (sceneList [iScene] -> GetName () == sceneName) break;
  }

  G4Scene* pScene = 0;  // Zero unless scene has been found...
  if (iScene < nScenes) {
    pScene = sceneList [iScene];
  }

  if (!pScene) {
    if (verbosity >= G4VisManager::warnings) {
      G4cout << "WARNING: Scene \"" << sceneName << "\" not found."
	     << G4endl;
    }
    return;
  }

  fpVisManager -> SetCurrentScene (pScene);

  // Scene has changed.  Trigger a rebuild of graphical database...
  G4VViewer* pViewer = fpVisManager -> GetCurrentViewer();
  G4VSceneHandler* sceneHandler = fpVisManager -> GetCurrentSceneHandler();
  if (sceneHandler && sceneHandler -> GetScene ()) {
    if (pViewer && pViewer -> GetViewParameters().IsAutoRefresh()) {
      G4UImanager::GetUIpointer () ->
	ApplyCommand ("/vis/scene/notifyHandlers");
    }
  }
}