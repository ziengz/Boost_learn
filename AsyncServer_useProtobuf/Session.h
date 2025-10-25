#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include <map>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "Server.h"
#include <queue>
#include <mutex>
using namespace boost;
#define MAX_LENGTH 1024*2 //消息最大长度
#define HEAD_LENGTH 2     //头部数据包大小
#define MAX_SENDQUE 1000  //消息队列最大容量

class Server;

class MsgNode
{
	friend class Session;
public:
	//处理发送消息的构造函数
	MsgNode(const char* msg, short max_len) :_max_len(max_len + HEAD_LENGTH),_cur_len(0) {
		_data = new char[_max_len + 1];
		int max_len_network = asio::detail::socket_ops::host_to_network_short(max_len);
		memcpy(_data, &max_len_network, HEAD_LENGTH);
		memcpy(_data + HEAD_LENGTH, msg, max_len);
		_data[_max_len] = '\0';
	}
	//处理读取消息的构造函数
	MsgNode(short max_len) :_max_len(max_len), _cur_len(0) {
		_data = new char[max_len + 1];
	}
	~MsgNode(){
		delete[] _data;
	}
	void Clear()
	{
		memset(_data, 0, _max_len);
		_cur_len = 0;
	}
private:
	int _cur_len;   //已读长度
	int _max_len; //总长度
	char* _data;
};

class Session:public std::enable_shared_from_this<Session>
{
public:
	Session(asio::io_context& ioc,Server* server) :_socket(ioc),_server(server) {
		uuids::uuid a_uuid = uuids::random_generator()();
		_uuid = uuids::to_string(a_uuid);  //生成唯一id
		_b_head_parse = false;
	}

	asio::ip::tcp::socket& Socket(){
		return _socket;
	}
	
	std::string& getUuid();
	void start();
private:
	void handle_read(const system::error_code& ec,std::size_t byte_transfered,std::shared_ptr<Session> _self_shared);
	void handle_write(const system::error_code& ec,std::shared_ptr<Session> _self_shared);
	void Send(char* msg, int len);
	void Send(std::string msg);

	asio::ip::tcp::socket _socket;
	std::queue<std::shared_ptr<MsgNode>> _send_queue;

	//收到的头部结构
	std::shared_ptr<MsgNode> _recv_head_node;
	//判断头节点是否解析，为true则说明已经解析，说明还有数据未读完。
	//为false说明没解析或者没有数据
	bool _b_head_parse; 
	//收到的消息体结构
	std::shared_ptr<MsgNode> _recv_msg_node;

	bool _send_pending;
	Server* _server;
	std::string _uuid;
	char _data[MAX_LENGTH];
	std::mutex _send_lock;

};

