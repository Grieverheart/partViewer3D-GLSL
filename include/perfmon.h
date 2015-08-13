#ifndef PERFMON_H
#define PERFMON_H

#define MAX_PERF_BLOCKS 128
#define NS_IN_SEC 1E9

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
    PerfMon(void):
        curr_frame_(0),
        current_depth_(0),
        curr_wpos_(0), curr_rpos_(0)
    {
#ifdef _WIN32
        LARGE_INTEGER li;
        QueryPerformanceFrequency(&li);
        cpu_freq = li.QuadPart; //ns
#endif
    }

    ~PerfMon(void){};

    void push_query(const char* name){
        blocks_[curr_wpos_] = PerfBlock(get_time_ns(), name, current_depth_++);
        curr_wpos_ = (curr_wpos_ + 1 == MAX_PERF_BLOCKS)? 0: curr_wpos_ + 1;
    }

    void pop_query(void){
        --current_depth_;
        size_t curr_upos_ = (curr_wpos_ > 0)? curr_wpos_ - 1: MAX_PERF_BLOCKS - 1;
        while(blocks_[curr_upos_].depth_ > current_depth_){
            curr_upos_ = (curr_upos_ > 0)? curr_upos_ - 1: MAX_PERF_BLOCKS - 1;
        }
        blocks_[curr_upos_].end_time_ = get_time_ns();
    }

    void sync(void){
        current_depth_ = 0;
        push_query("Frame");

        if(curr_frame_ > 0){
            PerfBlock& block = blocks_[curr_rpos_];
            printf("Frame %d time: %fms\n", curr_frame_, (get_time_ns() - block.start_time_) / 1.0e6);
            curr_rpos_ = (curr_rpos_ + 1 == MAX_PERF_BLOCKS)? 0: curr_rpos_ + 1;

            for(;
                blocks_[curr_rpos_].depth_ > 0; 
                curr_rpos_ = (curr_rpos_ + 1 == MAX_PERF_BLOCKS)? 0: curr_rpos_ + 1
            ){
                const PerfBlock& block = blocks_[curr_rpos_];
                printf("%.*s Block %s. Time: %fms\n", 4 * block.depth_, "-------------------------", block.name_, (block.end_time_ - block.start_time_) / 1.0e6);
            }
        }

        ++curr_frame_;
    }

    uint64_t get_time_ns(void)const{
#ifdef _WIN32
        LARGE_INTEGER li;
        QueryPerformanceCounter(&li);
        return li.QuadPart / cpu_freq;
#elif __linux
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
        return ts.tv_sec * NS_IN_SEC + ts.tv_nsec;
#endif
    }

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
