// -*- C++ -*-

#ifndef _BENCH_GETTIME_INCLUDED
#define _BENCH_GETTIME_INCLUDED

#include <stdlib.h>
#include <sys/time.h>
#include <iomanip>
#include <iostream>

struct timer {
  float totalTime;
  float lastTime;
  float totalWeight;
  bool on;
  struct timezone tzp;
  timer() {
    struct timezone tz = {0, 0};
    totalTime=0.0; 
    totalWeight=0.0;
    on=0; tzp = tz;}
  float getTime() {
    timeval now;
    gettimeofday(&now, &tzp);
    return ((float) now.tv_sec) + ((float) now.tv_usec)/1000000.;
  }
  void start () {
    on = 1;
    lastTime = getTime();
  } 
  float stop () {
    on = 0;
    float d = (getTime()-lastTime);
    totalTime += d;
    return d;
  } 
  float stop (float weight) {
    on = 0;
    totalWeight += weight;
    float d = (getTime()-lastTime);
    totalTime += weight*d;
    return d;
  } 

  float total() {
    if (on) return totalTime + getTime() - lastTime;
    else return totalTime;
  }

  float next() {
    if (!on) return 0.0;
    float t = getTime();
    float td = t - lastTime;
    totalTime += td;
    lastTime = t;
    return td;
  }

  void reportT(float time) {
    std::cout << "PBBS-time: " << std::setprecision(3) << time <<  std::endl;;
  }

  void reportTime(float time) {
    reportT(time);
  }

  void reportStop(float weight, std::string str) {
    std::cout << str << " :" << weight << ": ";
    reportTime(stop(weight));
  }

  void reportTotal() {
    float to = (totalWeight > 0.0) ? total()/totalWeight : total();
    reportTime(to);
    totalTime = 0.0;
    totalWeight = 0.0;
  }

  void reportTotal(std::string str) {
    std::cout << str << " : "; 
    reportTotal();}

  void reportNext() {reportTime(next());}

  void reportNext(std::string str) {std::cout << str << " : "; reportNext();}
};

static timer _tm;
#define timeStatement(_A,_string) _tm.start();  _A; _tm.reportNext(_string);
#define startTime() _tm.start();
#define stopTime(_weight,_str) _tm.reportStop(_weight,_str);
#define reportTime(_str) _tm.reportTotal(_str);
#define nextTime(_string) _tm.reportNext(_string);
#define nextTimeN() _tm.reportT(_tm.next());

#endif // _BENCH_GETTIME_INCLUDED

