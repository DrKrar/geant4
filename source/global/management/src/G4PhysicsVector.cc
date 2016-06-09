//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
// $Id: G4PhysicsVector.cc,v 1.49 2010-11-01 13:55:53 gcosmo Exp $
// GEANT4 tag $Name: geant4-09-04-patch-02 $
//
// 
// --------------------------------------------------------------
//      GEANT 4 class implementation file
//
//  G4PhysicsVector.cc
//
//  History:
//    02 Dec. 1995, G.Cosmo : Structure created based on object model
//    03 Mar. 1996, K.Amako : Implemented the 1st version
//    01 Jul. 1996, K.Amako : Hidden bin from the user introduced
//    12 Nov. 1998, K.Amako : A bug in GetVectorLength() fixed
//    11 Nov. 2000, H.Kurashige : use STL vector for dataVector and binVector
//    18 Jan. 2001, H.Kurashige : removed ptrNextTable
//    09 Mar. 2001, H.Kurashige : added G4PhysicsVector type 
//    05 Sep. 2008, V.Ivanchenko : added protections for zero-length vector
//    11 May  2009, A.Bagulya : added new implementation of methods 
//            ComputeSecondDerivatives - first derivatives at edge points 
//                                       should be provided by a user
//            FillSecondDerivatives - default computation base on "not-a-knot"
//                                    algorithm
//    19 Jun. 2009, V.Ivanchenko : removed hidden bin 
//    17 Nov. 2009, H.Kurashige   : use pointer for DataVector
//    04 May  2010  H.Kurashige   : use G4PhyscisVectorCache
//    28 May  2010  H.Kurashige  : Stop using  pointers to G4PVDataVector
// --------------------------------------------------------------

#include "G4PhysicsVector.hh"
#include <iomanip>

// --------------------------------------------------------------

G4PhysicsVector::G4PhysicsVector(G4bool spline)
 : type(T_G4PhysicsVector),
   edgeMin(0.), edgeMax(0.), numberOfNodes(0),
   useSpline(spline)
{
  cache      = new G4PhysicsVectorCache();
}

// --------------------------------------------------------------

G4PhysicsVector::~G4PhysicsVector() 
{
  delete cache; cache =0;
}

// --------------------------------------------------------------

G4PhysicsVector::G4PhysicsVector(const G4PhysicsVector& right)
{
  cache      = new G4PhysicsVectorCache();
  CopyData(right);
}

// --------------------------------------------------------------

G4PhysicsVector& G4PhysicsVector::operator=(const G4PhysicsVector& right)
{
  if (&right==this)  { return *this; }
  CopyData(right);
  return *this;
}

// --------------------------------------------------------------

G4int G4PhysicsVector::operator==(const G4PhysicsVector &right) const
{
  return (this == &right);
}

// --------------------------------------------------------------

G4int G4PhysicsVector::operator!=(const G4PhysicsVector &right) const
{
  return (this != &right);
}

// --------------------------------------------------------------

void G4PhysicsVector::DeleteData()
{
  secDerivative.clear();
}

// --------------------------------------------------------------

void G4PhysicsVector::CopyData(const G4PhysicsVector& vec)
{
  type = vec.type;
  edgeMin = vec.edgeMin;
  edgeMax = vec.edgeMax;
  numberOfNodes = vec.numberOfNodes;
  cache->lastEnergy = vec.GetLastEnergy();
  cache->lastValue = vec.GetLastValue();
  cache->lastBin = vec.GetLastBin();
  useSpline = vec.useSpline;
  comment = vec.comment;

  size_t i;
  dataVector.clear();
  for(i=0; i<(vec.dataVector).size(); i++){ 
    dataVector.push_back( (vec.dataVector)[i] );
  }
  binVector.clear();
  for(i=0; i<(vec.binVector).size(); i++){ 
    binVector.push_back( (vec.binVector)[i] );
  }
  secDerivative.clear();
  for(i=0; i<(vec.secDerivative).size(); i++){ 
    secDerivative.push_back( (vec.secDerivative)[i] );
  }
}

// --------------------------------------------------------------

G4double G4PhysicsVector::GetLowEdgeEnergy(size_t binNumber) const
{
  return binVector[binNumber];
}

// --------------------------------------------------------------

