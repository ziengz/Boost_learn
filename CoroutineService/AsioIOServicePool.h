#pragma once
#include <boost/asio.hpp>
#include <thread>
#include <vector>
using boost::asio::ip::tcp;
using io_context = boost::asio::io_context;
using Work = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
using Workptr = std::unique_ptr<Work>;


class AsioIOServicePool
{
public:
	~AsioIOServicePool();
	AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;
	AsioIOServicePool(const AsioIOServicePool&) = delete;
	void stop();
	static AsioIOServicePool& GetInstance();
	io_context& GetIOContext();

private:
	AsioIOServicePool(std::size_t threadnum = std::thread::hardware_concurrency());
	std::vector<io_context> _io_contexts;
	std::vector<Workptr> _works;
	std::vector<std::thread> _threads;
	std::size_t _nextIOContext;
};

