/* This file belongs to the GridPro2FOAM distribution
   Developed by Vincent Rivola and Martin Spel
   R.Tech SARL
   Parc Technologique Cap Delta
   09340 Verniolle
   France
   For contact information: http://www.rtech.fr/contact.html
   or email: support@rtech-engineering.com
*/


#ifndef TRIPLET_HPP
#define TRIPLET_HPP

#include <string>
using namespace std;

class Triplet
{ friend class Patch;
  public:
    int i, j, k;
    void Set(int setI,int setJ,int setK);
  
};

#endif
