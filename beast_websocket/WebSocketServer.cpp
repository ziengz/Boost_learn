#include "WebSocketServer.h"

WebSocketServer::WebSocketServer(asio::io_context&ioc, unsigned short port):_ioc(ioc),_acception(ioc,tcp::endpoint(tcp::v4(),port))
{
	std::cout << "server is opened,port is " << port << std::endl;
}

void WebSocketServer::Start_Accept()
{
	auto connect = std::make_shared<connection>(_ioc);
	_acception.async_accept(connect->GetSocket(), [this, connect](error_code ec) {
		if (!ec) {
			connect->Async_accept();
		}
		else {
			std::cout << "accept failed,exception is " << ec.what() << std::endl;
			return;
		}
		});
}
