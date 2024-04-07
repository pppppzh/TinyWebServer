#include "log.h"

int main()
{
    log *lg = log::Instance();
    lg->init(0,599);
    LOG_DEBUG("1111");
    LOG_DEBUG("11");
    LOG_INFO("2");
    LOG_WARN("3");
    LOG_ERROR("4");


    return 0;
}