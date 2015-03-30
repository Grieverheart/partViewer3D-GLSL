#include <include/perfmon.h>
#include <cstdio>

#define NS_IN_SEC 1E9

PerfMon::PerfMon(void):
    curr_frame_(0),
    current_depth_(0),
    curr_wpos_(0), curr_rpos_(0)
{
#ifdef _WIN32
    LARGE_INTEGER li;
    QueryPerformanceFrequency(&li)
    cpu_freq = li.QuadPart; //ns
#endif
}

uint64_t PerfMon::get_time_ns(void)const{
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

void PerfMon::push_query(const char* name){
    blocks_[curr_wpos_] = PerfBlock(get_time_ns(), name, current_depth_++);
    curr_wpos_ = (curr_wpos_ + 1 == MAX_PERF_BLOCKS)? 0: curr_wpos_ + 1;
}

void PerfMon::pop_query(void){
    --current_depth_;
    size_t curr_upos_ = (curr_wpos_ > 0)? curr_wpos_ - 1: MAX_PERF_BLOCKS - 1;
    while(blocks_[curr_upos_].depth_ > current_depth_){
        curr_upos_ = (curr_upos_ > 0)? curr_upos_ - 1: MAX_PERF_BLOCKS - 1;
    }
    blocks_[curr_upos_].end_time_ = get_time_ns();
}

void PerfMon::sync(void){
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
