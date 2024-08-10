#include "threadpool.h"
#include <functional>
#include <thread>
#include <iostream>
#include <chrono>

const size_t TASK_QUEUE_MAX_THRESHOLD = 1024;

// �̳߳ع���
ThreadPool::ThreadPool()
	: initThreadSize_(0)
	, taskSize_(0)
	, taskQueueMaxThreshold_(TASK_QUEUE_MAX_THRESHOLD)
	, poolMode_(ThreadPoolMode::MODE_FIXED)
{

}


// �̳߳�����
ThreadPool::~ThreadPool() { }

// �����̳߳ص�ģʽ
void ThreadPool::setMode(ThreadPoolMode mode) {
	this->poolMode_ = mode;
}

// �������������������������ֵ
void ThreadPool::setTaskQueueMaxThreshold(size_t threshold) {
	this->taskQueueMaxThreshold_ = threshold;
}

// ���̳߳��ύ����
void ThreadPool::submitTask(std::shared_ptr<Task> sp_task) {
	// ��ȡ��
	std::unique_lock<std::mutex> lock(this->taskQueueMtx_);
	
	// �̵߳�ͨ��, �ȴ���������п����λ��
	// ���ǿ���ʹ��һ�ּ���д��, ����: 
	// notFull_.wait(lock, [&]()->bool { return taskQueue.size() < taskQueueMaxThreshold_; });
	// �û��ύ����, ������������� 1s, �����ж��ύ����ʧ��, (�������񽵼�)����, ���ʱ�������� while ѭ���Ͳ�̫����, ������Ը��� wait_for
	// notFull_.wait_for(lock, chrono::seconds(1), [&]()->bool { return taskQueue.size() < taskQueueMaxThreshold_; });
	//
	// while (taskQueue.size() == taskQueueMaxThreshold_) {
	//     this->notFull_.wait(lock);
	// }
	if (!notFull_.wait_for(lock, std::chrono::seconds(1), 
		[&]()->bool { return taskQueue.size() < taskQueueMaxThreshold_; })) {
		// notNull_.wait_for(...) �ȴ� 1s ��, ���ֻ��ǲ���������, �򷵻� false
		// ������������ʱ, ��������
		std::cerr << ">>> [ERROR] taskQueue IS FULL, Submit Task Failed..." << std::endl;
		return;
	}

	// ����п���, ������������������
	taskQueue.emplace(sp_task);
	taskSize_++;

	// ��Ϊ�·�������, �����������һ������, �� notNull_ ����֪ͨ -> �Ͻ�֪ͨ�̳߳�ִ������
	notNull_.notify_all();
}

// �����̳߳�
void ThreadPool::start(size_t initThreadSize) {
	// ��¼�̳߳�ʼ����
	this->initThreadSize_ = initThreadSize;

	// �����̶߳���
	for (size_t i = 0; i < initThreadSize_; i++) {
		std::unique_ptr<Thread> ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this));
		// std::unique_ptr<Thread, std::default_delete<Thread>>::unique_ptr(const std::unique_ptr<Thread, std::default_delete<Thread>> &)
		// ����������ɾ���ĺ���
		// ������Ϊ unique_ptr һ����Դֻ�ܱ�һ��ָ��ָ��, ���Բ�����ʹ�ÿ�������
		// emplace_back(ptr) �ײ���Ҫ��������һ������ָ�����, �����ǲ��������
		// ������Ҫ�� emplace_back(ptr) �޸�Ϊ emplace_back(move(ptr));
		threads_.emplace_back(std::move(ptr));
	}

	// �����̶߳���
	for (size_t i = 0; i < initThreadSize_; i++) {
		threads_[i]->start();	// ��Ҫȥִ��һ���̺߳���, ÿһ���̺߳�������Ҫ"����"�������
	}
}

// �����̺߳���
void ThreadPool::threadFunc() {
	std::cout << "Start Thread Function, tid: " << std::this_thread::get_id() << std::endl;
	for (;;) {
		std::shared_ptr<Task> task;
		{
			// �Ȼ�ȡ��
			std::unique_lock<std::mutex> lock(this->taskQueueMtx_);

			// �ȴ� notNull_ ����
			notNull_.wait(lock, [&]()->bool { return taskQueue.size() > 0; });

			// �����������, ȡһ���������, ȡ��������ͷ���
			// ������ͷ�, ���Ǿ���Ҫ�ȴ����, for ѭ��������, ��ʱ�����Ѿ�ִ����ϲ��ͷ���
			// ��ͻ�����̳߳���ֻ��һ���߳��ڹ���������
			task = taskQueue.front();
			taskQueue.pop();
			taskSize_--;

			// �������ʣ������, ����֪ͨ�����߳�ִ������
			if (taskQueue.size() > 0) {
				notNull_.notify_all();
			}

			// ȡ����������֪ͨ, ֪ͨ���Լ����ύ��������
			notFull_.notify_all();
		} // ���˴�����������, �Զ��ͷ���

		// ��ǰ�̸߳���ִ���������, ����̳ж�̬ʹ��
		if (task != nullptr) {
			task->run();
		}
	}
	std::cout << "  End Thread Function, tid: " << std::this_thread::get_id() << std::endl;
}

//////////// �̷߳�����ʵ��
// ���캯��
Thread::Thread(ThreadFunc func) 
	: func_(func)
{ }

// ��������
Thread::~Thread() { }

void Thread::start() {
	// ����һ���߳���ִ���̺߳���
	std::thread t(func_);	// �̶߳��� t, �̺߳��� func
	t.detach();				// ���÷����߳�
}