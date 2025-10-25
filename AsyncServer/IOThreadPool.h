#pragma once
#include "Singleton.h"
#include <thread>
#include <mutex>
#include <boost/asio.hpp> 
using namespace boost;

class IOThreadPool:public Singleton<IOThreadPool>
{
	friend Singleton<IOThreadPool>;
public:
	~IOThreadPool() {}
	IOThreadPool(const IOThreadPool&) = delete;
	IOThreadPool& operator=(const IOThreadPool&) = delete;
	asio::io_context& GetIOContext();
	void stop();
private:
	IOThreadPool(int threadnum = std::thread::hardware_concurrency());
	asio::io_context io_context;
	std::unique_ptr <asio::executor_work_guard<asio::io_context::executor_type>>_works;
	std::vector<std::thread> _threads;
};