G4bool G4PhysicsVector::Store(std::ofstream& fOut, G4bool ascii)
{
  // Ascii mode
  if (ascii)
  {
    fOut << *this;
    return true;
  } 
  // Binary Mode

  // binning
  fOut.write((char*)(&edgeMin), sizeof edgeMin);
  fOut.write((char*)(&edgeMax), sizeof edgeMax);
  fOut.write((char*)(&numberOfNodes), sizeof numberOfNodes);

  // contents
  size_t size = dataVector.size(); 
  fOut.write((char*)(&size), sizeof size);

  G4double* value = new G4double[2*size];
  for(size_t i = 0; i < size; ++i)
  {
    value[2*i]  =  binVector[i];
    value[2*i+1]=  dataVector[i];
  }
  fOut.write((char*)(value), 2*size*(sizeof (G4double)));
  delete [] value;

  return true;
}

// --------------------------------------------------------------

G4bool G4PhysicsVector::Retrieve(std::ifstream& fIn, G4bool ascii)
{
  // clear properties;
  cache->lastEnergy=-DBL_MAX;
  cache->lastValue =0.;
  cache->lastBin   =0;
  dataVector.clear();
  binVector.clear();
  secDerivative.clear();
  comment = "";

  // retrieve in ascii mode
  if (ascii)
  {
    // binning
    fIn >> edgeMin >> edgeMax >> numberOfNodes; 
    if (fIn.fail())  { return false; }
    // contents
    G4int siz=0;
    fIn >> siz;
    if (fIn.fail())  { return false; }
    if (siz<=0)
    {
#ifdef G4VERBOSE  
      G4cerr << "G4PhysicsVector::Retrieve():";
      G4cerr << " Invalid vector size: " << siz << G4endl;
#endif
      return false;
    }

    binVector.reserve(siz);
    dataVector.reserve(siz);
    G4double vBin, vData;

    for(G4int i = 0; i < siz ; i++)
    {
      vBin = 0.;
      vData= 0.;
      fIn >> vBin >> vData;
      if (fIn.fail())  { return false; }
      binVector.push_back(vBin);
      dataVector.push_back(vData);
    }
    return true ;
  }

  // retrieve in binary mode
  // binning
  fIn.read((char*)(&edgeMin), sizeof edgeMin);
  fIn.read((char*)(&edgeMax), sizeof edgeMax);
  fIn.read((char*)(&numberOfNodes), sizeof numberOfNodes ); 
 
  // contents
  size_t size;
  fIn.read((char*)(&size), sizeof size); 
 
  G4double* value = new G4double[2*size];
  fIn.read((char*)(value),  2*size*(sizeof(G4double)) );
  if (G4int(fIn.gcount()) != G4int(2*size*(sizeof(G4double))) )
  {
    delete [] value;
    return false;
  }

  binVector.reserve(size);
  dataVector.reserve(size);
  for(size_t i = 0; i < size; ++i)
  {
    binVector.push_back(value[2*i]);
    dataVector.push_back(value[2*i+1]);
  }
  delete [] value;
  return true;
}

// --------------------------------------------------------------

void 
G4PhysicsVector::ScaleVector(G4double factorE, G4double factorV)
{
  size_t n = dataVector.size();
  size_t i;
  if(n > 0) { 
    for(i=0; i<n; ++i) {
      binVector[i]  *= factorE;
      dataVector[i] *= factorV;
    } 
  }
  //  n = secDerivative.size();
  // if(n > 0) { for(i=0; i<n; ++i) { secDerivative[i] *= factorV; } }
  secDerivative.clear();

  edgeMin *= factorE;
  edgeMax *= factorE;
  cache->lastEnergy = factorE*(cache->lastEnergy);
  cache->lastValue  = factorV*(cache->lastValue);
}

// --------------------------------------------------------------

