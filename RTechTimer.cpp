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

#include "RTechTimer.hpp"
#include <sys/time.h>


#ifndef  HZ
#define HZ CLK_TCK
#endif


RTechTimer::RTechTimer()
{
 started=stopped=0;
 accumulator_wall=accumulator_cpu=0.0;

}


void RTechTimer::Reset()
{ accumulator_wall=accumulator_cpu=0.0;
}

void RTechTimer::Restart()
{ accumulator_wall=accumulator_cpu=0.0;
  Start();
}

void RTechTimer::Start()
{ struct tms cpu_t_start;
  struct timeval wall_t_start;

  times(&cpu_t_start);
  cpu_start=((double)cpu_t_start.tms_utime/(double)HZ+(double)cpu_t_start.tms_stime/(double)HZ);

  gettimeofday(&wall_t_start, (struct timezone*)NULL);
  wall_start=(double)wall_t_start.tv_sec*1000000. + (double)wall_t_start.tv_usec;

  started=1; stopped=0;
}

void RTechTimer::Stop()
{ struct tms cpu_t_end;
  struct timeval wall_t_end;

  times(&cpu_t_end);
  cpu_end=((double)cpu_t_end.tms_utime/HZ+(double)cpu_t_end.tms_stime/HZ);
  accumulator_cpu+=cpu_end-cpu_start;

  gettimeofday(&wall_t_end, (struct timezone*)NULL);
  wall_end=double(wall_t_end.tv_sec*1000000. + wall_t_end.tv_usec);
  accumulator_wall+=wall_end-wall_start;

  started=0; stopped=1;
}

double RTechTimer::CPU_microsec()
{ return 1000000.*CPU_sec();
}

double RTechTimer::CPU_sec()
{ 
  if (stopped) return accumulator_cpu;
  if (!started)
  { printf("Invalid call to CPU_msec\n");
    return 0;
  }
  struct tms cpu_t_now;
  times(&cpu_t_now);
  double now=((double)cpu_t_now.tms_utime/HZ+(double)cpu_t_now.tms_stime/HZ);
  return accumulator_cpu+ (now-cpu_start);
}

double RTechTimer::WALL_microsec()
{
  if (stopped) return accumulator_wall;
  if (!started)
  { printf("Invalid call to CPU_msec\n");
    return 0;
  }

  struct timeval wall_t_now;
  gettimeofday(&wall_t_now, (struct timezone*)NULL);
  double now=(double)(wall_t_now.tv_sec*1000000. + wall_t_now.tv_usec);
  return accumulator_wall + (now-wall_start);
}
