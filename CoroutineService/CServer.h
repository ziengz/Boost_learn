#pragma once
#include <boost/asio.hpp>
#include <mutex>
#include <map>
#include <thread>
#include <iostream>
#include <memory>
#include "CSession.h"

using boost::asio::ip::tcp;
using namespace boost;

class CServer
{
public:
	CServer(asio::io_context&ioc, short port);
	void ClearServer(std::string& uuid);

private:
	void start_accept();
	void handle_accept(std::shared_ptr<CSession> new_session, const system::error_code& error);
	asio::io_context& _ioc;
	tcp::acceptor _acceptor;
	std::map<std::string, std::shared_ptr<CSession>> _sessions;
	std::mutex _mutex;

};

