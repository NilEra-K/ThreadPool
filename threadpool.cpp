#include "threadpool.h"
#include <functional>
#include <thread>

const size_t TASK_QUEUE_MAX_THRESHOLD = 1024;

// 线程池构造
ThreadPool::ThreadPool()
	: initThreadSize_(0)
	, taskSize_(0)
	, taskQueueMaxThreshold_(TASK_QUEUE_MAX_THRESHOLD)
	, poolMode_(ThreadPoolMode::MODE_FIXED)
{

}


// 线程池析构
ThreadPool::~ThreadPool() { }

// 设置线程池的模式
void ThreadPool::setMode(ThreadPoolMode mode) {
	this->poolMode_ = mode;
}

// 设置任务队列任务数的上限阈值
void ThreadPool::setTaskQueueMaxThreshold(size_t threshold) {
	this->taskQueueMaxThreshold_ = threshold;
}

// 给线程池提交任务
void ThreadPool::submitTask(std::shared_ptr<Task> task) {

}

// 开启线程池
void ThreadPool::start(size_t initThreadSize) {
	// 记录线程初始个数
	this->initThreadSize_ = initThreadSize;

	// 创建线程对象
	for (size_t i = 0; i < initThreadSize_; i++) {
		threads_.emplace_back(new Thread(std::bind(&ThreadPool::threadFunc, this));
	}

	// 启动线程对象
	for (size_t i = 0; i < initThreadSize_; i++) {
		threads_[i]->start();	// 需要去执行一个线程函数, 每一个线程函数都需要"监视"任务队列
	}
}

// 定义线程方法
void ThreadPool::threadFunc() {

}

//////////// 线程方法的实现
// 构造函数
Thread::Thread(ThreadFunc func) 
	: func_(func)
{ }

// 析构函数
Thread::~Thread() { }

void Thread::start() {
	// 创建一个线程来执行线程函数
	std::thread t(func_);
	t.detach();
}