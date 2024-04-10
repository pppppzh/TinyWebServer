#include "sqlconpool.h"

sqlconpool* sqlconpool::Instance()
{
    static sqlconpool pool;
    return &pool;
}

void sqlconpool::Init(const char *host, int port, const char *user, const char *pwd, const char *dbName, int connSize)
{
    assert(connSize>0);
    for(int i=0;i<connSize;++i)
    {
        MYSQL* conn = nullptr;
        conn = mysql_init(conn);
        if(!conn)
        {
            LOG_ERROR("MySql init error");
            assert(conn);
        }
        conn = mysql_real_connect(conn,host,user,pwd,dbName,port,nullptr,0);
        if(!conn)
        {
            LOG_ERROR("MySql connect error");
        }
        connQue_.emplace(conn);
    }
    maxConn_ = connSize;
    sem_init(&semId_,0,maxConn_);
}

MYSQL* sqlconpool::getConn()
{
    sem_wait(&semId_);
    std::lock_guard<std::mutex> lock(mtx_);
    if(connQue_.empty())
    {
        LOG_WARN("SqlConnpool is busy");
        return nullptr;
    }
    MYSQL* conn = connQue_.front();
    connQue_.pop();
    return conn;
}

void sqlconpool::freeConn(MYSQL* conn)
{
    assert(conn);
    std::lock_guard<std::mutex> lock(mtx_);
    connQue_.push(conn);
    sem_post(&semId_);
}

void sqlconpool::closePool()
{
    std::lock_guard<std::mutex> lock(mtx_);
    while(!connQue_.empty())
    {
        auto conn = connQue_.front();
        connQue_.pop();
        mysql_close(conn);
    }
    mysql_library_end();
}

int sqlconpool::getFreeConnCount()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return connQue_.size();
}