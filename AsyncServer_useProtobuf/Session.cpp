#include "Session.h"
#include "msg.pb.h"

std::string& Session::getUuid()
{
	return _uuid;
}
//构造伪闭包
//思路：
//1、利用智能指针被复制或使用引用计数加一的原理保证内存不被回收
//2、bind操作可以将值绑定在一个函数对象上生成新的函数对象，
//   如果将智能指针作为参数绑定给函数对象，那么智能指针就以值的方式被新函数对象使用，
//   那么智能指针的生命周期将和新生成的函数对象一致，从而达到延长生命的效果。

void Session::start()
{
	
	memset(_data, 0, MAX_LENGTH);
	_socket.async_read_some(asio::buffer(_data, MAX_LENGTH),
		std::bind(&Session::handle_read, this,
		std::placeholders::_1, std::placeholders::_2,
		shared_from_this())); //不能使用`shared_ptr<Session>(this)`创建了一个新的智能指针，
		//因为这样就代表两个智能指针管理一个对象，当其中一个释放，另一个调用就出现崩溃
		//所以就用到shared_from_this()函数，共享引用计数
	_recv_head_node = std::make_shared<MsgNode>(HEAD_LENGTH);
}
void Session::Send(std::string  msg)
{
	_send_pending = false;
	std::lock_guard<std::mutex> lock(_send_lock); //如果不锁，如果在一条消息没有全部发送出去，新来一条消息会出现错误
	int send_que_size = _send_queue.size();
	if (send_que_size > MAX_SENDQUE)
	{
		std::cout << "session: " << _uuid << " send que fulled, size is " << MAX_SENDQUE << std::endl;
		return;
	}

	if (_send_queue.size() > 0)  //如果有数据未发完
	{
		_send_pending = true;
	}
	_send_queue.push(std::make_shared<MsgNode>(msg.c_str(), msg.length()));
	if (_send_pending) //如果队列中依然有数据，则返回，如果没有则发送
	{
		return;
	}
	auto& data = _send_queue.front();
	asio::async_write(_socket, asio::buffer(data->_data, data->_max_len),
		std::bind(&Session::handle_write, this, std::placeholders::_1, shared_from_this()));

}

// 发送接口里判断发送队列是否为空，如果不为空说明有数据未发送完，需要将数据放入队列，然后返回。
// 如果发送队列为空，则说明当前没有未发送完的数据，将要发送的数据放入队列并调用async_write函数发送数据。
void Session::Send(char* msg, int len)
{
	_send_pending = false;
	std::lock_guard<std::mutex> lock(_send_lock); //如果不锁，如果在一条消息没有全部发送出去，新来一条消息会出现错误
	int send_que_size = _send_queue.size();
	if (send_que_size > MAX_SENDQUE)
	{
		std::cout << "session: " << _uuid << " send que fulled, size is " << MAX_SENDQUE << std::endl;
		return;
	}
	
	if (_send_queue.size() > 0)  //如果有数据未发完
	{
		_send_pending = true;
	}
	_send_queue.push(std::make_shared<MsgNode>(msg, len));
	if (_send_pending) //如果队列中依然有数据，则返回，如果没有则发送
	{
		return;
	}
	auto& data = _send_queue.front();
	asio::async_write(_socket, asio::buffer(data->_data, data->_max_len),
		std::bind(&Session::handle_write, this, std::placeholders::_1, shared_from_this()));

}

