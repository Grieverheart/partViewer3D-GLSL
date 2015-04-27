#ifndef PERFMON_H
#define PERFMON_H

#define MAX_PERF_BLOCKS 128

#ifdef _WIN32
#include <windows.h>
#elif __linux
#include <ctime>
#endif

#ifdef _MSC_VER
#define snprintf sprintf_s
#endif

#include <cstdint>
#include <cstdio>

class PerfMon{
public:
    PerfMon(void);
    ~PerfMon(void){};

    void push_query(const char* name);
    void pop_query(void);
    void sync(void);

    uint64_t get_time_ns(void)const;

private:
    struct PerfBlock{
        PerfBlock(uint64_t start_time, const char* name, unsigned char depth):
            start_time_(start_time), depth_(depth)
        {
            snprintf(name_, 128, "%s", name);
        }

        PerfBlock(void){}

        char name_[128];
        uint64_t start_time_;
        uint64_t end_time_;
        unsigned char depth_;
    };
#ifdef _WIN32
    uint64_t cpu_freq;
#endif

    unsigned int curr_frame_;
    unsigned char current_depth_;

    size_t curr_wpos_;
    size_t curr_rpos_;
    PerfBlock blocks_[MAX_PERF_BLOCKS];
};

#endif
