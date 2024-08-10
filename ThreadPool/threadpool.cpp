#include "threadpool.h"
#include <functional>
#include <thread>
#include <iostream>
#include <chrono>

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
void ThreadPool::submitTask(std::shared_ptr<Task> sp_task) {
	// 获取锁
	std::unique_lock<std::mutex> lock(this->taskQueueMtx_);
	
	// 线程的通信, 等待任务队列有空余的位置
	// 我们可以使用一种简便的写法, 如下: 
	// notFull_.wait(lock, [&]()->bool { return taskQueue.size() < taskQueueMaxThreshold_; });
	// 用户提交任务, 最长不能阻塞超过 1s, 否则判断提交任务失败, (进行任务降级)返回, 这个时候我们用 while 循环就不太简单了, 这里可以改用 wait_for
	// notFull_.wait_for(lock, chrono::seconds(1), [&]()->bool { return taskQueue.size() < taskQueueMaxThreshold_; });
	//
	// while (taskQueue.size() == taskQueueMaxThreshold_) {
	//     this->notFull_.wait(lock);
	// }
	if (!notFull_.wait_for(lock, std::chrono::seconds(1), 
		[&]()->bool { return taskQueue.size() < taskQueueMaxThreshold_; })) {
		// notNull_.wait_for(...) 等待 1s 后, 发现还是不满足条件, 则返回 false
		// 当不满足条件时, 则进行输出
		std::cerr << ">>> [ERROR] taskQueue IS FULL, Submit Task Failed..." << std::endl;
		return;
	}

	// 如果有空余, 将任务放入任务队列中
	taskQueue.emplace(sp_task);
	taskSize_++;

	// 因为新放了任务, 所以任务队列一定不空, 在 notNull_ 进行通知 -> 赶紧通知线程池执行任务
	notNull_.notify_all();
}

// 开启线程池
void ThreadPool::start(size_t initThreadSize) {
	// 记录线程初始个数
	this->initThreadSize_ = initThreadSize;

	// 创建线程对象
	for (size_t i = 0; i < initThreadSize_; i++) {
		std::unique_ptr<Thread> ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this));
		// std::unique_ptr<Thread, std::default_delete<Thread>>::unique_ptr(const std::unique_ptr<Thread, std::default_delete<Thread>> &)
		// 尝试引用已删除的函数
		// 这是因为 unique_ptr 一个资源只能被一个指针指向, 所以不允许使用拷贝构造
		// emplace_back(ptr) 底层需要拷贝构造一份智能指针出来, 但这是不被允许的
		// 所以需要将 emplace_back(ptr) 修改为 emplace_back(move(ptr));
		threads_.emplace_back(std::move(ptr));
	}

	// 启动线程对象
	for (size_t i = 0; i < initThreadSize_; i++) {
		threads_[i]->start();	// 需要去执行一个线程函数, 每一个线程函数都需要"监视"任务队列
	}
}

// 定义线程函数
void ThreadPool::threadFunc() {
	std::cout << "Start Thread Function, tid: " << std::this_thread::get_id() << std::endl;
	for (;;) {
		std::shared_ptr<Task> task;
		{
			// 先获取锁
			std::unique_lock<std::mutex> lock(this->taskQueueMtx_);

			// 等待 notNull_ 条件
			notNull_.wait(lock, [&]()->bool { return taskQueue.size() > 0; });

			// 从任务队列中, 取一个任务出来, 取出任务后释放锁
			// 如果不释放, 我们就需要等待其出, for 循环作用域, 此时任务已经执行完毕才释放锁
			// 这就会造成线程池中只有一个线程在工作的现象
			task = taskQueue.front();
			taskQueue.pop();
			taskSize_--;

			// 如果仍有剩余任务, 继续通知其他线程执行任务
			if (taskQueue.size() > 0) {
				notNull_.notify_all();
			}

			// 取出任务后进行通知, 通知可以继续提交生产任务
			notFull_.notify_all();
		} // 出了大括号作用域, 自动释放锁

		// 当前线程负责执行这个任务, 经典继承多态使用
		if (task != nullptr) {
			task->run();
		}
	}
	std::cout << "  End Thread Function, tid: " << std::this_thread::get_id() << std::endl;
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
	std::thread t(func_);	// 线程对象 t, 线程函数 func
	t.detach();				// 设置分离线程
}