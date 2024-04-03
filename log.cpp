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
    while (!deque_->empty())
    {
        deque_->flush();
    }
    deque_->close();
    writeThread_->join();
    if (fp_)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        flush();
        fclose(fp_);
    }
}

bool log::IsOpen() 
{ 
    return isOpen_; 
}

void log::flush()
{
    if (isAsync_)
        deque_->flush();
    fflush(fp_);
}

log *log::Instance()
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
    while (1)
    {
        std::string str = deque_->front();
        deque_->pop();
        std::lock_guard<std::mutex> lock(mtx_);
        fputs(str.c_str(), fp_);
    }
}

void log::init(int level, int capacity, const char *path, const char *suffix)
{
    isOpen_ = true;
    level_ = level;
    path_ = path;
    suffix_ = suffix;
    if (capacity > 0)
    {
        isAsync_ = true;
        if (deque_ == nullptr)
        {
            std::unique_ptr<blockqueue<std::string>> q(new blockqueue<std::string>);
            deque_ = std::move(q);
            std::unique_ptr<std::thread> t(new std::thread(FlushLogThread));
            writeThread_ = move(t);
        }
    }
    else
    {
        isAsync_ = false;
    }
    lines_ = 0;
    time_t timer = time(nullptr);
    struct tm *systime = localtime(&timer);
    char fileName[NAME_LEN] = {0};
    snprintf(fileName, NAME_LEN, "%s/%04d_%02d_%02d%s", path_, systime->tm_year + 1900, systime->tm_mon + 1, systime->tm_mday, suffix_);
    day_ = systime->tm_mday;

    {
        std::lock_guard<std::mutex> lock(mtx_);
        buff_.RetrieveAll();
        if (fp_)
        {
            flush();
            fclose(fp_);
        }
        fp_ = fopen(fileName, "a");
        if (fp_ == nullptr)
        {
            int t = mkdir(path_, 0777);
            assert(t==0);
            fp_ = fopen(fileName, "a");
        }
        assert(fp_ != nullptr);
    }
}

void log::write(int level, const char *format, ...)
{
    time_t timer = time(nullptr);
    struct tm *systime = localtime(&timer);
    if (systime->tm_mday != day_ || lines_ > MAX_LINES)
    {
        char filename[NAME_LEN];
        char tail[36] = {0};
        snprintf(tail, 35, "%04d_%02d_%02d", systime->tm_year + 1900, systime->tm_mon + 1, systime->tm_mday);
        if (day_ != systime->tm_mday)
        {
            snprintf(filename, NAME_LEN-1, "%s/%s%s", path_, tail, suffix_);
            day_ = systime->tm_mday;
            lines_ = 0;
        }
        else
        {
            snprintf(filename, NAME_LEN-1, "%s/%s-%d%s", path_, tail, (lines_ / MAX_LINES), suffix_);
        }

        std::unique_lock<std::mutex> lock(mtx_);
        flush();
        fclose(fp_);
        fp_ = fopen(filename, "a");
        assert(fp_ != nullptr);
    }

    std::unique_lock<std::mutex> lock(mtx_);
    lines_++;
    int n = snprintf(buff_.BeginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d ",
                     systime->tm_year + 1900, systime->tm_mon + 1, systime->tm_mday,
                     systime->tm_hour, systime->tm_min, systime->tm_sec);
    buff_.HasWritten(n);
    LogLevel(level);
    va_list vaList;
    va_start(vaList, format);
    int m = vsnprintf(buff_.BeginWrite(), buff_.WritableBytes(), format, vaList);
    va_end(vaList);
    buff_.HasWritten(m);
    buff_.Append("\n\0");

    if (isAsync_ && deque_ && !deque_->full())
    {
        deque_->push_back(buff_.RetrieveAllToStr());
    }
    else
    {
        fputs(buff_.Peek(), fp_);
    }
    buff_.RetrieveAll();
}

void log::LogLevel(int level)
{
    switch (level)
    {
    case 0:
        buff_.Append("debug: ");
        break;
    case 1:
        buff_.Append("info: ");
        break;
    case 2:
        buff_.Append("warn: ");
        break;
    case 3:
        buff_.Append("error: ");
        break;
    default:
        buff_.Append("info: ");
        break;
    }
}

int log::getLevel()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return level_;
}

void log::setLevel(int level)
{
    std::lock_guard<std::mutex> lock(mtx_);
    level_ = level;
}