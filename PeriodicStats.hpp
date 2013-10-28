#ifndef PERIODIC_STATS
#define PERIODIC_STATS

#include "Node.hpp"

class PeriodicStats
{
  private:
    int count;
    double dx,dy,dz;
    double l,m,n;
    double dxrot, dyrot, dzrot;

  public:
    PeriodicStats();
    void AddPoint(Node *n1, Node *n2);
    void Print();
    double MeanDX();
    double MeanDY();
    double MeanDZ();
    double MeanL();
    double MeanM();
    double MeanN();
    double MeanDXrot();
    double MeanDYrot();
    double MeanDZrot();
};

#endif
