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

#include <utility>
#include <map>
#include <string>
using namespace std;

main()
{

  pair<int, string> a(1, "test 1");
  pair<int, string> b(2, "test 2");
  pair<int, string> c(1, "test 3");

  map<int, string> myMap;

  myMap.insert(a);
  myMap.insert(b);
  myMap.insert(c);


  printf("Count 1: %d\n", myMap.count(1));
  printf("Count 2: %d\n", myMap.count(2));
  printf("Count 3: %d\n", myMap.count(3));

  map<int, string>::iterator it = myMap.find(1);
  printf("Object with int=1: %s\n", (*it).second.c_str());
}
