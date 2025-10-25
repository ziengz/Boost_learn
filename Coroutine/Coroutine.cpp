// Coroutine.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/signal_set.hpp>
using boost::asio::ip::tcp;
using boost::asio::detached;
using boost::asio::co_spawn;
using boost::asio::use_awaitable;
using boost::asio::awaitable;
namespace this_coro = boost::asio::this_coro;  //获取当前协程
const int MAX_LENGTH = 1024;

awaitable<void>echo(tcp::socket sock)
{
    try {
        char data[MAX_LENGTH] = { 0 };
        std::cout << "accepted one client" << std::endl;
        for (;;) {
            //挂起等待读取数据
            std::size_t n = co_await sock.async_read_some(boost::asio::buffer(data), use_awaitable);
            std::cout << "recv data is " << std::string(data,n) << std::endl;
            //写成挂起等待写入完成
            co_await async_write(sock,boost::asio::buffer(data, n), use_awaitable);
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "!!!Exception is " << e.what() << std::endl;
    }
}

awaitable<void> listener()
{
    auto executor = co_await this_coro::executor;
    tcp::acceptor acceptor(executor, { boost::asio::ip::make_address("127.0.0.1"),10086});
    for (;;) {
        //协程挂起等待连接 （co_await挂起等待）
        tcp::socket socket = co_await acceptor.async_accept(use_awaitable);  //use_awaitable表示告诉asio使用协程方式，并且可等待
        
        co_spawn(executor, echo(std::move(socket)), detached);
        
    }
}

int main()
{
    try {
        boost::asio::io_context ioc;
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](auto, auto) {
            ioc.stop();
            });
        co_spawn(ioc, listener(), detached);
        ioc.run();
    }
    catch (std::exception& e)
    {
        std::cout << "Exception is " << e.what() << std::endl;

    }
}