#include "PeriodicStats.hpp"
#include <math.h>
#include <cstdlib>


PeriodicStats::PeriodicStats()
{ count=0;
  dx=dy=dz=0.0;
  l=m=n=0.0;
  dxrot=dyrot=dzrot=0;

}


void PeriodicStats::AddPoint(Node *n1, Node *n2)
{ 
  // obtain two nodes which are supposed to be rotational periodic
  double dxl=fabs(n2->x-n1->x);
  double dyl=fabs(n2->y-n1->y);
  double dzl=fabs(n2->z-n1->z);
  dx+=dxl;
  dy+=dyl;
  dz+=dzl;
  
  if (n1==n2)
  { n1->y=n2->y=0;
    n1->z=n2->z=0;
  }
  //if (dxl+dyl+dzl<1e-6) return;
  printf("ANGLE %lg , DIST %lg %lg %lg\n", atan2(n1->z,n1->y)*180./M_PI-atan2(n2->z,n2->y)*180./M_PI, fabs(n2->x-n1->x), fabs(n2->y-n1->y), fabs(n2->z-n1->z));

  l+=fabs(atan2(n1->z,n1->y)-atan2(n2->z,n2->y));
  m+=fabs(atan2(n1->z,n1->x)-atan2(n2->z,n2->x));
  n+=fabs(atan2(n1->y,n1->x)-atan2(n2->y,n2->x));

  double periodicAngle = 360/11.;
  double theta = -(atan2(n1->z,n1->y)>atan2(n2->z,n2->y)?-periodicAngle*M_PI/180.:periodicAngle*M_PI/180.);
  //double x2=n1->x*cos(theta) - n1->z*sin(theta);
  double z2=n1->z*cos(theta) - n1->y*sin(theta);
  double y2=n1->z*sin(theta) + n1->y*cos(theta);

  double m=fabs(atan2(n1->z,n1->y)-atan2(z2,y2));

  //printf("Points x:%lg %lg, y:%lg,%lg,  z: %lg %lg,  m=%lg\n", n2->y, y2, n1->y, n2->y, n2->z, z2, m*180/M_PI);
  //dxrot+=fabs(n2->x- x2);
  dyrot+=fabs(n2->y- y2);
  dzrot+=fabs(n2->z- z2);

  n2->z=n1->z*cos(theta) - n1->y*sin(theta);
  n2->y=n1->z*sin(theta) + n1->y*cos(theta);


  if (fabs(n2->x-n1->x)+ fabs(n2->y-y2)+ fabs(n2->z-z2)>0.001)
  { printf("Error; point difference too large after rotation\n");
    printf("deltaX= %lg, deltaY= %lg, deltaZ=%lg\n", fabs(n2->x-n1->x), fabs(n2->y-y2), fabs(n2->z-z2));
    exit(1);
  }
  count ++;
}


double PeriodicStats::MeanDX()
{ if (count==0) return 0;
  return dx/count;
}

double PeriodicStats::MeanDY()
{ if (count==0) return 0;
  return dy/count;
}

double PeriodicStats::MeanDZ()
{ if (count==0) return 0;
  return dz/count;
}

double PeriodicStats::MeanDXrot()
{ if (count==0) return 0;
  return dxrot/count;
}

double PeriodicStats::MeanDYrot()
{ if (count==0) return 0;
  return dyrot/count;
}

double PeriodicStats::MeanDZrot()
{ if (count==0) return 0;
  return dzrot/count;
}

double PeriodicStats::MeanL()
{ if (count==0) return 0;
  return l/count;
}

double PeriodicStats::MeanM()
{ if (count==0) return 0;
  return m/count;
}

double PeriodicStats::MeanN()
{ if (count==0) return 0;
  return n/count;
}

void PeriodicStats::Print()
{
  printf("dx=%1.16lg\n", MeanDX());
  printf("dy=%1.16lg\n", MeanDY());
  printf("dz=%1.16lg\n", MeanDZ());
  printf("dl=%1.16lg\n", MeanL()*180./M_PI);
  printf("dm=%1.16lg\n", MeanM()*180./M_PI);
  printf("dn=%1.16lg\n", MeanN()*180./M_PI);
  printf("dxrot=%1.16lg\n", MeanDXrot());
  printf("dyrot=%1.16lg\n", MeanDYrot());
  printf("dzrot=%1.16lg\n", MeanDZrot());
}
