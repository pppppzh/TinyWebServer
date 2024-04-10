#ifndef SQLCONPOOL_H
#define SQLCONPOOL_H

#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <thread>
#include <mysql/mysql.h>
#include "log.h"

class sqlconpool
{
public:
    static sqlconpool *Instance();
    MYSQL *getConn();
    void freeConn(MYSQL*);
    int getFreeConnCount();
    void Init(const char *host, int port, const char *user, const char *pwd, const char *dbName, int connSize);
    void closePool();

private:
    sqlconpool() = default;
    ~sqlconpool() { closePool(); }

    int maxConn_;
    std::queue<MYSQL *> connQue_;
    std::mutex mtx_;
    sem_t semId_;
};

class sqlconRAII
{
public:
    sqlconRAII(MYSQL **sql, sqlconpool *connpool)
    {
        assert(connpool);
        *sql = connpool->getConn();
        sql_ = *sql;
        connpool_ = connpool;
    }

private:
    MYSQL *sql_;
    sqlconpool *connpool_;
};

#endif