#include "IOThreadPool.h"

IOThreadPool::IOThreadPool(int threadnum) :
	_works(std::make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(
	io_context.get_executor()))
{
	for (int i = 0; i < threadnum; i++)
	{
		_threads.emplace_back([this] {
			io_context.run();
			});
	}
}

asio::io_context& IOThreadPool::GetIOContext()
{
	return io_context;
}

void IOThreadPool::stop() {
	io_context.stop();
	_works.reset();
	for(auto& t:_threads)
	{
		t.join();
	}
}
