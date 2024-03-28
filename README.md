# TinyWebServer

## 1.buffer类
由于使用非阻塞I/O模型，所以需要实现应用层buffer。即在应用程序发送数据时，并不需要关注生产的数据如何发送（一次性或分成多次发送），只需要调用send()函数，将数据交给buffer，等待socket变为可写时buffer将存储的数据进行发送。同理，在应用程序接受数据时，由于TCP协议是无边界的字节流协议，极大可能会出现数据流不完整情况，所以收到的数据要放到buffer中，等接受消息完整后，再通知应用程序进行读取。

buffer类的设计参考了muduo网络库，内部使用vector\<char>进行数据存储。为了防止迭代器失效的问题，buffer类中使用了两个int类型的成员变量作为index，即readIndex、writeIndex。