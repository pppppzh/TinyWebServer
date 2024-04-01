# TinyWebServer

## 1.buffer类
由于使用非阻塞I/O模型，所以需要实现应用层buffer。即在应用程序发送数据时，并不需要关注生产的数据如何发送（一次性或分成多次发送），只需要调用send()函数，将数据交给buffer，等待socket变为可写时buffer将存储的数据进行发送。同理，在应用程序接受数据时，由于TCP协议是无边界的字节流协议，极大可能会出现数据流不完整情况，所以收到的数据要放到buffer中，等接受消息完整后，再通知应用程序进行读取。

buffer类的设计参考了muduo网络库，内部使用vector\<char>进行数据存储。为了防止迭代器失效的问题，buffer类中使用了两个int类型的成员变量作为index，即readIndex、writeIndex。两个index将vector分成三份：prependable、readable、writable。当向buffer写入数据，writeIndex向后移动；当从buffer取出数据，readIndex向后移动。经过多次读写后，readIndex可能移动到较后的位置，留下了较大的prependable空间，此时如果写入的数据大小大于writable空间并且小于writable+prependable的大小，buffer不会重新分配内存，而是把已有的数据移动到prependable。

在buffer::ReadFd()函数中，在栈上申请了一个stackbuff，利用readv()读取数据，将数据读入到buffer和stackbuf中。当读入的数据不多时，全部读入buffer中；如果长度超过buffer大小，就会读到stackbuff中。减少了系统调用的同时，降低了系统内存占用。


## 2.日志系统