void 
G4PhysicsVector::ComputeSecondDerivatives(G4double firstPointDerivative, 
                                          G4double endPointDerivative)
  //  A standard method of computation of second derivatives 
  //  First derivatives at the first and the last point should be provided
  //  See for example W.H. Press et al. "Numerical reciptes and C"
  //  Cambridge University Press, 1997.
{
  if(4 > numberOfNodes)   // cannot compute derivatives for less than 4 bins
  {
    ComputeSecDerivatives();
    return;
  }

  if(!SplinePossible()) { return; }

  G4int n = numberOfNodes-1;

  G4double* u = new G4double [n];
  
  G4double p, sig, un;

  u[0] = (6.0/(binVector[1]-binVector[0]))
    * ((dataVector[1]-dataVector[0])/(binVector[1]-binVector[0])
       - firstPointDerivative);
 
  secDerivative[0] = - 0.5;

  // Decomposition loop for tridiagonal algorithm. secDerivative[i]
  // and u[i] are used for temporary storage of the decomposed factors.

  for(G4int i=1; i<n; ++i)
  {
    sig = (binVector[i]-binVector[i-1]) / (binVector[i+1]-binVector[i-1]);
    p = sig*(secDerivative[i-1]) + 2.0;
    secDerivative[i] = (sig - 1.0)/p;
    u[i] = (dataVector[i+1]-dataVector[i])/(binVector[i+1]-binVector[i])
         - (dataVector[i]-dataVector[i-1])/(binVector[i]-binVector[i-1]);
    u[i] = 6.0*u[i]/(binVector[i+1]-binVector[i-1]) - sig*u[i-1]/p;
  }

  sig = (binVector[n-1]-binVector[n-2]) / (binVector[n]-binVector[n-2]);
  p = sig*secDerivative[n-2] + 2.0;
  un = (6.0/(binVector[n]-binVector[n-1]))
    *(endPointDerivative - 
      (dataVector[n]-dataVector[n-1])/(binVector[n]-binVector[n-1])) - u[n-1]/p;
  secDerivative[n] = un/(secDerivative[n-1] + 2.0);

  // The back-substitution loop for the triagonal algorithm of solving
  // a linear system of equations.
   
  for(G4int k=n-1; k>0; --k)
  {
    secDerivative[k] *= 
      (secDerivative[k+1] - 
       u[k]*(binVector[k+1]-binVector[k-1])/(binVector[k+1]-binVector[k]));
  }
  secDerivative[0] = 0.5*(u[0] - secDerivative[1]);

  delete [] u;
}

// --------------------------------------------------------------

void G4PhysicsVector::FillSecondDerivatives()
  // Computation of second derivatives using "Not-a-knot" endpoint conditions
  // B.I. Kvasov "Methods of shape-preserving spline approximation"
  // World Scientific, 2000
{  
  if(5 > numberOfNodes)  // cannot compute derivatives for less than 4 points
  {
    ComputeSecDerivatives();
    return;
  }

  if(!SplinePossible()) { return; }
 
  G4int n = numberOfNodes-1;

  //G4cout << "G4PhysicsVector::FillSecondDerivatives() n= " << n 
  //	 << "   " << this << G4endl;
  // G4cout << *this << G4endl;

  G4double* u = new G4double [n];
  
  G4double p, sig;

  u[1] = ((dataVector[2]-dataVector[1])/(binVector[2]-binVector[1]) -
          (dataVector[1]-dataVector[0])/(binVector[1]-binVector[0]));
  u[1] = 6.0*u[1]*(binVector[2]-binVector[1])
    / ((binVector[2]-binVector[0])*(binVector[2]-binVector[0]));
 
  // Decomposition loop for tridiagonal algorithm. secDerivative[i]
  // and u[i] are used for temporary storage of the decomposed factors.

  secDerivative[1] = (2.0*binVector[1]-binVector[0]-binVector[2])
    / (2.0*binVector[2]-binVector[0]-binVector[1]);

  for(G4int i=2; i<n-1; ++i)
  {
    sig = (binVector[i]-binVector[i-1]) / (binVector[i+1]-binVector[i-1]);
    p = sig*secDerivative[i-1] + 2.0;
    secDerivative[i] = (sig - 1.0)/p;
    u[i] = (dataVector[i+1]-dataVector[i])/(binVector[i+1]-binVector[i])
         - (dataVector[i]-dataVector[i-1])/(binVector[i]-binVector[i-1]);
    u[i] = (6.0*u[i]/(binVector[i+1]-binVector[i-1])) - sig*u[i-1]/p;
  }

  sig = (binVector[n-1]-binVector[n-2]) / (binVector[n]-binVector[n-2]);
  p = sig*secDerivative[n-3] + 2.0;
  u[n-1] = (dataVector[n]-dataVector[n-1])/(binVector[n]-binVector[n-1])
    - (dataVector[n-1]-dataVector[n-2])/(binVector[n-1]-binVector[n-2]);
  u[n-1] = 6.0*sig*u[n-1]/(binVector[n]-binVector[n-2])
    - (2.0*sig - 1.0)*u[n-2]/p;  

  p = (1.0+sig) + (2.0*sig-1.0)*secDerivative[n-2];
  secDerivative[n-1] = u[n-1]/p;

  // The back-substitution loop for the triagonal algorithm of solving
  // a linear system of equations.
   
  for(G4int k=n-2; k>1; --k)
  {
    secDerivative[k] *= 
      (secDerivative[k+1] - 
       u[k]*(binVector[k+1]-binVector[k-1])/(binVector[k+1]-binVector[k]));
  }
  secDerivative[n] = (secDerivative[n-1] - (1.0-sig)*secDerivative[n-2])/sig;
  sig = 1.0 - ((binVector[2]-binVector[1])/(binVector[2]-binVector[0]));
  secDerivative[1] *= (secDerivative[2] - u[1]/(1.0-sig));
  secDerivative[0]  = (secDerivative[1] - sig*secDerivative[2])/(1.0-sig);

  delete [] u;
}

