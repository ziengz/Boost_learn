// websocket_test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "WebSocketServer.h"

int main()
{
    asio::io_context ioc;
    WebSocketServer server(ioc, 10086);
    server.Start_Accept();
    ioc.run();

}
