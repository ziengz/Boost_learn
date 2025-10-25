#pragma once
#include <memory>
#include <boost/asio.hpp>
#include <queue>
#include <iostream>
using namespace boost;
const int RECVSIZE = 1024;

class MsgNode 
{
public:
	//用于写入
	MsgNode(const char* msg, int total_len) :_total_len(total_len),_cur_len(0) {
		_msg = new char[RECVSIZE];
		memcpy(_msg, msg, total_len);
	}
	//用于读取
	MsgNode(int total_len) :_total_len(total_len), _cur_len(0)
	{
		_msg = new char[total_len];
	}
	~MsgNode(){
		delete[]_msg;
	}

	//消息首地址
	char* _msg;
	//总长度
	int _total_len;
	//当前长度
	int _cur_len;
};

class Session
{
public:
	Session(std::shared_ptr<asio::ip::tcp::socket> sock);
	void Connect(const asio::ip::tcp::endpoint& ep);

	//1、未使用队列
	void writeCallBackErr(const system::error_code&ec,std::size_t bytes_transferred,
		std::shared_ptr<MsgNode>);
	void writeToSocketErr(const std::string& buf);

	//2、使用队列
	void writeCallBack(const system::error_code& ec, std::size_t bytes_transferred);
	void writeToSocket(const std::string& buf);

	//3、使用async_send函数一次性发送完，减少调用回调函数次数
	void writeAllCallBack(const system::error_code& ec, std::size_t bytes_transferred);
	void writeAllToSocket(const std::string& buf);

	//读取
	void readFromSocket();
	void readCallBack(const system::error_code&ec,std::size_t bytes_transferred);

	//使用async_receive()读取
	void readAllFromSocket();
	void readAllCallBack(const system::error_code& ec, std::size_t bytes_transferred);


private:
	std::shared_ptr<asio::ip::tcp::socket> _socket;
	std::shared_ptr<MsgNode>_send_node;
	std::queue<std::shared_ptr<MsgNode>>_send_queue; //定义队列
	bool _send_pending; //判断队列中是否有数据，为true则还有数据

	std::shared_ptr<MsgNode>_recv_node;
	bool _recv_pending; //_recv_pending为true表示节点正在接收数据，还未接受完。
};

