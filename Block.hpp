/* This file belongs to the GridPro2FOAM distribution
   Developed by Vincent Rivola and Martin Spel
   R.Tech SARL
   Parc Technologique Cap Delta
   09340 Verniolle
   France
   For contact information: http://www.rtech.fr/contact.html
   or email: support@rtech-engineering.com
*/


#ifndef BLOCK_HPP
#define BLOCK_HPP

#include "Node.hpp"
#include "Triplet.hpp"
#include "Face.hpp"
#include "Patch.hpp"
#include <vector>
#include <list>

#define IMIN 0
#define IMAX 1
#define JMIN 2
#define JMAX 3
#define KMIN 4
#define KMAX 5

using namespace std;
class Patch;
class Block
{ 
  friend class Patch;
  private:
    int id;
    int I, J, K;
    int cellOffset; // offset of cells for current block
    vector <Node*> nodes;
    int BC;
    
  public:
    vector <Patch*> externalPatches;
    // boundaryCellSide contains a list of sides (0: IMIN, 1 IMAX etc) that have a boundaryCell condition
    vector<vector <int> > boundaryCellSide;
    Block(int setI, int setJ, int setK, int setId);
    ~Block();
    void ReadGridPro(FILE *f, int meshType, bool axisym, double scaling); // read coordinates from file f
    int Id() {return id;};     // block number
    Node *GetNode(int i, int j, int k); // return node @ (i,j,k)
    Node *GetNode(Triplet *t); // return node @ (t->i, t->j, t->k)
    void Equivalence(Node *setHostNode, Node *setDonorNode); // gives node pointer equivalence for edge points
    void PrintEquivalent();
    void CountPoints(int &count);
    void CountCells(int &count);
    void WritePoints(FILE * f);
    void CreateFaces(list <Face*> &faces);
    int MakeCellId(int i, int j, int k);
    void AddExternalPatch(Patch *patch);
    int FindPatch(int direction, int i, int j, int k);
    void SetBC(int setBC) {BC=setBC;};
    int GetBC() {return BC;};
    void PrintCellList(FILE *f);
    int NrCells();
    // boundary zones
    void AddBoundaryZone(int side, int nBZ);
    void PrintBoundaryCellList(FILE *f, int nBZ); 
    int NrBoundaryZoneCells(int nBZ);
    void CreateBoundaryZoneVector(int numberBZ);

};
#endif

