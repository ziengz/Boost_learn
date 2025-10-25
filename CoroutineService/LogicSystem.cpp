#include "LogicSystem.h"

LogicSystem::LogicSystem()
{
	RegisterCallBack();
	work_thread = std::thread(&LogicSystem::DealMsg, this);
}

void LogicSystem::RegisterCallBack()
{
	_fun_callbacks[MSG_HELLOWORLD] = std::bind(&LogicSystem::HelloWorldCallBack, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

}

void LogicSystem::PostMsgToQue(std::shared_ptr<LogicNode> node)
{
	std::unique_lock<std::mutex>unique_lock(_mutex);
	_msg_que.push(node);
	if (_msg_que.size() == 1) {
		_consumer.notify_one();
	}
}

void LogicSystem::HelloWorldCallBack(std::shared_ptr<CSession> session, short msg_id, std::string data)
{
	Json::Reader reader;
	Json::Value root;
	reader.parse(data,root);
	std::cout << "msg is received,msg id is " << root["id"].asInt() << "msg data is " << root["data"].asString() << std::endl;
	root["data"] = "server is received msg ,msg data is " + root["data"].asString();

	std::string return_str = root.toStyledString();
	session->Send(return_str, root["id"].asInt());
}

void LogicSystem::DealMsg()
{
	std::unique_lock<std::mutex>unique_lk(_mutex);
	while (true)
	{
		while (_msg_que.empty() && !_b_close) {
			_consumer.wait(unique_lk);
		}

		if (_b_close) {
			while (!_msg_que.empty())
			{
				auto msg_node = _msg_que.front();
				short msg_id = msg_node->_recv_node->_msg_id;
				std::cout << "msg_id is " << msg_id << std::endl;
				auto msg_node_iter = _fun_callbacks.find(msg_node->_recv_node->_msg_id);
				if (msg_node_iter == _fun_callbacks.end()) {
					_msg_que.pop();
					continue;
				}
				msg_node_iter->second(msg_node->_session, msg_node->_recv_node->_msg_id, 
					std::string(msg_node->_recv_node->_data,msg_node->_recv_node->_total_len));
				_msg_que.pop();
			}
			break;
		}
		//没停服说明还有数据
		auto msg_node = _msg_que.front();
		short msg_id = msg_node->_recv_node->_msg_id;
		std::cout << "msg_id is " << msg_id << std::endl;
		auto msg_node_iter = _fun_callbacks.find(msg_node->_recv_node->_msg_id);
		if (msg_node_iter == _fun_callbacks.end()) {
			_msg_que.pop();
			continue;
		}
		msg_node_iter->second(msg_node->_session, msg_node->_recv_node->_msg_id,
			std::string(msg_node->_recv_node->_data, msg_node->_recv_node->_total_len));
		_msg_que.pop();
	}

}

LogicSystem::~LogicSystem()
{
	_b_close = true;
}

LogicSystem& LogicSystem::GetInstance() {
	static LogicSystem instance;
	return instance;
}