// --------------------------------------------------------------

void 
G4PhysicsVector::ComputeSecDerivatives()
  //  A simplified method of computation of second derivatives 
{
  if(!SplinePossible())  { return; }

  if(3 > numberOfNodes)  // cannot compute derivatives for less than 4 bins
  {
    useSpline = false;
    return;
  }

  size_t n = numberOfNodes-1;

  for(size_t i=1; i<n; ++i)
  {
    secDerivative[i] =
      3.0*((dataVector[i+1]-dataVector[i])/(binVector[i+1]-binVector[i]) -
           (dataVector[i]-dataVector[i-1])/(binVector[i]-binVector[i-1]))
      /(binVector[i+1]-binVector[i-1]);
  }
  secDerivative[n] = secDerivative[n-1];
  secDerivative[0] = secDerivative[1];
}

// --------------------------------------------------------------

G4bool G4PhysicsVector::SplinePossible()
  // Initialise second derivative array. If neighbor energy coincide 
  // or not ordered than spline cannot be applied
{
  secDerivative.clear();
  if(!useSpline)  { return useSpline; }
  secDerivative.reserve(numberOfNodes);
  for(size_t j=0; j<numberOfNodes; ++j)
  {
    secDerivative.push_back(0.0);
    if(j > 0)
    {
      if(binVector[j]-binVector[j-1] <= 0.)  { useSpline = false; }
    }
  }  
  return useSpline;
}
   
// --------------------------------------------------------------

std::ostream& operator<<(std::ostream& out, const G4PhysicsVector& pv)
{
  // binning
  out << std::setprecision(12) << pv.edgeMin << " "
      << pv.edgeMax << " " << pv.numberOfNodes << G4endl; 

  // contents
  out << pv.dataVector.size() << G4endl; 
  for(size_t i = 0; i < pv.dataVector.size(); i++)
  {
    out << pv.binVector[i] << "  " << pv.dataVector[i] << G4endl;
  }
  out << std::setprecision(6);

  return out;
}

//---------------------------------------------------------------

G4double G4PhysicsVector::Value(G4double theEnergy) 
{
  // Use cache for speed up - check if the value 'theEnergy' is same as the 
  // last call. If it is same, then use the last bin location. Also the
  // value 'theEnergy' lies between the last energy and low edge of of the 
  // bin of last call, then the last bin location is used.

  if( theEnergy == cache->lastEnergy ) {

  } else if( theEnergy < cache->lastEnergy
	&&   theEnergy >= binVector[cache->lastBin]) {
     cache->lastEnergy = theEnergy;
     Interpolation(cache->lastBin);

  } else if( theEnergy <= edgeMin ) {
     cache->lastBin = 0;
     cache->lastEnergy = edgeMin;
     cache->lastValue  = dataVector[0];

  } else if( theEnergy >= edgeMax ){
     cache->lastBin = numberOfNodes-1;
     cache->lastEnergy = edgeMax;
     cache->lastValue  = dataVector[cache->lastBin];

  } else {
     cache->lastBin = FindBinLocation(theEnergy); 
     cache->lastEnergy = theEnergy;
     Interpolation(cache->lastBin);
  }
  return cache->lastValue;        
}

