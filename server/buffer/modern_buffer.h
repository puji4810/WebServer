//
// Created by puji on 25-3-9.
//

#ifndef MODERN_BUFFER_H
#define MODERN_BUFFER_H
#include <cstdint>
#include <iostream>
#include <vector>

template <typename T>
concept SupportType = std::is_same_v<char, std::decay_t<T>>
                    || std::is_same_v<std::string, std::decay_t<T>>
                    || std::is_same_v<uint8_t, std::decay_t<T>>
                    || std::is_same_v<std::byte, std::decay_t<T>>;

template <SupportType T>
class Buffer
{
public:
    Buffer(std::size_t buffer_size=4096) : store_size(buffer_size) {
        buffer_.reserve(buffer_size);
    }
    ~Buffer() = default;
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    Buffer(Buffer &&oth) noexcept = default;
    Buffer& operator=(Buffer &&oth) noexcept = default;

    std::size_t size() const {return buffer_.size();}
    std::size_t capacity() const {return buffer_.capacity();}
    void retriveAll() {
        buffer_.clear();
    }
    std::string retrieveAllAsString() {
        std::string ret;
        if constexpr (std::is_same_v<char, std::decay_t<T>>) {
            ret = std::string(buffer_.data(), buffer_.size());
        }
        if constexpr (std::is_same_v<std::string, std::decay_t<T>>) {
            ret = buffer_.data();
        }
        if constexpr (std::is_same_v<uint8_t, std::decay_t<T>>) {
            ret = reinterpret_cast<const char*>(buffer_.data());
        }
        if constexpr (std::is_same_v<std::byte, std::decay_t<T>>) {
            ret = reinterpret_cast<const char*>(buffer_.data());
        }
        buffer_.clear();
        return ret;
    }
private:
    std::vector<T> buffer_{};

};
#endif //MODERN_BUFFER_H
