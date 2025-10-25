#pragma once
#include <boost/asio.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <queue>
#include <mutex>
#include <memory>
#include "const.h"
#include "MsgNode.h"
using namespace boost;

class Server;
class LogicSystem;

class Session:public std::enable_shared_from_this<Session>
{
public:
	Session(asio::io_context& ioc, Server* server);

	asio::ip::tcp::socket& Socket(){
		return _socket;
	}
	
	std::string& getUuid();
	void start();
	void Send(char* msg, int len, short msgid);
	void Send(std::string msg, short msgid);
private:
	void handle_read(const system::error_code& ec,std::size_t byte_transfered,std::shared_ptr<Session> _self_shared);
	void handle_write(const system::error_code& ec,std::shared_ptr<Session> _self_shared);


	asio::ip::tcp::socket _socket;
	std::queue<std::shared_ptr<SendNode>> _send_queue;

	//�յ���ͷ���ṹ
	std::shared_ptr<MsgNode> _recv_head_node;
	//�ж�ͷ�ڵ��Ƿ������Ϊtrue��˵���Ѿ�������˵����������δ���ꡣ
	//Ϊfalse˵��û��������û������
	bool _b_head_parse; 
	//�յ�����Ϣ��ṹ
	std::shared_ptr<RecvNode> _recv_msg_node;

	bool _send_pending;
	Server* _server;
	std::string _uuid;
	char _data[MAX_LENGTH];
	std::mutex _send_lock;
	//Ϊ������̴߳������ú��������ʹ�ô�����strand��װ
    asio::strand<boost::asio::io_context::executor_type> _strand;
};

class LogicNode
{
	friend class LogicSystem;
public:
	LogicNode(std::shared_ptr<Session>, std::shared_ptr<RecvNode>);
private:
	std::shared_ptr<RecvNode>_recvnode;
	std::shared_ptr<Session>_session;
};
