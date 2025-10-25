#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include <map>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "Session.h"
using namespace boost;

class Session;
class Server {
public:
	Server(asio::io_context& ioc, int port);
	void ClearServer(std::string& uuid);
private:
	void start_accept();
	void handle_accept(std::shared_ptr<Session> new_session, const system::error_code& error);
	asio::io_context& _ioc;
	asio::ip::tcp::acceptor _acceptor;
	std::map<std::string, std::shared_ptr<Session>> _sessions;
};