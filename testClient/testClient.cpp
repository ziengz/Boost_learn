// testClient.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <boost/asio.hpp>
using namespace std;
using namespace boost;
const int MAX_LENGTH = 1024;

int main()
{
    try {
        asio::io_context ioc;
        asio::ip::tcp::endpoint remote_ep(asio::ip::make_address("127.0.0.1"), 10086);
        asio::ip::tcp::socket socket(ioc);
        system::error_code ec = asio::error::host_not_found;
        socket.connect(remote_ep,ec);
        if (ec) {
            cout << "connect failed,code is " << ec.value() << "error msg is " << ec.what() << endl;
            return 0;
        }
        char request[MAX_LENGTH];
        cout << "Enter message:";
        cin.getline(request,MAX_LENGTH);
        size_t request_length = strlen(request);
        asio::write(socket,asio::buffer(request, request_length));

        char reply[MAX_LENGTH] = { 0 };
        size_t reply_length = asio::read(socket, asio::buffer(reply, request_length));
        cout << "reply is " << string(reply, reply_length) << endl;
        getchar();

    }
    catch (std::exception& e){
        cout << "exception is " << e.what() << endl;
    }
    
    return 0;

}