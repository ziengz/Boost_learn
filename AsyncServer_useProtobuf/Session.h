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
#define MAX_LENGTH 1024*2 //��Ϣ��󳤶�
#define HEAD_LENGTH 2     //ͷ�����ݰ���С
#define MAX_SENDQUE 1000  //��Ϣ�����������

class Server;

class MsgNode
{
	friend class Session;
public:
	//��������Ϣ�Ĺ��캯��
	MsgNode(const char* msg, short max_len) :_max_len(max_len + HEAD_LENGTH),_cur_len(0) {
		_data = new char[_max_len + 1];
		int max_len_network = asio::detail::socket_ops::host_to_network_short(max_len);
		memcpy(_data, &max_len_network, HEAD_LENGTH);
		memcpy(_data + HEAD_LENGTH, msg, max_len);
		_data[_max_len] = '\0';
	}
	//�����ȡ��Ϣ�Ĺ��캯��
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
	int _cur_len;   //�Ѷ�����
	int _max_len; //�ܳ���
	char* _data;
};

class Session:public std::enable_shared_from_this<Session>
{
public:
	Session(asio::io_context& ioc,Server* server) :_socket(ioc),_server(server) {
		uuids::uuid a_uuid = uuids::random_generator()();
		_uuid = uuids::to_string(a_uuid);  //����Ψһid
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

	//�յ���ͷ���ṹ
	std::shared_ptr<MsgNode> _recv_head_node;
	//�ж�ͷ�ڵ��Ƿ������Ϊtrue��˵���Ѿ�������˵����������δ���ꡣ
	//Ϊfalse˵��û��������û������
	bool _b_head_parse; 
	//�յ�����Ϣ��ṹ
	std::shared_ptr<MsgNode> _recv_msg_node;

	bool _send_pending;
	Server* _server;
	std::string _uuid;
	char _data[MAX_LENGTH];
	std::mutex _send_lock;

};

