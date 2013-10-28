/* This file belongs to the GridPro2FOAM distribution
   Developed by Vincent Rivola and Martin Spel
   R.Tech SARL
   Parc Technologique Cap Delta
   09340 Verniolle
   France
   For contact information: http://www.rtech.fr/contact.html
   or email: support@rtech-engineering.com
*/


#ifndef FACE_HPP
#define FACE_HPP
#include "Node.hpp"
#include <string>
#include <math.h>
#include <stdio.h>

class Face
{ friend class Node;
  private:
    Node *n0, *n1, *n2, *n3;
    int idFace;
    int owner, neighbor;
    int BC;
    int internalBC;
    int BCorg; // only for debug
    int sumId;
    class Face* matchingFace;

    // for periodic BCs
    bool isPeriodic;
    int sumPeriodicId;
    int idPeriodicFace;
    short periodicSide;
  public:
    
    Face(Node *n0, Node *n1, Node *n2, Node *n3, int setOwner, int setNeighbor, int setBC);
    ~Face();
    void Print(FILE *f = stdout);
    void PrintDebug(FILE *f = stdout);
    void AnalysePeriodic(PeriodicStats *stats, int BC);
    void WriteTecplot(FILE *f = stdout);
    void SetIdFace(int setIdFace);
    int GetIdFace() {return idFace;};
    int GetOwner();
    int GetNeighbor();
    void SetOwner(int setOwner);
    void SetNeighbor(int setNeighbor);
    bool MergeNeeded(Face *other);
    int SumId();
    void ComputeSumPeriodicId();
    bool PeriodicMatch(Face *x);
    int SumPeriodicId();
    int GetBCorg() {return BCorg;};
    int GetBC() {return BC;};

    void SetPeriodic() { isPeriodic=true;};
    bool IsInternal() { return BC==-1;};
    bool IsPeriodic() { return isPeriodic;};
    void SetMatchingFace(Face *face) { matchingFace=face;};
    void SetPeriodicFaceId(int x) { idPeriodicFace=x;};
    int GetPeriodicFaceId() { return idPeriodicFace;};
    int GetPeriodicSide() { return periodicSide;};
};
#endif
