#include "Session.h"
#include <json/value.h>
#include <json/reader.h>
#include <json/json.h>
#include "LogicSystem.h"
#include "Server.h"

Session::Session(asio::io_context& ioc, Server* server):
	_socket(ioc),_server(server),_b_head_parse(false),_strand(ioc.get_executor())
{
	uuids::uuid a_uuid = uuids::random_generator()();
	_uuid = uuids::to_string(a_uuid);  //����Ψһid
	_recv_head_node = std::make_shared<MsgNode>(HEAD_TOTAL_LEN);
}

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
		asio::bind_executor(_strand, std::bind(&Session::handle_read, this,
		std::placeholders::_1, std::placeholders::_2,
		shared_from_this()))); //����ʹ��`shared_ptr<Session>(this)������һ���µ�����ָ�룬
		//��Ϊ�����ʹ�����������ָ�����һ�����󣬵�����һ���ͷţ���һ�����þͳ��ֱ���
		//���Ծ��õ�shared_from_this()�������������ü���
	
}

void Session::Send(std::string msg, short msgid)
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
	_send_queue.push(std::make_shared<SendNode>(msg.c_str(), msg.length(), msgid));
	if (_send_pending) //�����������Ȼ�����ݣ��򷵻أ����û������
	{
		return;
	}
	auto& data = _send_queue.front();
	asio::async_write(_socket, asio::buffer(data->_data, data->_max_len),
		asio::bind_executor(_strand,std::bind(&Session::handle_write, this, std::placeholders::_1, shared_from_this())));

}

