/* This file belongs to the GridPro2FOAM distribution
   Developed by Vincent Rivola and Martin Spel
   R.Tech SARL
   Parc Technologique Cap Delta
   09340 Verniolle
   France
   For contact information: http://www.rtech.fr/contact.html
   or email: support@rtech-engineering.com
*/


#include "Face.hpp"

#include <list>
#include <vector>
#include <utility>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;


Face::Face(Node *setN0,Node *setN1,Node *setN2,Node *setN3, int setOwner, int setNeighbor, int setBC)
{ 
  n0=setN0;
  n1=setN1;
  n2=setN2;
  n3=setN3;
  owner=setOwner;
  neighbor=setNeighbor;
  BC=setBC;
  BCorg=setBC;
  // sumId only used to check if the sum of the four nodes is identical,
  // to see faster if the face can be merged or not (4 identical nodes)
  sumId=n0->GetId()+n1->GetId()+n2->GetId()+n3->GetId();

  isPeriodic=false;
  idPeriodicFace=-1;
  periodicSide=0;
}

Face::~Face()
{
}

void Face::Print(FILE *f)
{ 
 fprintf(f, "4(%d %d %d %d)\n", n0->GetId(), n1->GetId(), n2->GetId(), n3->GetId());
}

void Face::AnalysePeriodic(PeriodicStats *stats, int BC=0)
{ if (idFace==57801 || idFace==57817) printf("DEBUG %d\n", idFace);
 n0->AnalysePeriodic(stats, BC); n1->AnalysePeriodic(stats,BC); n2->AnalysePeriodic(stats,BC); n3->AnalysePeriodic(stats,BC);
}

void Face::PrintDebug(FILE *f)
{ 
 n0->PrintDebug(f); n1->PrintDebug(f); n2->PrintDebug(f); n3->PrintDebug(f);
}

void Face::SetIdFace(int setIdFace)
{
 idFace=setIdFace;
}

int Face::GetOwner()
{
 return owner;
}

int Face::GetNeighbor()
{
 return neighbor;
}

void Face::SetOwner(int setOwner)
{
 owner=setOwner;
}

void Face::SetNeighbor(int setNeighbor)
{
 neighbor=setNeighbor;
}

void Face::ComputeSumPeriodicId()
{ sumPeriodicId=n0->GetPeriodicId(BC)+n1->GetPeriodicId(BC)+n2->GetPeriodicId(BC)+n3->GetPeriodicId(BC);
  periodicSide=n0->GetPeriodicSide(BC)+n1->GetPeriodicSide(BC)+n2->GetPeriodicSide(BC)+n3->GetPeriodicSide(BC);
}

bool Face::PeriodicMatch(Face *x)
{ list<int> A, B;
  A.push_back(x->n0->GetPeriodicId(BC));
  A.push_back(x->n1->GetPeriodicId(BC));
  A.push_back(x->n2->GetPeriodicId(BC));
  A.push_back(x->n3->GetPeriodicId(BC));
  B.push_back(n0->GetId());
  B.push_back(n1->GetId());
  B.push_back(n2->GetId());
  B.push_back(n3->GetId());
  A.sort(); B.sort();
  return A==B;
}

int Face::SumPeriodicId()
{ // used to compare faster if two faces contain the same peridic ids
  return sumPeriodicId;
}

int Face::SumId()
{ // used to compare faster if two faces contain the same ids (before checking the permutations)
  return sumId;
}

bool Face::MergeNeeded(Face *other)
{ if (owner!=-1 && neighbor!=-1) return false;
  if (other->owner!=-1 && other->neighbor!=-1) return false;
  if (SumId()!=other->SumId()) return false;
#ifdef DEBUG
  //printf ("\033Mpossible candidate\n");fflush(stdout);
  //Print();
  //other->Print();
#endif

  list<int> A;
  A.push_back(n0->GetId());
  A.push_back(n1->GetId());
  A.push_back(n2->GetId());
  A.push_back(n3->GetId());
  list<int> B;
  B.push_back(other->n0->GetId());
  B.push_back(other->n1->GetId());
  B.push_back(other->n2->GetId());
  B.push_back(other->n3->GetId());
  
  A.sort();
  B.sort();
  
  if (A==B) 
  { 
    //printf ("\033Mmerge needed\n");fflush(stdout);
    //Print();
    //other->Print();fflush(stdout);
    return true;
  }
  return false;
}


void Face::WriteTecplot(FILE *f)
{ fprintf(f, "ZONE I=2 J=2\n");
  n0->PrintTecplot(f);
  n3->PrintTecplot(f);
  n1->PrintTecplot(f);
  n2->PrintTecplot(f);
}


