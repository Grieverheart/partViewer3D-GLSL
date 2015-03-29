#ifndef PERFMON_H
#define PERFMON_H

#ifdef _WIN32
#include <windows.h>
#elif __linux
#include <ctime>
#endif

#include <stack>

class PerfMon{
public:
    PerfMon(void);
    ~PerfMon(void){};

    void push_query(void);
    double pop_query(void);
private:
    double get_time(void)const;
#ifdef _WIN32
    double cpu_freq;
#endif

    std::stack<double> queries_;
};

#endif
