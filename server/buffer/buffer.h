#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>	 // for read/write
#include <sys/uio.h> // for readv
#include <sstream>
#include <fstream>

class Buffer
{
public:
	Buffer(size_t initialSize = 4096)
		: buffer_(initialSize), readIndex_(0), writeIndex_(0) {}
	~Buffer();

	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;
	
	Buffer(Buffer &&oth) noexcept{
		buffer_ = std::move(oth.buffer_);
		readIndex_ = oth.readIndex_;
		writeIndex_ = oth.writeIndex_;
		oth.readIndex_ = 0;
		oth.writeIndex_ = 0;
	}
	
	Buffer& operator=(Buffer &&oth) noexcept{
		if(this != &oth){
			buffer_ = std::move(oth.buffer_);
			readIndex_ = oth.readIndex_;
			writeIndex_ = oth.writeIndex_;
			oth.readIndex_ = 0;
			oth.writeIndex_ = 0;
		}
		return *this;
	}


	// 可读取数据的长度
	size_t readableBytes() const { return writeIndex_ - readIndex_; }

	// 可写入数据的长度
	size_t writableBytes() const { return buffer_.size() - writeIndex_; }

	// 缓冲区前面的空间（可以被重用的空间）
	size_t prependableBytes() const { return readIndex_; }

	// 返回可读数据的指针（不会修改缓冲区状态）
	const char* peek() const { return &*begin() + readIndex_; }

	// 读取 len 个字节的数据，并更新读指针
	void retrieve(size_t len);

	// 清空缓冲区
	void retrieveAll()
	{
		readIndex_ = 0;
		writeIndex_ = 0;
	}

	// 返回缓冲区中的所有数据，并清空缓冲区
	std::string retrieveAllAsString();

	// 添加数据到缓冲区
	void append(const char *data, size_t len);

	// 添加 std::string 到缓冲区
	void append(const std::string &str);

	// 确保缓冲区有足够的可写空间
	void ensureWritableBytes(size_t len);

	// 从套接字中读取数据到缓冲区
	ssize_t readFd(int fd, int *savedErrno);

	// 将缓冲区中的数据写入套接字
	ssize_t writeFd(int fd, int *savedErrno);

	bool readFile(const std::string &filePath);

	bool writeFile(const std::string &filePath);

private:
	// 返回缓冲区的起始地址
	char* begin() { return buffer_.data(); }
	const char* begin() const { return buffer_.data(); }

	// 返回可写位置的指针
	char* beginWrite() { return begin() + writeIndex_; }
	const char* beginWrite() const { return begin() + writeIndex_; }

	// 更新写指针
	void hasWritten(size_t len) { writeIndex_ += len; }

	// 扩展缓冲区
	void makeSpace(size_t len);

private:
	std::vector<char> buffer_; // 存储数据的缓冲区
	size_t readIndex_;		   // 可读数据的起始位置
	size_t writeIndex_;		   // 可写数据的起始位置
};

#endif // BUFFER_H
