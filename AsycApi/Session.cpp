#include "Session.h"

Session::Session(std::shared_ptr<asio::ip::tcp::socket> socket):_socket(socket), _send_pending(false)
{
	

}

void Session::Connect(const asio::ip::tcp::endpoint& ep)
{
	_socket->connect(ep);
}
//�ڶ������������첽������ֽ���
void Session::writeCallBackErr(const system::error_code& ec, std::size_t bytes_transferred,
	std::shared_ptr<MsgNode> msg_node) //ʹ������ָ��ȷ�����첽�����ڼ����ݲ��ᱻ�ͷ�
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
//���ʹ�����ַ�ʽ�������ݣ�ÿ�η���5���ֽڣ�����ܳ���Ϊ12����һ�η�����׼�����ûص���������ʱ�û��ٴη�������
//�ͻ���ֻ�û���ü���ʣ�����ݷ��ͣ��͵ô������������ݣ���ɴ�������Ϊ������⣬ʹ�ö���
void Session::writeToSocketErr(const std::string& buf)
{
	_send_node = std::make_shared<MsgNode>(buf.c_str(), buf.length());
	//�첽��������
	_socket->async_write_some(asio::buffer(_send_node->_msg, _send_node->_total_len),
		std::bind(&Session::writeCallBackErr,
			this, std::placeholders::_1, std::placeholders::_2, _send_node));

}

//2��ʹ�ö��з�������
void Session::writeCallBack(const system::error_code& ec, std::size_t bytes_transferred)
{
	if (ec.value() != 0)
	{
		std::cout << "Error occured! Error code = "
			<< ec.value()
			<< ". Message: " << ec.message();
		return;
	}
	_send_queue.pop(); //��Ϊ��writeToSocket�������Ѿ�����ͷ�ڵ�
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
	
	//˵����һ����δ�����������
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
	_send_queue.pop(); //��Ϊ��writeToSocket�������Ѿ�����ͷ�ڵ�
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

	//˵����һ����δ�����������
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
