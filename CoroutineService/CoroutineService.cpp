// CoroutineService.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "AsioIOServicePool.h"
#include "CServer.h"
#include <boost/asio.hpp>
using namespace boost;

int main()
{
    try {
        auto& pool = AsioIOServicePool::GetInstance();
        asio::io_context ioc;
        asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc, &pool](auto, auto) {
            ioc.stop();
            pool.stop();
            });
        CServer s(ioc, 10086);
        ioc.run();

    }
    catch (std::exception& e)
    {
        std::cout << "Exception is:" << e.what() << std::endl;
    }
}
