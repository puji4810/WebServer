#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H
#include <mutex>
#include <condition_variable>
#include <queue>
#include <sys/time.h>	
#include <stdexcept>
template<typename T>
class BlockQueue{
public:
	BlockQueue(size_t maxsize = 1000) : maxsize_(maxsize), stop_(false) {};
	BlockQueue(const BlockQueue &rhs) = delete;
	BlockQueue &operator=(const BlockQueue &rhs) = delete;

	void push(const T &item){
		std::unique_lock<std::mutex> lock(mutex_);
		condFull_.wait(lock,[this](){return stop_ || !full();});//阻塞到队列非满
		if(stop_){
			throw std::runtime_error("BlockQueue is stopped");
		}
		queue_.push(item);
		condEmpty_.notify_one();//通知等待队列非空的线程（pop里面需要队列中有元素）
	}

	T pop(){
		std::unique_lock<std::mutex> lock{mutex_};
		condEmpty_.wait(lock,[this](){return stop_ || !empty();});
		if(stop_ && empty()){
			throw std::runtime_error("BlockQueue is stopped");
		}
		T item = queue_.front();
		queue_.pop();
		condFull_.notify_one();
		return item;
	}

	size_t size(){
		std::lock_guard<std::mutex> lock(mutex_);
		return queue_.size();
	}

	bool empty(){
		std::lock_guard<std::mutex> lock(mutex_);
		return queue_.empty();
	}

	bool full(){
		std::lock_guard<std::mutex> lock(mutex_);
		return queue_.size() >= maxsize_;
	}

	void stop(){
		{
			std::lock_guard<std::mutex> lock(mutex_);
			stop_ = true;
		}
		condEmpty_.notify_all();
		condFull_.notify_all();
	}

	// size_t capacity();
	~BlockQueue()
	{
		stop();
	}

private:
	std::queue<T> que_;
	size_t maxsize_;
	std::mutex mutex_;
	std::condition_variable condEmpty_;
	std::condition_variable condFull_;
	bool stop_;
};



#endif