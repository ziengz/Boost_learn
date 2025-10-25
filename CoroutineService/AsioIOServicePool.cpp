#include "AsioIOServicePool.h"
#include "LogicSystem.h"

AsioIOServicePool::AsioIOServicePool(std::size_t threadnum) : _io_contexts(threadnum), _works(threadnum), _nextIOContext(0)
{
    for (size_t i = 0; i < threadnum; ++i) {
        _works[i] = std::make_unique<Work>(boost::asio::make_work_guard(_io_contexts[i]));
    }
    for (std::size_t i = 0; i < threadnum; ++i) {
        _threads.emplace_back([this, i] {
            _io_contexts[i].run();
            });
    }
}
io_context& AsioIOServicePool::GetIOContext()
{
    auto& service = _io_contexts[_nextIOContext++];
    if (_nextIOContext == _io_contexts.size()) {
        _nextIOContext = 0;
    }
    return service;
}

AsioIOServicePool::~AsioIOServicePool()
{
    std::cout << "AsioIOServicePool destruct" << std::endl;
}

void AsioIOServicePool::stop()
{
    for (auto& w : _works) {
        w.reset();
    }

    for (auto& t : _threads) {
        t.join();
    }
}

AsioIOServicePool& AsioIOServicePool::GetInstance()
{
    static AsioIOServicePool instance;
    return instance;
}




