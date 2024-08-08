#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <vector>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>

// ����������, �̳С���̬��˼��
class Task {
public:
	// �û������Զ���������������, �� Task �̳�, ��д run() ����, ʵ���Զ���������
	virtual void run() = 0;
private:
};


// �̳߳�֧�ֵ�ģʽ
// enum class ���г���Ա�� ThreadPoolMode::MODE_FIXED ��ʽ����ö������ʹ��
// ��ֹ����ö���г�����ͬ������ö����
enum class ThreadPoolMode {
	MODE_FIXED,		// �̶��������߳�
	MODE_CACHED,	// �߳������ɶ�̬����
};

// �߳���
class Thread {
public:
	// �̺߳�����������
	using ThreadFunc = std::function<void()>;

	// ���캯��
	Thread(ThreadFunc func);

	// ��������
	~Thread();

	// �����߳�
	void start();
private:
	ThreadFunc func_;
};

// �����̳߳�����
class ThreadPool {
public:
	// �̳߳صĹ��캯��
	ThreadPool();

	// �̳߳ص���������
	~ThreadPool();

	// �����̳߳صĹ���ģʽ
	void setMode(ThreadPoolMode mode);
	
	// �������������������������ֵ
	void setTaskQueueMaxThreshold(size_t threshold);

	// ���̳߳��ύ����
	void submitTask(std::shared_ptr<Task> task);

	// �����̳߳�
	void start(size_t initThreadSize = 4);

	// ��ֹ�û����̳߳ض�������п����͸�ֵ����
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

private:
	// �����̺߳���
	void threadFunc() {

	}

private:
	std::vector<Thread*> threads_;	// �߳��б�
	size_t initThreadSize_;			// ��ʼ�߳�����
	ThreadPoolMode poolMode_;		// ��ǰ�̳߳صĹ���ģʽ

	std::queue<std::shared_ptr<Task>> taskQueue;	// �������
	std::atomic_size_t taskSize_;					// �����������������
	size_t taskQueueMaxThreshold_;					// ������������������������ֵ
	std::mutex taskQueueMtx_;						// ���ڱ�֤������е��̰߳�ȫ
	std::condition_variable notFull_;				// ��ʾ������в���
	std::condition_variable notNull_;				// ��ʾ������в���

};

#endif // !_THREADPOOL_H
