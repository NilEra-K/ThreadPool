#include <iostream>
#include <chrono>
#include <thread>

#include "threadpool.h"

// 主函数入口
int main(void) {
	ThreadPool pool;
	pool.start(6);
	std::this_thread::sleep_for(std::chrono::seconds(5));
}