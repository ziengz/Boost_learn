#include <iostream>
#include <boost/asio.hpp>
using namespace boost;
const int MAX_LENGTH = 1024;
const int HEAD_LENGTH = 2;

int main()
{
    try {
        std::string raw_ip_address = "127.0.0.1";
        unsigned short port_num = 10086;
        asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address), port_num);
        asio::io_context ioc;
        asio::ip::tcp::socket sock(ioc,ep.protocol());
        sock.connect(ep);
        
		char request[MAX_LENGTH];
        std::cout<<"Enter message to send: ";
        std::cin.getline(request, MAX_LENGTH);
		int request_length = std::strlen(request);
        char send_data[MAX_LENGTH] = { 0 };
        memcpy(send_data, &request_length, 2);
        memcpy(send_data + 2, request, request_length);
		asio::write(sock, asio::buffer(send_data, request_length+2));

		char reply_head[HEAD_LENGTH];
		int reply_ec = asio::read(sock, asio::buffer(reply_head,HEAD_LENGTH));
        short msglen = 0;
        memcpy(&msglen, reply_head, HEAD_LENGTH);
        char msg[MAX_LENGTH] = { 0 };
        asio::read(sock, asio::buffer(msg, msglen));
        std::cout << "reply is:";
        std::cout.write(msg, msglen) << std::endl;
        std::cout << "reply len is:" << msglen << std::endl;
    }
    catch (system::system_error& e)
    {
        std::cout << "Error occured! Error code = " << e.code()
            << ".Message = " << e.what();
        return e.code().value();
    }
}
