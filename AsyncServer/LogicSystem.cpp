#include "LogicSystem.h"

LogicSystem::LogicSystem():_b_stop(false)
{
	RegisterCallBack();          //如果是类成员，就需要给指针
	_worker_thread = std::thread(&LogicSystem::DealMsg, this);
}

void LogicSystem::RegisterCallBack()
{
	_fun_callbacks[MSG_HELLO_WORLD] = std::bind(&LogicSystem::HelloWorldCallBack, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

}

void LogicSystem::HelloWorldCallBack(std::shared_ptr<Session> session, short msg_id, std::string msg_data)
{
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	std::cout << "recevie msg id  is " << root["id"].asInt() << " msg data is "
		<< root["data"].asString() << endl;
	root["data"] = "server has received, msg data is " + root["data"].asString();
	std::string return_str = root.toStyledString();
	session->Send(return_str, root["id"].asInt());
}

 //从队列中取出数据并根据msg_id调用对应回调函数
void LogicSystem::DealMsg()
{
	std::unique_lock<std::mutex> unique_lk(_mutex);
	while (true)
	{
		while (_msg_que.empty() && !_b_stop) {
			//wait函数可以释放锁，让其他线程可以进来存放消息，有消息进来会激活，并重新打开锁，处理消息
			_consumer.wait(unique_lk);
		}
		//判断是否为关闭状态，把所有逻辑执行完后则退出循环
		if (_b_stop)
		{
			while (!_msg_que.empty())
			{
				//1、取出数据
				auto msg_node = _msg_que.front();
				std::cout << "recv msg id is " << msg_node->_recvnode->_msg_id << std::endl;
				//2、根据msg_id找到对应回调函数
				auto call_back_iter = _fun_callbacks.find(msg_node->_recvnode->_msg_id);
				if (call_back_iter == _fun_callbacks.end())
				{
					_msg_que.pop();
					continue;
				}
				call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_id, 
					std::string(msg_node->_recvnode->_data,msg_node->_recvnode->_cur_len));
				_msg_que.pop();
			}
			break;
		}
		//如果没有停服，说明还有数据
		auto msg_node = _msg_que.front();
		std::cout << "recv msg id is " << msg_node->_recvnode->_msg_id << std::endl;
		auto call_back_iter = _fun_callbacks.find(msg_node->_recvnode->_msg_id);
		if (call_back_iter == _fun_callbacks.end())
		{
			_msg_que.pop();
			continue;
		}
		call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_id,
			std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len));
		_msg_que.pop();
	}
}

void LogicSystem::PostMsgToQue(std::shared_ptr<LogicNode> msg)
{
	std::unique_lock<std::mutex> unique_lk(_mutex);
	_msg_que.push(msg);
	//有消息进来了，唤醒线程
	if (_msg_que.size() == 1)
	{
		_consumer.notify_one();
	}

}

LogicSystem::~LogicSystem()
{
	_b_stop = true;

}


