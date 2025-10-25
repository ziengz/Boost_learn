#include "Session.h"
#include "msg.pb.h"

std::string& Session::getUuid()
{
	return _uuid;
}
//����α�հ�
//˼·��
//1����������ָ�뱻���ƻ�ʹ�����ü�����һ��ԭ��֤�ڴ治������
//2��bind�������Խ�ֵ����һ�����������������µĺ�������
//   ���������ָ����Ϊ�����󶨸�����������ô����ָ�����ֵ�ķ�ʽ���º�������ʹ�ã�
//   ��ô����ָ����������ڽ��������ɵĺ�������һ�£��Ӷ��ﵽ�ӳ�������Ч����

void Session::start()
{
	
	memset(_data, 0, MAX_LENGTH);
	_socket.async_read_some(asio::buffer(_data, MAX_LENGTH),
		std::bind(&Session::handle_read, this,
		std::placeholders::_1, std::placeholders::_2,
		shared_from_this())); //����ʹ��`shared_ptr<Session>(this)`������һ���µ�����ָ�룬
		//��Ϊ�����ʹ�����������ָ�����һ�����󣬵�����һ���ͷţ���һ�����þͳ��ֱ���
		//���Ծ��õ�shared_from_this()�������������ü���
	_recv_head_node = std::make_shared<MsgNode>(HEAD_LENGTH);
}
void Session::Send(std::string  msg)
{
	_send_pending = false;
	std::lock_guard<std::mutex> lock(_send_lock); //��������������һ����Ϣû��ȫ�����ͳ�ȥ������һ����Ϣ����ִ���
	int send_que_size = _send_queue.size();
	if (send_que_size > MAX_SENDQUE)
	{
		std::cout << "session: " << _uuid << " send que fulled, size is " << MAX_SENDQUE << std::endl;
		return;
	}

	if (_send_queue.size() > 0)  //���������δ����
	{
		_send_pending = true;
	}
	_send_queue.push(std::make_shared<MsgNode>(msg.c_str(), msg.length()));
	if (_send_pending) //�����������Ȼ�����ݣ��򷵻أ����û������
	{
		return;
	}
	auto& data = _send_queue.front();
	asio::async_write(_socket, asio::buffer(data->_data, data->_max_len),
		std::bind(&Session::handle_write, this, std::placeholders::_1, shared_from_this()));

}

// ���ͽӿ����жϷ��Ͷ����Ƿ�Ϊ�գ������Ϊ��˵��������δ�����꣬��Ҫ�����ݷ�����У�Ȼ�󷵻ء�
// ������Ͷ���Ϊ�գ���˵����ǰû��δ����������ݣ���Ҫ���͵����ݷ�����в�����async_write�����������ݡ�
void Session::Send(char* msg, int len)
{
	_send_pending = false;
	std::lock_guard<std::mutex> lock(_send_lock); //��������������һ����Ϣû��ȫ�����ͳ�ȥ������һ����Ϣ����ִ���
	int send_que_size = _send_queue.size();
	if (send_que_size > MAX_SENDQUE)
	{
		std::cout << "session: " << _uuid << " send que fulled, size is " << MAX_SENDQUE << std::endl;
		return;
	}
	
	if (_send_queue.size() > 0)  //���������δ����
	{
		_send_pending = true;
	}
	_send_queue.push(std::make_shared<MsgNode>(msg, len));
	if (_send_pending) //�����������Ȼ�����ݣ��򷵻أ����û������
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
		//�Ѿ��ƶ����ַ���
		int copy_len = 0;
		while (byte_transfered > 0)
		{
			if (!_b_head_parse)
			{
				//����յ������ݲ���ͷ����С
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
				//�յ����ݱ�ͷ����С��
				int head_remind = HEAD_LENGTH - _recv_head_node->_cur_len;
				memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, head_remind);
				copy_len += head_remind;
				byte_transfered -= head_remind;
				//��ȡͷ������
				short data_len = 0;
				memcpy(&data_len, _recv_head_node->_data, HEAD_LENGTH);

				//Ϊ��ֹ�ڲ�ͬϵͳ���ֽ������⣬ʹ��network_to_host_short()�����������ֽ���ת��Ϊ�����ֽ���
				data_len = asio::detail::socket_ops::network_to_host_short(data_len);
				
				std::cout << "data len is " << data_len << std::endl;
				if (data_len > MAX_LENGTH) {
					std::cout << "data length is " << data_len << std::endl;
					_server->ClearServer(_uuid);
					return;
				}

				_recv_msg_node = std::make_shared<MsgNode>(data_len);

				//��Ϣ�ĳ���С��ͷ���涨�ĳ��ȣ�˵������δ��ȫ�����Ƚ�������Ϣ�ŵ����սڵ���
				if (byte_transfered < data_len) {
					memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, byte_transfered);
					_recv_msg_node->_cur_len += byte_transfered;
					::memset(_data, 0, MAX_LENGTH);
					_socket.async_read_some(asio::buffer(_data, MAX_LENGTH),
						std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
					//ͷ���������
					_b_head_parse = true;
					return;
				}
				memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data+copy_len, data_len);
				
				_recv_msg_node->_cur_len += data_len;
				byte_transfered -= data_len;
				copy_len += data_len;

				_recv_msg_node->_data[_recv_msg_node->_max_len] = '\0';
				//std::cout << "�յ����ݣ�" << _recv_msg_node->_data << std::endl;
				//����ȥ�����ԣ�ʹ��protobuf����
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

				//��Ϣ��������
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
			//���ͷ���Ѿ������ˣ������ϴ�δ�����������
			//������Ϣ������Ҫ�ĳ���
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
			

			std::cout << "�յ���Ϣ��" << _recv_msg_node->_data << std::endl;
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
		_send_queue.pop(); //��Ϊ��Send()������ͷ�ڵ㷢���ˣ�
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


