#include "buffer.h"

buffer::buffer(int size) : buffer_(size), readPos_(0), writePos_(0) {}

size_t buffer::WritableBytes() const
{
    return buffer_.size() - writePos_;
}

size_t buffer::ReadableBytes() const
{
    return writePos_ - readPos_;
}

size_t buffer::PrependableBytes() const
{
    return readPos_;
}

const char *buffer::Peek() const
{
    return &buffer_[readPos_];
}

void buffer::EnsureWriteable(size_t len)
{
    if (len > WritableBytes())
    {
        MakeSpace(len);
    }
    assert(WritableBytes() >= len);
}

void buffer::HasWritten(size_t len)
{
    writePos_ += len;
}

void buffer::Retrieve(size_t len)
{
    readPos_ += len;
}

void buffer::RetrieveUntil(const char *end)
{
    const char *start = Peek();
    assert(start < end);
    Retrieve(end - start);
}

void buffer::RetrieveAll()
{
    readPos_ = 0;
    writePos_ = 0;
}

std::string buffer::RetrieveAllToStr()
{
    std::string str(Peek(), ReadableBytes());
    RetrieveAll();
    return str;
}

const char *buffer::BeginWriteConst() const
{
    return &buffer_[writePos_];
}

char *buffer::BeginWrite()
{
    return &buffer_[writePos_];
}

void buffer::Append(const char *str, size_t len)
{
    assert(str);
    EnsureWriteable(len);
    std::copy(str, str + len, BeginWrite());
    HasWritten(len);
}

void buffer::Append(const std::string &str)
{
    Append(str.c_str(), str.size());
}

void buffer::Append(const void *data, size_t len)
{
    Append(static_cast<const char *>(data), len);
}

void buffer::Append(const buffer &buff)
{
    Append(buff.Peek(), buff.ReadableBytes());
}

ssize_t buffer::ReadFd(int fd, int *Errno)
{
    char buff[65535];
    struct iovec iov[2];
    size_t writable = WritableBytes();
    iov[0].iov_base = BeginWrite();
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);
    ssize_t len = readv(fd, iov, 2);
    if (len < 0)
    {
        *Errno = errno;
    }
    else if (static_cast<size_t>(len) <= writable)
    {
        writePos_ += len;
    }
    else
    {
        writePos_ = buffer_.size();
        Append(buff, static_cast<size_t>(len - writable));
    }
    return len;
}

ssize_t buffer::WriteFd(int fd, int *Errno)
{
    ssize_t len = write(fd, Peek(), ReadableBytes());
    if (len < 0)
        *Errno = errno;
    else
        Retrieve(len);
    return len;
}

char *buffer::Begin()
{
    return &buffer_[0];
}

const char *buffer::Begin() const
{
    return &buffer_[0];
}

void buffer::MakeSpace(size_t len)
{
    if (WritableBytes() + PrependableBytes() < len)
    {
        buffer_.resize(writePos_ + len);
    }
    else
    {
        size_t read = ReadableBytes();
        std::copy(Begin() + readPos_, Begin() + writePos_, Begin());
        readPos_ = 0;
        writePos_ = read;
        assert(read == ReadableBytes());
    }
}