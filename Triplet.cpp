/* This file belongs to the GridPro2FOAM distribution
   Developed by Vincent Rivola and Martin Spel
   R.Tech SARL
   Parc Technologique Cap Delta
   09340 Verniolle
   France
   For contact information: http://www.rtech.fr/contact.html
   or email: support@rtech-engineering.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Triplet.hpp"

void Triplet::Set(int setI, int setJ, int setK)
{ 
  i=setI;
  j=setJ;
  k=setK;
}

