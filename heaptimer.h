#ifndef HEAP_TIMER_H
#define HEAP_TIMER_H

#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h>
#include <functional>
#include <assert.h>
#include <chrono>
#include "log.h"

struct TimerNode
{
    int id;
    std::chrono::high_resolution_clock::time_point expires;
    std::function<void()> TimeoutCallback;
    bool operator<(const TimerNode &t)
    {
        return expires < t.expires;
    }
    TimerNode(int id_, std::chrono::high_resolution_clock::time_point expires_, std::function<void()> TimeoutCallback_)
        : id(id_), expires(expires_), TimeoutCallback(TimeoutCallback_) {}
};

class HeapTimer
{
public:
    HeapTimer() { heap_.reserve(64); }
    ~HeapTimer() { clear(); }

    void adjust(int id, int newExpires);
    void add(int id, int timeOut, const std::function<void()> &cb);
    void work(int id);
    void clear();
    void tick();
    void pop();
    int GetNextTick();

private:
    void del_(size_t i);
    void siftup_(size_t i);
    void siftdown_(size_t i);
    void SwapNode_(size_t i, size_t j);

    std::vector<TimerNode> heap_;
    // key:id value:vector的下标
    std::unordered_map<int, size_t> ref_; // id对应的在heap_中的下标，方便用heap_的时候查找
};

#endif
