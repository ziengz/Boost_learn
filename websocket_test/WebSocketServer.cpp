#include "WebSocketServer.h"

WebSocketServer::WebSocketServer(asio::io_context& ioc, unsigned short port):_ioc(ioc),_acceptor(ioc,tcp::endpoint(tcp::v4(), port))
{

}

void WebSocketServer::Start_Accept()
{
	auto conn = std::make_shared<connection>(_ioc);
	_acceptor.async_accept(conn->GetSocket(), [this,conn](beast::error_code ec) {
		if (!ec) {
			conn->AsyncAcceptor();
		}
		else {
			std::cout << "Accept failed,exception is " << ec.what() << std::endl;
			return;
		}
		});
}