void Session::handle_read(const system::error_code& ec, std::size_t byte_transfered,
	 std::shared_ptr<Session> _self_shared)
{
	if (!ec)
	{
		//已经移动的字符数
		int copy_len = 0;
		while (byte_transfered > 0)
		{
			if (!_b_head_parse)
			{
				//如果收到的数据不足头部大小
				if (byte_transfered + _recv_head_node->_cur_len < HEAD_LENGTH)
				{
					memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data+copy_len, byte_transfered);
					_recv_head_node->_cur_len += byte_transfered;
					memset(_data, 0, MAX_LENGTH);
					_socket.async_read_some(asio::buffer(_data, MAX_LENGTH),
						std::bind(&Session::handle_read, this,
							std::placeholders::_1, std::placeholders::_2, _self_shared));
					return;
				}
				//收到数据比头部大小多
				int head_remind = HEAD_LENGTH - _recv_head_node->_cur_len;
				memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, head_remind);
				copy_len += head_remind;
				byte_transfered -= head_remind;
				//获取头部数据
				short data_len = 0;
				memcpy(&data_len, _recv_head_node->_data, HEAD_LENGTH);

				//为防止在不同系统中字节序问题，使用network_to_host_short()函数将网络字节序转换为主机字节序
				data_len = asio::detail::socket_ops::network_to_host_short(data_len);
				
				std::cout << "data len is " << data_len << std::endl;
				if (data_len > MAX_LENGTH) {
					std::cout << "data length is " << data_len << std::endl;
					_server->ClearServer(_uuid);
					return;
				}

				_recv_msg_node = std::make_shared<MsgNode>(data_len);

				//消息的长度小于头部规定的长度，说明数据未收全，则先将部分消息放到接收节点里
				if (byte_transfered < data_len) {
					memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, byte_transfered);
					_recv_msg_node->_cur_len += byte_transfered;
					::memset(_data, 0, MAX_LENGTH);
					_socket.async_read_some(asio::buffer(_data, MAX_LENGTH),
						std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
					//头部处理完成
					_b_head_parse = true;
					return;
				}
				memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data+copy_len, data_len);
				
				_recv_msg_node->_cur_len += data_len;
				byte_transfered -= data_len;
				copy_len += data_len;

				_recv_msg_node->_data[_recv_msg_node->_max_len] = '\0';
				//std::cout << "收到数据：" << _recv_msg_node->_data << std::endl;
				//发回去做测试，使用protobuf发送
				std::cout << "1";
				MsgData msgdata;
				std::string receive_data;
				msgdata.ParseFromString(std::string(_recv_msg_node->_data, _recv_msg_node->_max_len));
				std::cout << "recevie msg id  is " << msgdata.id() << " msg data is " << msgdata.data() << std::endl;
				std::string return_str = "server has received msg, msg data is " + msgdata.data();
				MsgData msgreturn;
				msgreturn.set_id(msgdata.id());
				msgreturn.set_data(return_str);
				msgreturn.SerializeToString(&return_str);
				Send(return_str);

				//消息处理完了
				_b_head_parse = false;
				_recv_head_node->Clear();
				if (byte_transfered <= 0)
				{
					memset(_data, 0, MAX_LENGTH);
					_socket.async_read_some(asio::buffer(_data, MAX_LENGTH),
						std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
					return;
				}
				continue;
			}
			//如果头部已经解析了，处理上次未接收完的数据
			//计算消息体所需要的长度
			int remind_msg = _recv_msg_node->_max_len - _recv_msg_node->_cur_len;
			if (byte_transfered < remind_msg)
			{
				memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, byte_transfered);
				_recv_msg_node->_cur_len += byte_transfered;
				memset(_data, 0, MAX_LENGTH);
				_socket.async_read_some(asio::buffer(_data, MAX_LENGTH),
					std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
				return;
			}
			memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, remind_msg);
			_recv_msg_node->_cur_len += remind_msg;
			copy_len += remind_msg;
			byte_transfered -= remind_msg;
			

			std::cout << "收到消息：" << _recv_msg_node->_data << std::endl;
			Send(_recv_msg_node->_data, _recv_msg_node->_max_len);
			if (byte_transfered <= 0)
			{
				memset(_data, 0, MAX_LENGTH);
				_socket.async_read_some(asio::buffer(_data, MAX_LENGTH),
					std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
				return;
			}
			continue;

		}


	}
	else {
		std::cout << "read error : " << ec.value() << std::endl;
		_server->ClearServer(_uuid);
	}
}

void Session::handle_write(const system::error_code& ec, std::shared_ptr<Session> _self_shared)
{
	if (!ec)
	{
		std::lock_guard<std::mutex> lock(_send_lock);
		_send_queue.pop(); //因为在Send()函数中头节点发送了，
		if (_send_queue.size() > 0)
		{
			auto& data = _send_queue.front();
			asio::async_write(_socket, asio::buffer(data->_data, data->_max_len),
				std::bind(&Session::handle_write, this, std::placeholders::_1, _self_shared));
		}
	}
	else {
		std::cout << "write error : " <<ec.value() << std::endl;
		_server->ClearServer(_uuid);
	}
}


