/* This file belongs to the GridPro2FOAM distribution
   Developed by Vincent Rivola and Martin Spel
   R.Tech SARL
   Parc Technologique Cap Delta
   09340 Verniolle
   France
   For contact information: http://www.rtech.fr/contact.html
   or email: support@rtech-engineering.com
*/


#ifndef NODE_HPP
#define NODE_HPP

#include <string>
#include <map>
#include <stdio.h>
using namespace std;
class PeriodicStats;

class Node 
{ friend class Face;
  friend class PeriodicStats;
  private:
    double x, y, z;
    int id;
    Node *equivalent;
    map<int, Node*> periodicNeighbor;
    bool isBoundaryNode;
    map <int, short> periodicSide;  // which side of the periodic surface: -1, 0 (matching) or 1
  public:
    Node(double x=0, double y=0, double z=0, bool setBoundaryNode=false);
    ~Node();
    void SetEquivalent(Node *n2, bool debug=false);
    void PrintEquivalent();
    void PrintDebug(FILE *f = stdout);
    void AnalysePeriodic(PeriodicStats *stats, int BC);
    Node* GetEquivalent() {return equivalent;};
    void Print(FILE *f = stdout);
    void PrintTecplot(FILE *f = stdout);
    double Distance(Node *n2);
    void SetId(int setId);
    int GetId();
    int GetRealId() {return id;};
    void SetPeriodicNeighbor(int BC, Node *n2);
    int GetPeriodicId(int BC);
    void SetPeriodicSide(int BC, short n);
    short GetPeriodicSide(int BC);
};

#include "PeriodicStats.hpp"
#endif

