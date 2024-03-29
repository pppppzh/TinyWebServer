#include "buffer.h"

int main()
{
    buffer buff = buffer();
    int *Errno = 0;
    std::cout<<buff.ReadFd(2,Errno)<<std::endl;
    std::cout<<buff.WriteFd(1,Errno);

    return 0;
}