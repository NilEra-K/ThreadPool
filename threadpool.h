#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <vector>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>

// 任务抽象基类, 继承、多态的思想
class Task {
public:
	// 用户可以自定义任意任务类型, 从 Task 继承, 重写 run() 方法, 实现自定义任务处理
	virtual void run() = 0;
private:
};


// 线程池支持的模式
// enum class 运行程序员以 ThreadPoolMode::MODE_FIXED 方式进行枚举量的使用
// 防止出现枚举中出现相同命名的枚举量
enum class ThreadPoolMode {
	MODE_FIXED,		// 固定数量的线程
	MODE_CACHED,	// 线程数量可动态增长
};

// 线程类
class Thread {
public:
	// 线程函数对象类型
	using ThreadFunc = std::function<void()>;

	// 构造函数
	Thread(ThreadFunc func);

	// 析构函数
	~Thread();

	// 启动线程
	void start();
private:
	ThreadFunc func_;
};

// 抽象线程池类型
class ThreadPool {
public:
	// 线程池的构造函数
	ThreadPool();

	// 线程池的析构函数
	~ThreadPool();

	// 设置线程池的工作模式
	void setMode(ThreadPoolMode mode);
	
	// 设置任务队列任务数的上限阈值
	void setTaskQueueMaxThreshold(size_t threshold);

	// 给线程池提交任务
	void submitTask(std::shared_ptr<Task> task);

	// 开启线程池
	void start(size_t initThreadSize = 4);

	// 禁止用户对线程池对象本身进行拷贝和赋值操作
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

private:
	// 定义线程函数
	void threadFunc() {

	}

private:
	std::vector<Thread*> threads_;	// 线程列表
	size_t initThreadSize_;			// 初始线程数量
	ThreadPoolMode poolMode_;		// 当前线程池的工作模式

	std::queue<std::shared_ptr<Task>> taskQueue;	// 任务队列
	std::atomic_size_t taskSize_;					// 任务队列中任务数量
	size_t taskQueueMaxThreshold_;					// 任务队列中任务数量的最大阈值
	std::mutex taskQueueMtx_;						// 用于保证任务队列的线程安全
	std::condition_variable notFull_;				// 表示任务队列不满
	std::condition_variable notNull_;				// 表示任务队列不空

};

#endif // !_THREADPOOL_H
