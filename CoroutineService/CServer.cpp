#include "CServer.h"

CServer::CServer(asio::io_context& ioc, short port):_ioc(ioc),_acceptor(ioc,tcp::endpoint(tcp::v4(),port))
{
	std::cout << "Server start success ,on port : " << port << std::endl;
	start_accept();
}

void CServer::ClearServer(std::string& uuid)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_sessions.erase(uuid);
}

void CServer::start_accept()
{
	std::shared_ptr<CSession>new_session = std::make_shared<CSession>(_ioc, this);
	_acceptor.async_accept(new_session->GetSocket(),
		std::bind(&CServer::handle_accept, this, new_session, std::placeholders::_1));
}

void CServer::handle_accept(std::shared_ptr<CSession> new_session, const system::error_code& error)
{
	if (!error)
	{
		new_session->start();
		_sessions.insert(std::make_pair(new_session->GetUuid(), new_session));
	}
	else {
		std::cout << "session accept failed, error is " << error.what() << std::endl;
	}
	start_accept();
}

