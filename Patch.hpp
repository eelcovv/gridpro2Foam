/* This file belongs to the GridPro2FOAM distribution
   Developed by Vincent Rivola and Martin Spel
   R.Tech SARL
   Parc Technologique Cap Delta
   09340 Verniolle
   France
   For contact information: http://www.rtech.fr/contact.html
   or email: support@rtech-engineering.com
*/


#ifndef PATCH_HPP
#define PATCH_HPP

#include <string>
#include <stdio.h>
#include "Triplet.hpp"
#include "Block.hpp"

using namespace std;

#define IDIR 0
#define JDIR 1
#define KDIR 2
class Block;
class Patch
{ 
  private:
    Block *hostBlock; // number of host block
    int hostFace; // face of host block
    Block *donorBlock; // number of host block
    int donorFace; // face of host block
    string orientation; // mapping of host ijk to donor ijk
    bool isPeriodic;    // true for periodic boundary condition
    short periodicSide;
    int A[3][3];        // transformation matrix
    int BC;
    Triplet lowHost, highHost;   // two i,j,k triplets to define the region of this patch
    Triplet lowDonor, highDonor;  // two i,j,k triplets to define the region of this patch
    int nrFaces;
    
    
  public:
    Patch(int setBC, Block *setHostBlock, int setSF1, Block *setDonorBlock, string setOrientation, bool setPeriodic=false, short setPeriodicSide=0);
    ~Patch();
    void MakeEdgePointsUnique(); // Makes the points of an interface between two blocks unique
    // if a patch has BC 0 we assume it is an internal BC
    bool IsInternal() {return BC==0;};
    bool IsPeriodic() {return isPeriodic;};
    int GetBC() {return BC;};
    bool Contains(int direction, int i, int j, int k);
    int NrFaces() {return nrFaces;};
};

#endif
