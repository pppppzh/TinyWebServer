#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>
#include <assert.h>

class threadpool
{
private:
    struct pool
    {
        std::mutex mtx_;
        std::condition_variable cond_;
        bool isClosed;
        std::queue<std::function<void()>> tasks; 
    };
    std::shared_ptr<pool> pool_;
    
public:
    explicit threadpool(int);
    ~threadpool();
    template<typename T>
    void AddTask(T&&);
};

explicit threadpool::threadpool(int count=8)
:pool_(std::make_shared<pool>())
{
    assert(count>0);
    for(int i=0;i<count;++i)
    {
        std::thread([this](){
            std::unique_lock<std::mutex> lock(pool_->mtx_);
            while(!pool_->isClosed)
            {
                if(!pool_->tasks.empty())
                {
                    auto task = std::move(pool_->tasks.front());
                    pool_->tasks.pop();
                    lock.unlock();
                    task();
                    lock.lock();
                }
                else
                {
                    pool_->cond_.wait(lock);
                }
            }
        }).detach();
    }
}

threadpool::~threadpool()
{
    if(pool_)
    {
        std::unique_lock<std::mutex> lock(pool_->mtx_);
        pool_->isClosed = true;
    }
    pool_->cond_.notify_all();
}

template<typename T>
void AddTask(T&& task)
{
    std::unique_lock<std::mutex> lock(pool_->mtx_);
    pool_->tasks.emplace(std::forward<T>(task));
    pool_->cond_.notify_one();
}


#endif