// ���ͽӿ����жϷ��Ͷ����Ƿ�Ϊ�գ������Ϊ��˵��������δ�����꣬��Ҫ�����ݷ�����У�Ȼ�󷵻ء�
// ������Ͷ���Ϊ�գ���˵����ǰû��δ����������ݣ���Ҫ���͵����ݷ�����в�����async_write�����������ݡ�
void Session::Send(char* msg, int len, short msgid)
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
	_send_queue.push(std::make_shared<SendNode>(msg, len,msgid));
	if (_send_pending) //�����������Ȼ�����ݣ��򷵻أ����û������
	{
		return;
	}
	auto& data = _send_queue.front();
	asio::async_write(_socket, asio::buffer(data->_data, data->_max_len),
		asio::bind_executor(_strand,std::bind(&Session::handle_write, this, std::placeholders::_1, shared_from_this())));

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
				if (byte_transfered + _recv_head_node->_cur_len < HEAD_TOTAL_LEN)
				{
					memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data+copy_len, byte_transfered);
					_recv_head_node->_cur_len += byte_transfered;
					memset(_data, 0, MAX_LENGTH); 
					_socket.async_read_some(asio::buffer(_data, MAX_LENGTH),
						asio::bind_executor(_strand,std::bind(&Session::handle_read, this,
							std::placeholders::_1, std::placeholders::_2, _self_shared)));
					return;
				}
				//�յ����ݱ�ͷ����С��
				int head_remind = HEAD_TOTAL_LEN - _recv_head_node->_cur_len;
				memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, head_remind);
				copy_len += head_remind;
				byte_transfered -= head_remind;
				
				short msg_id = 0;
				memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);
				msg_id = asio::detail::socket_ops::network_to_host_short(msg_id);
				std::cout << "msg id is " << msg_id << std::endl;
				if (msg_id > MAX_LENGTH) {
					std::cout << "invalid msg id is " << msg_id << std::endl;
					_server->ClearServer(_uuid);
					return;
				}
				
				//��ȡͷ������
				short msg_len = 0;
				memcpy(&msg_len, _recv_head_node->_data+HEAD_ID_LEN, HEAD_DATA_LEN);

				//Ϊ��ֹ�ڲ�ͬϵͳ���ֽ������⣬ʹ��network_to_host_short()�����������ֽ���ת��Ϊ�����ֽ���
				msg_len = asio::detail::socket_ops::network_to_host_short(msg_len);
				
				std::cout << "data len is " << msg_len << std::endl;
				if (msg_len > MAX_LENGTH) {
					std::cout << "invalid data length is " << msg_len << std::endl;
					_server->ClearServer(_uuid);
					return;
				}
				_recv_msg_node = std::make_shared<RecvNode>(msg_len,msg_id);

				//��Ϣ�ĳ���С�����ݳ��ȣ�˵������δ���꣬�Ƚ�������Ϣ������սڵ���
				if (byte_transfered < msg_len)
				{
					memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, byte_transfered);
					_recv_msg_node->_cur_len += byte_transfered;
					memset(_data, 0, MAX_LENGTH);
					_socket.async_read_some(asio::buffer(_data, MAX_LENGTH),
						asio::bind_executor(_strand,std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, _self_shared)));
					//ͷ���ڵ��ѽ���					
					_b_head_parse = true;
					return;
				}
				memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data+copy_len, msg_len);
				
				_recv_msg_node->_cur_len += msg_len;
				byte_transfered -= msg_len;
				copy_len += msg_len;

				_recv_msg_node->_data[_recv_msg_node->_max_len] = '\0';
				//std::cout << "�յ����ݣ�" << _recv_msg_node->_data << std::endl;
				//Json::Reader reader;
				//Json::Value root;
				//reader.parse(std::string(_recv_msg_node->_data, _recv_msg_node->_max_len), root);
				//std::cout << "receive msg id is " << root["id"].asInt() << ",data is " << root["data"].asString() << std::endl;
				//root["data"] = "server has receive msg,msg data is " + root["data"].asString();
				//std::string return_str = root.toStyledString();
				//
				////����ȥ������
				//Send(return_str, root["id"].asInt());
				// 

				LogicSystem::GetInstance()->PostMsgToQue(std::make_shared<LogicNode>(shared_from_this(), _recv_msg_node));

				//��Ϣ��������
				_b_head_parse = false;
				_recv_head_node->Clear();
				if (byte_transfered <= 0)
				{
					memset(_data, 0, MAX_LENGTH);
					_socket.async_read_some(asio::buffer(_data, MAX_LENGTH),
						asio::bind_executor(_strand,std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, _self_shared)));
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
					asio::bind_executor(_strand,std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, _self_shared)));
				return;
			}
			memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, remind_msg);
			_recv_msg_node->_cur_len += remind_msg;
			copy_len += remind_msg;
			byte_transfered -= remind_msg;
			

			//std::cout << "�յ���Ϣ��" << _recv_msg_node->_data << std::endl;
			//Json::Reader reader;
			//Json::Value root;
			//reader.parse(std::string(_recv_msg_node->_data, _recv_msg_node->_max_len), root);
			//std::cout << "receive msg id is " << root["id"].asInt() << ",data is " << root["data"].asString() << std::endl;
			//root["data"] = "server has receive msg,msg data is " + root["data"].asString();
			//std::string return_str = root.toStyledString();

			////����ȥ������
			//Send(return_str, root["id"].asInt());

			LogicSystem::GetInstance()->PostMsgToQue(std::make_shared<LogicNode>(shared_from_this(),_recv_msg_node));

			if (byte_transfered <= 0)
			{
				memset(_data, 0, MAX_LENGTH);
				_socket.async_read_some(asio::buffer(_data, MAX_LENGTH),
					asio::bind_executor(_strand,std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, _self_shared)));
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
				asio::bind_executor(_strand,std::bind(&Session::handle_write, this, std::placeholders::_1, _self_shared)));
		}
	}
	else {
		std::cout << "write error : " <<ec.value() << std::endl;
		_server->ClearServer(_uuid);
	}
}

LogicNode::LogicNode(std::shared_ptr<Session> session, std::shared_ptr<RecvNode> recvnode):_session(session),_recvnode(recvnode)
{
}