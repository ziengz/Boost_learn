// JsonClient.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <boost/asio.hpp>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
using namespace boost;
#define MAX_LENGTH 2048
#define HEAD_LENGTH 2
#define HEAD_TOTAL_LEN 4

int main()
{
	try {
		asio::io_context ioc;
		asio::ip::tcp::endpoint remote_ep(asio::ip::make_address("127.0.0.1"), 10086);
		asio::ip::tcp::socket sock(ioc);
		system::error_code ec = asio::error::host_not_found;
		sock.connect(remote_ep,ec);

		if (ec)
		{
			std::cout << "connect failed code is " << ec.what() << "error msg is " << ec.message() << std::endl;
			return 0;
		}

		Json::Value root;
		root["id"] = 1001;
		root["data"] = "hello world!";
		std::string request = root.toStyledString();
		short request_length = request.length();
		char send_data[MAX_LENGTH] = {0};
		short msg_id = 1001;
		short msg_id_host = asio::detail::socket_ops::host_to_network_short(msg_id);

		short request_host_length = asio::detail::socket_ops::host_to_network_short(request_length);
		memcpy(send_data, &msg_id_host, 2);
		memcpy(send_data + 2, &request_host_length, 2);
		memcpy(send_data + HEAD_TOTAL_LEN, request.c_str(), request_length);
		asio::write(sock, asio::buffer(send_data, request_length + 4));
		std::cout << "send msg success" << std::endl;
		
		std::cout << "begin receive msg..." << std::endl;
		char reply_head[HEAD_TOTAL_LEN];
		size_t reply_ec = asio::read(sock, asio::buffer(reply_head, HEAD_TOTAL_LEN));
		msg_id = 0;
		memcpy(&msg_id, reply_head, HEAD_LENGTH);
		short msg_len = 0;
		memcpy(&msg_len, reply_head + 2, HEAD_LENGTH);
		msg_id = asio::detail::socket_ops::network_to_host_short(msg_id);
		msg_len = asio::detail::socket_ops::network_to_host_short(msg_len);

		char msg[MAX_LENGTH] = { 0 };
		size_t msg_length = asio::read(sock, asio::buffer(msg, msg_len));
		Json::Reader reader;
		reader.parse(std::string(msg, msg_len), root);
		std::cout << "msg id is " << root["id"].asInt() << ",receive msg is " << root["data"].asString() << std::endl;
		getchar();
	}
	catch (system::error_code& ec){
		std::cout << "Error occured! Error value = " << ec.value()
			<< ".Message = " << ec.what();
		return ec.value();
	}
}