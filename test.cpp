#include "log.h"

int main()
{
    log *lg = log::Instance();
    lg->init(0,0);
    LOG_DEBUG("11111111111111");
    LOG_INFO("222222222222");
    LOG_WARN("333333333333333");
    LOG_ERROR("4444444444444");
    LOG_DEBUG("6666666666666");

    std::cout<<"pp";

    return 0;
}