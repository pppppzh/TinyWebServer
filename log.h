#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <string>
#include <thread>
#include <sys/time.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/stat.h>
#include "blockqueue.hpp"
#include "buffer.h"

#define LOG_BASE(level, format, ...)                 \
    do                                               \
    {                                                \
        log *lg = log::Instance();                   \
        if (lg->IsOpen() && lg->getLevel() <= level) \
        {                                            \
            lg->write(level, format, ##__VA_ARGS__); \
            lg->flush();                             \
        }                                            \
    } while (0);

#define LOG_DEBUG(format, ...)             \
    do                                     \
    {                                      \
        LOG_BASE(0, format, ##__VA_ARGS__) \
    } while (0);
#define LOG_INFO(format, ...)              \
    do                                     \
    {                                      \
        LOG_BASE(1, format, ##__VA_ARGS__) \
    } while (0);
#define LOG_WARN(format, ...)              \
    do                                     \
    {                                      \
        LOG_BASE(2, format, ##__VA_ARGS__) \
    } while (0);
#define LOG_ERROR(format, ...)             \
    do                                     \
    {                                      \
        LOG_BASE(3, format, ##__VA_ARGS__) \
    } while (0);

class log
{
private:
    static const int PATH_LEN = 256;
    static const int NAME_LEN = 256;
    static const int MAX_LINES = 50000;

    const char *path_;
    const char *suffix_;
    int max_lines_;
    int lines_;
    int day_;
    bool isOpen_;
    buffer buff_;
    int level_;
    bool isAsync_;

    FILE *fp_;
    std::unique_ptr<blockqueue<std::string>> deque_;
    std::unique_ptr<std::thread> writeThread_;
    std::mutex mtx_;

    log();
    virtual ~log();
    void LogLevel(int);
    void AsyncWrite();

public:
    void init(int level, int queuecapacity = 1024, const char *path = "./log", const char *suffix = ".log");
    static log *Instance();
    static void FlushLogThread();
    void write(int level, const char *format, ...);
    void flush();
    int getLevel();
    void setLevel(int);
    bool IsOpen();
};

#endif // LOG_H
