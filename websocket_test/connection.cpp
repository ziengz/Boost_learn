#include "connection.h"
#include "ConnectionMgr.h"

connection::connection(asio::io_context& ioc):_ioc(ioc), _ws_ptr(std::make_unique<stream<tcp_stream>>(make_strand(ioc)))
{
	boost::uuids::random_generator generator;
	boost::uuids::uuid uuid = generator();
	_uuid = boost::uuids::to_string(uuid);
}

tcp::socket& connection::GetSocket()
{
	auto& socket = beast::get_lowest_layer(*_ws_ptr).socket();
	return socket;
}

std::string connection::GetUuid()
{
	return _uuid;
}

void connection::AsyncAcceptor()
{
	auto self = shared_from_this();
	_ws_ptr->async_accept([self](beast::error_code ec) {
		try {
			if (!ec) {
				ConnectionMgr::GetInstance().AddConnection(self);
				self->Start();
			}
			else {
				std::cout << "accept is failed,exception is " << ec.what() << std::endl;
				return;
			}
		}
		catch (std::exception& ec) {
			std::cout << "exception is " << ec.what() << std::endl;
		}
		});
}

void connection::Start()
{
	auto self = shared_from_this();
	_ws_ptr->async_read(buffer_, [self](error_code ec,std::size_t byte_buffer) {
		try {
			if (ec) {
				std::cout << "websocket is failed to accept,exception is " << ec.what() << std::endl;
				ConnectionMgr::GetInstance().RMConnection(self->GetUuid());
				return;
			}
			self->_ws_ptr->text(self->_ws_ptr->got_text());
			std::string data = boost::beast::buffers_to_string(self->buffer_.data());
			self->buffer_.consume(self->buffer_.size());
			std::cout << "recv data is " << data << std::endl;
			self->AsyncSend(data);
			self->Start();    //继续读取数据

		}
		catch (std::exception& ec) {
			std::cout << "exception is " << ec.what() << std::endl;
		}
		});
}

void connection::AsyncSend(std::string data)
{
	{
		std::lock_guard<std::mutex>lock(_mutex);
		std::size_t len = que_msg.size();
		que_msg.push(data);
		if (len > 0) {   //说明队列中还有数据，返回继续接收数据
			return;
		}
	}
	auto self = shared_from_this();
	_ws_ptr->async_write(asio::buffer(data.c_str(), data.length()), 
		[self](error_code ec, std::size_t) {
		try {
			if (ec) {
				std::cout << "async send msg failed,exception is " << ec.what() << std::endl;
				ConnectionMgr::GetInstance().RMConnection(self->GetUuid());
				return;
			}

			std::string send_msg;
			{
				std::lock_guard<std::mutex>lock(self->_mutex);
				self->que_msg.pop();
				if (self->que_msg.empty()) {
					return;
				}
				send_msg = self->que_msg.front();
			}
			self->AsyncSend(std::move(send_msg));
		}
		catch (std::exception& ec) {
			std::cout << "exception is " << ec.what() << std::endl;
			return;
		}
		});
}
