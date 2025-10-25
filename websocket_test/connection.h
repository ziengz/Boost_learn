#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/http.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid.hpp>
#include <queue>
#include <mutex>
#include <memory>

using namespace boost;
using namespace boost::beast;
namespace beast = boost::beast;
using tcp = boost::asio::ip::tcp;
using namespace boost::beast::websocket;

class connection:public std::enable_shared_from_this<connection>
{
public:
	connection(asio::io_context& ioc);
	tcp::socket& GetSocket();
	std::string GetUuid();
	void AsyncAcceptor();
	void Start();
	void AsyncSend(std::string data);
private:
	asio::io_context& _ioc;
	std::unique_ptr<stream<tcp_stream>>_ws_ptr;
	std::string _uuid;
	flat_buffer buffer_;
	std::mutex _mutex;
	std::queue<std::string>que_msg;

};