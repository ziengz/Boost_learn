#pragma once
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid.hpp>
#include <memory>
#include <mutex>
#include <queue>

using namespace boost::beast;
using namespace boost;
using tcp = boost::asio::ip::tcp;
using namespace boost::beast::websocket;

class connection:public std::enable_shared_from_this<connection>
{
public:
	connection(asio::io_context& ioc);
	std::string GetUuid();
	tcp::socket& GetSocket();
	void Async_accept();
	void start();
	void AsyncSend(std::string);


private:
	asio::io_context& _ioc;
	std::string _uuid;
	std::unique_ptr<stream<tcp_stream>> _ws_ptr;
	flat_buffer buffer_;
	std::mutex _send_mutex;
	std::queue<std::string> que_msg;

};

