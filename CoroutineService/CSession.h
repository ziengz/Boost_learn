#pragma once
#include <boost/asio.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <queue>
#include <mutex>
#include <memory>
#include "MsgNode.h"


using namespace boost;
using boost::asio::ip::tcp;
using boost::asio::co_spawn;
using boost::asio::awaitable;
using boost::asio::detached;
using boost::asio::use_awaitable;

class CServer;
class LogicSystem;

class CSession:public std::enable_shared_from_this<CSession>
{
public:
	CSession(asio::io_context& ioc, CServer* server);
	tcp::socket& GetSocket();
	std::string& GetUuid();
	void start();
	void HandleWrite(const system::error_code& error, std::shared_ptr<CSession> shared_self);
	void Send(const char* data, short len, short msg_id);
	void Send(std::string data, short msg_id);
	void Close();
private:
	std::string _uuid;
	tcp::socket _socket;
	CServer* _server;
	std::shared_ptr<MsgNode>_recv_head_node;
	std::queue<std::shared_ptr<SendNode>>_send_que;
	std::shared_ptr<RecvNode>_recv_msg_node;
	bool _b_close;
	asio::io_context& _ioc;
	std::mutex _mutex;
	
};

class LogicNode {
	friend class LogicSystem;
public:
	LogicNode(std::shared_ptr<CSession>, std::shared_ptr<RecvNode>);
private:
	std::shared_ptr<RecvNode>_recv_node;
	std::shared_ptr<CSession>_session;
};
