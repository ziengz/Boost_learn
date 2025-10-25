// AsyncServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "Session.h"
#include <boost/asio.hpp>
#include "Server.h"
#include <mutex>
#include <csignal>
#include <thread>
#include "IOThreadPool.h"

bool bstop = false;
std::mutex _mutex;
std::condition_variable cond_quit;

void sig_handle(int sig)
{
    if (sig == SIGINT || sig == SIGTERM)
    {
        std::unique_lock<std::mutex> lock_quit(_mutex);
        bstop = true;
        cond_quit.notify_one();
    }
}

int main()
{
    asio::io_context ioc;
    
    try {
        //1、
        //std::thread net_work_thread([&ioc] {
        //    Server s(ioc, 10086);
        //    ioc.run();
        //   });
        //signal(SIGINT, sig_handle);
        //signal(SIGTERM, sig_handle);

        //while (!bstop)
        //{
        //    std::unique_lock<std::mutex> lock_quit(_mutex);
        //    cond_quit.wait(lock_quit);
        //}
        //ioc.stop();
        ////防止线程还没起起来，后面就退出
        //net_work_thread.join();

        //2、也可以使用asio库中的处理办法
        //asio::signal_set signals(ioc, SIGINT, SIGTERM);
        ////使用异步等待，不会阻塞主线程
        //signals.async_wait([&ioc](auto,auto) {
        //    ioc.stop();
        //    });
        //Server s(ioc, 10086);
        //ioc.run();

        //3、使用IOThreadPool
        auto pool = IOThreadPool::GetInstance();
        asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([pool,&ioc](auto, auto) {
            ioc.stop();
            pool->stop();
            std::unique_lock<std::mutex>lock(_mutex);
            bstop = true;
            cond_quit.notify_one();
            });
        //因为在IOThreadPool的GetIOContext()函数中，是开一个子线程去ioc.run()，所以可能会发生子线程没执行，主线程已经结束的现象
        Server s(pool->GetIOContext(), 10086);
        {
            std::unique_lock<std::mutex> lock(_mutex);
            while (!bstop){
                cond_quit.wait(lock);
            }
        }
        
    }
    catch (system::error_code& e)
    {
        std::cout << "error is " << e.what() << std::endl;
    }
}
