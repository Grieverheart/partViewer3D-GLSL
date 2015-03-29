#include <include/perfmon.h>

#define MS_IN_SEC 1.0E3
#define NS_IN_SEC 1.0E9
#define NS_IN_MS  1.0E6

PerfMon::PerfMon(void)
{
#ifdef _WIN32
    LARGE_INTEGER li;
    QueryPerformanceFrequency(&li)
    cpu_freq = li.QuadPart / MS_IN_SEC; //ns
#endif
}

double PerfMon::get_time(void)const{
#ifdef _WIN32
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return li.QuadPart / cpu_freq;
#elif __linux
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return ts.tv_sec * MS_IN_SEC + ts.tv_nsec / NS_IN_MS;
#endif
}

void PerfMon::push_query(void){
    queries_.push(get_time());
}

double PerfMon::pop_query(void){
    double res = get_time() - queries_.top();
    queries_.pop();
    return res;
}
