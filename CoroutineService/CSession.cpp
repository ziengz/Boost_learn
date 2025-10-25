#include "CSession.h"
#include "CServer.h"
#include "LogicSystem.h"

CSession::CSession(asio::io_context& ioc, CServer* server):
	_ioc(ioc),_socket(ioc),_server(server),_b_close(false)
{
	uuids::uuid a_uuid = uuids::random_generator()();
	_uuid = boost::uuids::to_string(a_uuid);
	_recv_head_node = std::make_shared<MsgNode>(HEAD_TOTAL_LEN);
}

tcp::socket& CSession::GetSocket()
{
	return _socket;
}

std::string& CSession::GetUuid()
{
	return _uuid;
}

void CSession::start()
{
	auto shared_this = shared_from_this();
	//开启协程
	co_spawn(_ioc, [shared_this,this]()->awaitable<void> {
		try {
			for (; !_b_close;)
			{
				_recv_head_node->Clear();
				if (!_b_close)
				{
					std::size_t n = co_await asio::async_read(_socket, asio::buffer(_recv_head_node->_data, _recv_head_node->_total_len),
						use_awaitable);
					if (n == 0) {
						std::cout << "receive peer is closed" << std::endl;
						_server->ClearServer(_uuid);
						Close();
						co_return;
					}
					short msg_id = 0;
					memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LENGTH);
					msg_id = asio::detail::socket_ops::network_to_host_short(msg_id);
					std::cout << "msg id is " << msg_id << std::endl;
					
					short msg_len = 0;
					memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LENGTH, HEAD_DATA_LEN);
					msg_len = asio::detail::socket_ops::network_to_host_short(msg_len);
					if (msg_len > MAX_LENGTH) {
						std::cout << "invalid msg_id is " << msg_id << std::endl;
						_server->ClearServer(_uuid);
						co_return;
					}

					_recv_msg_node = std::make_shared<RecvNode>(msg_len,msg_id);
					n = co_await asio::async_read(_socket, asio::buffer(_recv_msg_node->_data,_recv_msg_node->_total_len), use_awaitable);
					if (n == 0)
					{
						std::cout << "receive peer is closed" << std::endl;
						_server->ClearServer(_uuid);
						Close();
						co_return;
					}

					_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
					std::cout << "recv data is " << _recv_msg_node->_data << std::endl;
					//使用逻辑系统打印出来
					LogicSystem::GetInstance().PostMsgToQue(std::make_shared<LogicNode>(shared_this, _recv_msg_node));

				}
				else {
					std::cout << "receive peer is closed" << std::endl;
					_server->ClearServer(_uuid);
					Close();
					co_return;
				}
			}
		}
		catch (std::exception& e) {
			std::cout << "exception is " << e.what() << std::endl;
			Close();
			_server->ClearServer(_uuid);
		}
		},detached);
	
}

void CSession::HandleWrite(const system::error_code& error, std::shared_ptr<CSession> shared_self)
{
	try {
		if (!error) {
			std::unique_lock<std::mutex>lock(_mutex);
			_send_que.pop();        //因为send函数已经发送头包
			if (!_send_que.empty()) {
				auto msgnode = _send_que.front();
				asio::async_write(_socket, asio::buffer(msgnode->_data, msgnode->_total_len),
					std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_self));
			}
		}
		else {
			std::cout << "handle write is failed,error is " << error.what() << std::endl;
			Close();
			_server->ClearServer(_uuid);
		}
	}
	catch (std::exception& e) {
		std::cout << "exception is:" << e.what() << std::endl;
		return;
	}
}

void CSession::Send(const char* data, short len, short msg_id)
{
	std::unique_lock<std::mutex> _lock(_mutex);
	std::size_t _send_que_size = _send_que.size();
	if (_send_que_size > MAX_LENGTH) {
		std::cout << "session " << _uuid << "send que is fulled,length is" << MAX_LENGTH << std::endl;
		return;
	}
	_send_que.push(std::make_shared<SendNode>(data, len, msg_id));
	if (_send_que_size > 0) {
		return;
	}
	auto msgnode = _send_que.front();
	_lock.unlock();
	asio::async_write(_socket, asio::buffer(msgnode->_data, msgnode->_total_len),
		std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_from_this()));

}

void CSession::Send(std::string data, short msg_id)
{
	Send(data.c_str(), data.length(), msg_id);
}

void CSession::Close() {
	_b_close = true;
	_socket.close();
}

LogicNode::LogicNode(std::shared_ptr<CSession>session, std::shared_ptr<RecvNode>recv_node):_session(session),_recv_node(recv_node)
{
}
