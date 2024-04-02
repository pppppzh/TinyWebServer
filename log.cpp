#include "log.h"

log::log()
{
    fp_ = nullptr;
    deque_ = nullptr;
    writeThread_ = nullptr;
    lines_ = 0;
    day_ = 0;
    isAsync_ = false;
}

log::~log()
{
    while(!deque_->empty())
    {
        deque_->flush();
    }
    deque_->close();
    writeThread_->join();
    if(fp_)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        flush();
        fclose(fp_);
    }
}

void log::flush()
{
    if(isAsync_)
        deque_->flush();
    fflush(fp_);
}

log* log::Instance()
{
    static log log;
    return &log;
}

void log::FlushLogThread()
{
    log::Instance()->AsyncWrite();
}

void log::AsyncWrite()
{
    while(1)
    {
        std::string str = deque_->front();
        deque_->pop();
        std::lock_guard<std::mutex> lock(mtx_);
        fputs(str.c_str(),fp_);
    }
}

void log::init(int level,const char* path,const char* suffix,int capacity)
{
    isOpen_ = true;
    level_=level;
    path_=path;
    suffix_=suffix;
    if(capacity>0)
    {
        isAsync_=true;
        if(deque_==nullptr)
        {
            std::unique_ptr<blockqueue<std::string>> q(new blockqueue<std::string>);
            deque_ = std::move(q);
            std::unique_ptr<std::thread> t(new std::thread(FlushLogThread));
            writeThread_ = move(t);
        }
    }
    else
    {
        isAsync_=false;
    }
    lines_ = 0;
    time_t timer = time(nullptr);
    struct tm* systime = localtime(&timer);
    char fileName[NAME_LEN] = {0};
    snprintf(fileName,NAME_LEN,"%s/%04d_%02d_%02d%s",path_,systime->tm_year+1900,systime->tm_mon+1,systime->tm_mday,suffix_);
    day_=systime->tm_mday;

    {
        std::lock_guard<std::mutex> lock(mtx_);
        buff_.RetrieveAll();
        if(fp_)
        {
            flush();
            fclose(fp_);
        }
        fp_ = fopen(fileName,"a");
        if(fp_ == nullptr)
        {
            mkdir(fileName,0777);
            fp_ = fopen(fileName,"a");
        }
        assert(fp_!=nullptr);
    }
}