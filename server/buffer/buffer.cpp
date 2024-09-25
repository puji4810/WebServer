#include "buffer.h"
#include "../loger/log.h"

void Buffer::retrieve(size_t len)
{
	if (len < readableBytes())
	{
		readIndex_ += len;
	}
	else
	{
		retrieveAll();
	}
}

std::string Buffer::retrieveAllAsString()
{
	std::string res(peek(), readableBytes());
	retrieveAll();
	return res;
}

void Buffer::append(const char *data, size_t len)
{
	ensureWritableBytes(len);
	std::copy(data, data + len, beginWrite());
	writeIndex_ += len;
}

void Buffer::append(const std::string &str)
{
	append(str.data(), str.size());
}

void Buffer::ensureWritableBytes(size_t len)
{
	if (writableBytes() < len)
	{
		makeSpace(len);
	}
}

ssize_t Buffer::readFd(int fd, int *savedErrno)
{
	char extraBuffer[65536]; // 一个额外的栈上缓冲区，用于处理大数据块
	struct iovec vec[2];
	const size_t writable = writableBytes();

	// 第一块是我们缓冲区中剩余的可写空间
	vec[0].iov_base = begin() + writeIndex_;
	vec[0].iov_len = writable;

	// 第二块是额外的栈缓冲区
	vec[1].iov_base = extraBuffer;
	vec[1].iov_len = sizeof(extraBuffer);

	// 读两个缓冲区，避免过多的内存拷贝
	const ssize_t n = ::readv(fd, vec, 2);
	if (n < 0)
	{
		*savedErrno = errno;
		LOG_ERROR("BUFFER::writeFd has error:%d", *savedErrno);
	}
	else if (static_cast<size_t>(n) <= writable)
	{
		// 如果数据量小于或等于我们现有的缓冲区可写空间，直接写入缓冲区
		hasWritten(n);
	}
	else
	{
		// 如果数据量超过现有缓冲区空间，将剩余数据写入栈缓冲区
		hasWritten(writable);
		append(extraBuffer, n - writable);
	}
	return n;
}

ssize_t Buffer::writeFd(int fd, int *savedErrno)
{
	const size_t readable = readableBytes();
	const ssize_t n = ::write(fd, peek(), readable);
	if (n < 0)
	{
		*savedErrno = errno;
		LOG_ERROR("BUFFER::writeFd has error:%d", *savedErrno);
	}
	else
	{
		retrieve(n); // 更新读指针，表明已处理数据
	}
	return n;
}

void Buffer::makeSpace(size_t len)
{
	if (writableBytes() + prependableBytes() < len)
	{
		// 如果缓冲区总空间不够，直接扩展
		buffer_.resize(writeIndex_ + len);
	}
	else
	{
		// 如果总空间够但前面有未使用的空间，可以进行数据的移动
		const size_t readable = readableBytes();
		std::copy(begin() + readIndex_, begin() + writeIndex_, begin());
		readIndex_ = 0;
		writeIndex_ = readable;
	}
}

bool Buffer::readFile(const std::string& filePath){
	std::ifstream file(filePath, std::ios::binary);
	if(!file.is_open()){
		return false;
	}

	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	this->append(content);
	file.close();
	return true;
}

bool Buffer::writeFile(const std::string& filePath){
	std::ofstream file(filePath, std::ios::binary);
	if(!file.is_open()){
		return false;
	}

	file.write(this->peek(), this->readableBytes());
	file.close();
	return true;
}