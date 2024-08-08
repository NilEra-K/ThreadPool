#include "threadpool.h"
#include <functional>
#include <thread>

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
void ThreadPool::submitTask(std::shared_ptr<Task> task) {

}

// �����̳߳�
void ThreadPool::start(size_t initThreadSize) {
	// ��¼�̳߳�ʼ����
	this->initThreadSize_ = initThreadSize;

	// �����̶߳���
	for (size_t i = 0; i < initThreadSize_; i++) {
		threads_.emplace_back(new Thread(std::bind(&ThreadPool::threadFunc, this));
	}

	// �����̶߳���
	for (size_t i = 0; i < initThreadSize_; i++) {
		threads_[i]->start();	// ��Ҫȥִ��һ���̺߳���, ÿһ���̺߳�������Ҫ"����"�������
	}
}

// �����̷߳���
void ThreadPool::threadFunc() {

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
	std::thread t(func_);
	t.detach();
}