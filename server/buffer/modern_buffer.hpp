//
// Created by puji on 25-3-9.
//

#ifndef MODERN_Buffer_Vector_H
#define MODERN_Buffer_Vector_H
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sys/uio.h>
#include <vector>
#include "../loger/log.h"
#include "sys/uio.h"

template<typename T>
concept CharacterType = std::is_same_v<char, std::decay_t<T>> || std::is_same_v<std::string, std::decay_t<T>>;

// template <typename T>
// concept ByteType = std::is_same_v<uint8_t, std::decay_t<T>>
//                  || std::is_same_v<std::byte, std::decay_t<T>>;

template<typename T>
concept SupportType = CharacterType<T>;

template<SupportType T>
class Buffer_Vector {
public:
    Buffer_Vector(std::size_t Buffer_Vector_size = 4096) { buffer_.reserve(Buffer_Vector_size); }
    ~Buffer_Vector() = default;
    Buffer_Vector(const Buffer_Vector &) = delete;
    Buffer_Vector &operator=(const Buffer_Vector &) = delete;
    Buffer_Vector(Buffer_Vector &&oth) noexcept = default;
    Buffer_Vector &operator=(Buffer_Vector &&oth) noexcept = default;

    std::size_t size() const { return buffer_.size(); }
    std::size_t capacity() const { return buffer_.capacity(); }
    void retrieveAll() { buffer_.clear(); }
    void retrieve(std::size_t n) { buffer_.erase(buffer_.begin(), buffer_.begin() + n); }
    std::string retrieveAllAsString() {
        std::string ret(buffer_.begin(), buffer_.end());
        buffer_.clear(); // 清空缓冲区
        return ret;
    }

    ssize_t readFd(int fd, int *savedErrno) {
        // std::vector<char> BigBuffer_Vector(65536); // 容量固定为65536
        // struct iovec iov[2];
        // std::size_t writeable = buffer_.capacity() - buffer_.size();
        //
        // iov[0].iov_base = buffer_.data();
        // iov[0].iov_len = writeable;
        //
        // iov[1].iov_base = BigBuffer_Vector.data();
        // iov[1].iov_len = BigBuffer_Vector.size();
        //
        // ssize_t n = readv(fd, iov, 2);
        // std::cout << "read : " << n << std::endl;
        // if (n < 0) {
        //     *savedErrno = errno;
        //     if (errno != EAGAIN && errno != EWOULDBLOCK)
        //     {
        //         // LOG_ERROR("Buffer::readFd has error:%d", *savedErrno);
        //     }
        //     return -1;
        // }
        // else if ( n > writeable)
        // {
        //     buffer_.insert(buffer_.end(), BigBuffer_Vector.begin(), BigBuffer_Vector.begin() + (n - writeable));
        // }
        // return n;
        buffer_.resize(9096); // 设置缓冲区容量

        ssize_t n = read(fd, buffer_.data(), buffer_.capacity());
        if (n < 0) {
            *savedErrno = errno;
            return -1;
        }

        return n;
    }
    ssize_t writeFd(int fd, int *savedErrno) {
        ssize_t n = write(fd, buffer_.data(), buffer_.size());
        if (n < 0) {
            *savedErrno = errno;
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                // LOG_ERROR("Buffer_Vector::writeFd has error:%d", *savedErrno);
            }
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                // LOG_ERROR("Buffer_Vector::writeFd has error:%d", *savedErrno);
            }
            return -1;
        } else {
            retrieve(n);
            return n;
        }
    }

    bool readFile(const std::string &fileName) {
        std::ifstream file(fileName, std::ios::binary);
        if (!file.is_open()) {
            // LOG_ERROR("Buffer_Vector::readFile: Failed to open file %s", fileName.c_str());
            return false;
        }
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        buffer_.insert(buffer_.end(), content.begin(), content.end());
        file.close();
        return true;
    }

    bool writeFile(const std::string &fileName) {
        std::ofstream file(fileName, std::ios::binary);
        if (!file.is_open()) {
            // LOG_ERROR("Buffer_Vector::writeFile: Failed to open file %s", fileName.c_str());
            return false;
        }
        file.write(buffer_.data(), buffer_.size());
        buffer_.clear();
        file.close();
        return true;
    }

    void append(std::string extra) { buffer_.insert(buffer_.end(), extra.begin(), extra.end()); }

    void append(const char *extra, std::size_t len) { buffer_.insert(buffer_.end(), extra, extra + len); }

    void append(const std::vector<T> &extra) { buffer_.insert(buffer_.end(), extra.begin(), extra.end()); }

private:
    std::vector<T> buffer_{};
};

using Buffer = Buffer_Vector<char>;


#endif // MODERN_Buffer_Vector_H
