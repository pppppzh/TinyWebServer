#ifndef blockqueue_H
#define BOOCLQUEUE_H

#include <queue>
#include <condition_variable>
#include <mutex>
#include <sys/time.h>

template <typename T>
class blockqueue
{
private:
    std::deque<T> deq_;
    std::mutex mtx_;
    bool isClose_;
    size_t capacity_;
    std::condition_variable consumer_;
    std::condition_variable producer_;

public:
    explicit blockqueue(size_t size = 1000);
    ~blockqueue();
    void close();
    bool empty();
    bool full();
    void push_back(const T &);
    void push_front(const T &);
    void pop();
    bool pop(int);
    void clear();
    T front();
    T back();
    size_t capacity();
    size_t size();
    void flush();
};

template <typename T>
blockqueue<T>::blockqueue(size_t size)
    : capacity_(size)
{
    assert(size > 0);
    isClose_ = false;
}

template <typename T>
blockqueue<T>::~blockqueue()
{
    close();
}

template <typename T>
void blockqueue<T>::clear()
{
    std::lock_guard<std::mutex> lock(mtx_);
    deq_.clear();
}

template <typename T>
void blockqueue<T>::close()
{
    clear();
    isClose_ = true;
    producer_.notify_all();
    consumer_.notify_all();
}

template <typename T>
bool blockqueue<T>::empty()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return deq_.empty();
}

template <typename T>
bool blockqueue<T>::full()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return deq_.size() >= capacity_;
}

template <typename T>
T blockqueue<T>::front()
{
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.front();
}

template <typename T>
T blockqueue<T>::back()
{
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.back();
}

template <typename T>
size_t blockqueue<T>::capacity()
{
    std::lock_guard<std::mutex> locker(mtx_);
    return capacity_;
}

template <typename T>
size_t blockqueue<T>::size()
{
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.size();
}

template <typename T>
void blockqueue<T>::push_back(const T &item)
{
    std::unique_lock<std::mutex> lock(mtx_);
    while (deq_.size() >= capacity_)
    {
        producer_.wait(lock);
    }
    deq_.push_back(item);
    consumer_.notify_one();
}

template <typename T>
void blockqueue<T>::push_front(const T &item)
{
    std::unique_lock<std::mutex> lock(mtx_);
    while (deq_.size() >= capacity_)
    {
        producer_.wait(lock);
    }
    deq_.push_front(item);
    consumer_.notify_one();
}

template <typename T>
void blockqueue<T>::pop()
{
    std::unique_lock<std::mutex> lock(mtx_);
    while (deq_.empty())
    {
        consumer_.wait(lock);
    }
    deq_.pop_front();
    producer_.notify_one();
}

template <typename T>
bool blockqueue<T>::pop(int timeout)
{
    std::unique_lock<std::mutex> lock(mtx_);
    while (deq_.empty())
    {
        if (consumer_.wait_for(lock, std::chrono::seconds(timeout)) == std::cv_status::timeout)
            retrurn false;
        if (isClose_)
            return false;
    }
    deq_.pop_front();
    producer_.notify_one();
    return true;
}

template <typename T>
void blockqueue<T>::flush()
{
    consumer_.notify_one();
}

#endif