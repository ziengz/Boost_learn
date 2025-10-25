#include "Server.h"

Server::Server(asio::io_context& ioc, int port) :_ioc(ioc),
_acceptor(ioc, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
{
	std::cout << "Server start success ,on port : " << port << std::endl;
	start_accept();
}

void Server::ClearServer(std::string& uuid)
{
	_sessions.erase(uuid);
}

void Server::start_accept()
{
	std::shared_ptr<Session> new_session = std::make_shared<Session>(_ioc, this); //在这里生成临时的new_session
	_acceptor.async_accept(new_session->Socket(),
		std::bind(&Server::handle_accept, this, new_session, std::placeholders::_1)); //然后传递给handle_accept函数
}

void Server::handle_accept(std::shared_ptr<Session> new_session, const system::error_code& error)
{
	if (!error)
	{
		new_session->start();
		_sessions.insert(std::make_pair(new_session->getUuid(), new_session));  //将new_session存储在map中，保证session不会被自动释放
	}
	else {
		std::cout << "session accept failed, error is " << error.what() << std::endl;
	}
	start_accept(); //继续监听
}