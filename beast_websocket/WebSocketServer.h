#pragma once 
#include "connection.h"
#include <iostream>

class WebSocketServer
{
public:
	WebSocketServer (const WebSocketServer &) = delete;
	WebSocketServer& operator=(const WebSocketServer&) = delete;
	WebSocketServer(asio::io_context&ioc, unsigned short port);
	void Start_Accept();

private:
	tcp::acceptor _acception;
	asio::io_context& _ioc;

};

