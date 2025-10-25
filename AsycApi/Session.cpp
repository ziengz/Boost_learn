#include "Session.h"

Session::Session(std::shared_ptr<asio::ip::tcp::socket> socket):_socket(socket), _send_pending(false)
{
	

}

void Session::Connect(const asio::ip::tcp::endpoint& ep)
{
	_socket->connect(ep);
}
//第二个参数本次异步传输的字节数
void Session::writeCallBackErr(const system::error_code& ec, std::size_t bytes_transferred,
	std::shared_ptr<MsgNode> msg_node) //使用智能指针确保在异步操作期间数据不会被释放
{
	if (bytes_transferred + msg_node->_cur_len < msg_node->_total_len)
	{
		_send_node->_cur_len += bytes_transferred;
		_socket->async_write_some(asio::buffer(_send_node->_msg + _send_node->_cur_len
			, _send_node->_total_len - _send_node->_cur_len),
			std::bind(&Session::writeCallBackErr,
				this, std::placeholders::_1, std::placeholders::_2, _send_node));
	}
}
//如果使用这种方式发送数据，每次发送5个字节，如果总长度为12，第一次发送完准备调用回调函数，这时用户再次发送数据
//就会出现还没来得及把剩余数据发送，就得处理新来的数据，造成错误，所以为解决问题，使用队列
void Session::writeToSocketErr(const std::string& buf)
{
	_send_node = std::make_shared<MsgNode>(buf.c_str(), buf.length());
	//异步发送数据
	_socket->async_write_some(asio::buffer(_send_node->_msg, _send_node->_total_len),
		std::bind(&Session::writeCallBackErr,
			this, std::placeholders::_1, std::placeholders::_2, _send_node));

}

//2、使用队列发送数据
void Session::writeCallBack(const system::error_code& ec, std::size_t bytes_transferred)
{
	if (ec.value() != 0)
	{
		std::cout << "Error occured! Error code = "
			<< ec.value()
			<< ". Message: " << ec.message();
		return;
	}
	_send_queue.pop(); //因为在writeToSocket函数中已经传过头节点
	if (_send_queue.empty())
	{
		_send_pending = false;
		return;
	}
	if (!_send_queue.empty())
	{
		auto& send_data = _send_queue.front();
		_socket->async_write_some(asio::buffer(send_data->_msg+send_data->_cur_len,send_data->_total_len-send_data->_cur_len)
			, std::bind(&Session::writeCallBack,
			this, std::placeholders::_1,std::placeholders::_2));
	}
}

void Session::writeToSocket(const std::string& buf)
{
	_send_queue.emplace(new MsgNode(buf.c_str(), buf.length()));
	
	//说明上一次犹未发送完的数据
	if (_send_pending)
	{
		return;
	}
	_socket->async_write_some(asio::buffer(buf), std::bind(&Session::writeCallBack,
		this, std::placeholders::_1, std::placeholders::_2));
	_send_pending = false;
}

void Session::writeAllCallBack(const system::error_code& ec, std::size_t bytes_transferred)
{
	if (ec.value() != 0)
	{
		std::cout << "Error occured! Error code = "
			<< ec.value()
			<< ". Message: " << ec.message();
		return;
	}
	_send_queue.pop(); //因为在writeToSocket函数中已经传过头节点
	if (_send_queue.empty())
	{
		_send_pending = false;
		return;
	}
	if (!_send_queue.empty())
	{
		auto& send_data = _send_queue.front();
		_socket->async_send(asio::buffer(send_data->_msg + send_data->_cur_len, send_data->_total_len - send_data->_cur_len)
			, std::bind(&Session::writeCallBack,
				this, std::placeholders::_1, std::placeholders::_2));
	}
}

void Session::writeAllToSocket(const std::string& buf)
{
	_send_queue.emplace(new MsgNode(buf.c_str(), buf.length()));

	//说明上一次犹未发送完的数据
	if (_send_pending)
	{
		return;
	}
	_socket->async_send(asio::buffer(buf), std::bind(&Session::writeCallBack,
		this, std::placeholders::_1, std::placeholders::_2));
	_send_pending = false;
}

void Session::readFromSocket()
{
	if (_recv_pending)
	{
		return;
	}
	_recv_node = std::make_shared<MsgNode>(RECVSIZE);
	_socket->async_read_some(asio::buffer(_recv_node->_msg, _recv_node->_total_len),
		std::bind(&Session::readCallBack, this, std::placeholders::_1, std::placeholders::_2));
	_recv_pending = true;
}

void Session::readCallBack(const system::error_code& ec, std::size_t bytes_transferred)
{
	_recv_node->_cur_len += bytes_transferred;
	if (_recv_node->_cur_len < _recv_node->_total_len)
	{
		_socket->async_read_some(asio::buffer(_recv_node->_msg + _recv_node->_cur_len,
			_recv_node->_total_len - _recv_node->_cur_len),
			std::bind(&Session::readCallBack, this, std::placeholders::_1, std::placeholders::_2));
	}
	_recv_pending = false;
	_recv_node = nullptr;
}

void Session::readAllFromSocket()
{
	if (_recv_pending) {
		return;
	}
	_recv_node = std::make_shared<MsgNode>(RECVSIZE);
	_socket->async_receive(asio::buffer(_recv_node->_msg, _recv_node->_total_len),
		std::bind(&Session::readAllCallBack, this, std::placeholders::_1, std::placeholders::_2));
	_recv_pending = true;
}

void Session::readAllCallBack(const system::error_code& ec, std::size_t bytes_transferred)
{
	_recv_node->_cur_len += bytes_transferred;
	if (_recv_node->_cur_len < _recv_node->_total_len)
	{
		_socket->async_receive(asio::buffer(_recv_node->_msg + _recv_node->_cur_len,
			_recv_node->_total_len - _recv_node->_cur_len),
			std::bind(&Session::readAllCallBack, this, std::placeholders::_1, std::placeholders::_2));
		_recv_pending = false;
		_recv_node = nullptr;
	}
}
