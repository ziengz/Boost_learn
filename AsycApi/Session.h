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
	//����д��
	MsgNode(const char* msg, int total_len) :_total_len(total_len),_cur_len(0) {
		_msg = new char[RECVSIZE];
		memcpy(_msg, msg, total_len);
	}
	//���ڶ�ȡ
	MsgNode(int total_len) :_total_len(total_len), _cur_len(0)
	{
		_msg = new char[total_len];
	}
	~MsgNode(){
		delete[]_msg;
	}

	//��Ϣ�׵�ַ
	char* _msg;
	//�ܳ���
	int _total_len;
	//��ǰ����
	int _cur_len;
};

class Session
{
public:
	Session(std::shared_ptr<asio::ip::tcp::socket> sock);
	void Connect(const asio::ip::tcp::endpoint& ep);

	//1��δʹ�ö���
	void writeCallBackErr(const system::error_code&ec,std::size_t bytes_transferred,
		std::shared_ptr<MsgNode>);
	void writeToSocketErr(const std::string& buf);

	//2��ʹ�ö���
	void writeCallBack(const system::error_code& ec, std::size_t bytes_transferred);
	void writeToSocket(const std::string& buf);

	//3��ʹ��async_send����һ���Է����꣬���ٵ��ûص���������
	void writeAllCallBack(const system::error_code& ec, std::size_t bytes_transferred);
	void writeAllToSocket(const std::string& buf);

	//��ȡ
	void readFromSocket();
	void readCallBack(const system::error_code&ec,std::size_t bytes_transferred);

	//ʹ��async_receive()��ȡ
	void readAllFromSocket();
	void readAllCallBack(const system::error_code& ec, std::size_t bytes_transferred);


private:
	std::shared_ptr<asio::ip::tcp::socket> _socket;
	std::shared_ptr<MsgNode>_send_node;
	std::queue<std::shared_ptr<MsgNode>>_send_queue; //�������
	bool _send_pending; //�ж϶������Ƿ������ݣ�Ϊtrue��������

	std::shared_ptr<MsgNode>_recv_node;
	bool _recv_pending; //_recv_pendingΪtrue��ʾ�ڵ����ڽ������ݣ���δ�����ꡣ
};

