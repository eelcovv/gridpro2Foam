/* This file belongs to the GridPro2FOAM distribution
   Developed by Vincent Rivola and Martin Spel
   R.Tech SARL
   Parc Technologique Cap Delta
   09340 Verniolle
   France
   For contact information: http://www.rtech.fr/contact.html
   or email: support@rtech-engineering.com
*/

#include <list>
using namespace std;


main()
{ list<int> ll;
  ll.push_back(2);
  ll.push_back(3);
  ll.push_back(6);
  ll.push_back(8);
  ll.push_back(10);
  list<int>::iterator i,j;
  for (i=ll.begin(); i!=ll.end(); i++)
  { printf("%d\n", *i);
    j=i; j++;
    for (; j!=ll.end(); j++)
    { if (*j==8 && *i==6)
      { ll.erase(j);
        break;
      }
    }
  }
}
