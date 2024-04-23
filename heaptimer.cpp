#include "heaptimer.h"

void HeapTimer::SwapNode_(size_t i,size_t j)
{
    assert(i>=0&&j>=0&&i<heap_.size()&&j<heap_.size());
    std::swap(heap_[i],heap_[j]);
    ref_[heap_[i].id]=i;
    ref_[heap_[j].id]=j;
}

void HeapTimer::siftup_(size_t i)
{
    assert(i>=0 && i<heap_.size());
    size_t parent = (i-1)/2;
    while(parent>=0)
    {
        if(heap_[parent]<heap_[i]) break;
        SwapNode_(i,parent);
        i=parent;
        parent=(i-1)/2;
    }
}

bool HeapTimer::siftdown_(size_t i,size_t n)
{
    assert(i >= 0 && i < heap_.size());
    assert(n >= 0 && n <= heap_.size()); 

}

void HeapTimer::del_(size_t index)
{
    assert(index>=0 && index<heap_.size());
    size_t tmp = index;
    size_t n = heap_.size() - 1;
    
}