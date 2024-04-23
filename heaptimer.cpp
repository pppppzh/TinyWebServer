#include "heaptimer.h"

void HeapTimer::SwapNode_(size_t i,size_t j)
{
    assert(i>=0&&j>=0&&i<heap_.size()&&j<heap_.size());
    std::swap(heap_[i],heap_[j]);
    ref_[heap_[i].id]=i;
    ref_[heap_[j].id]=j;
}

void HeapTimer::siftup_(size_t i) {
    assert(i >= 0 && i < heap_.size());
    size_t parent = (i-1) / 2;
    while(parent >= 0) {
        if(heap_[parent] > heap_[i]) {
            SwapNode_(i, parent);
            i = parent;
            parent = (i-1)/2;
        } else {
            break;
        }
    }
}

void HeapTimer::siftdown_(size_t i)
{
    size_t n = heap_.size();
    assert(i >= 0 && i < heap_.size());
    int child = i*2+1;
    while(child<n)
    {
        if(child<n-1 && heap_[child+1]<heap_[child]) child++;
        if(heap_[child]<heap_[i])
        {
            SwapNode_(child,i);
            i=child;
            child = i*2+1;
        }
        else break;
    }
}

void HeapTimer::adjust(int id, int newExpires)
{
    assert(ref_.count(id));
    heap_[ref_[id]].expires = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(newExpires);
    siftdown_(ref_[id]);
}

void HeapTimer::del_(size_t index)
{
    assert(index>=0 && index<heap_.size());
    SwapNode_(index,heap_.size()-1);
    ref_.erase(heap_.back().id);
    heap_.pop_back();
    siftdown_(index);
}