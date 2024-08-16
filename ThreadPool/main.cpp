#include <iostream>
#include <chrono>
#include <thread>

#include "threadpool.h"

class MyTask : public Task {
public:
	void run() {
		std::cout << ">>> [LOGS] tid: " << std::this_thread::get_id() << " RUN TASK..." << std::endl;
		for (int i = 0; i < 5; i++) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		std::cout << ">>> [LOGS] tid: " << std::this_thread::get_id() << " √ FINISH TASK..." << std::endl;
	}
};

// 主函数入口
int main(void) {
	ThreadPool pool;
	pool.start(4);
	
	pool.submitTask(std::make_shared<MyTask>());
	pool.submitTask(std::make_shared<MyTask>());
	pool.submitTask(std::make_shared<MyTask>());
	pool.submitTask(std::make_shared<MyTask>());
	pool.submitTask(std::make_shared<MyTask>());
	pool.submitTask(std::make_shared<MyTask>());
	pool.submitTask(std::make_shared<MyTask>());
	pool.submitTask(std::make_shared<MyTask>());

	getchar();
}