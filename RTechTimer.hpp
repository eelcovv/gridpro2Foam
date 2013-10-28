/* This file belongs to the GridPro2FOAM distribution
   Developed by Vincent Rivola and Martin Spel
   R.Tech SARL
   Parc Technologique Cap Delta
   09340 Verniolle
   France
   For contact information: http://www.rtech.fr/contact.html
   or email: support@rtech-engineering.com
*/


#ifndef TIMER_HPP
#define TIMER_HPP

#include <time.h>
#include <sys/times.h>
#include <sys/param.h>
#include <sys/time.h>

class RTechTimer
{ private:
    double cpu_start, cpu_end;
    double wall_start, wall_end;
    double accumulator_cpu, accumulator_wall;
    int started, stopped;
  public:
    RTechTimer();
    void Reset();
    void Restart();
    void Start();
    void Stop();
    double CPU_sec();
    double CPU_microsec();
    double WALL_microsec();
};
#endif
