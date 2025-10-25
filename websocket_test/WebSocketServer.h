#pragma once
#include <boost/asio.hpp>
#include "connection.h"
using namespace boost;
class WebSocketServer
{
public:
	WebSocketServer(asio::io_context& ioc,unsigned short port);
	WebSocketServer& operator=(const WebSocketServer&) = delete;
	WebSocketServer(const WebSocketServer&) = delete;
	void Start_Accept();
private:
	tcp::acceptor _acceptor;
	asio::io_context& _ioc;
};

