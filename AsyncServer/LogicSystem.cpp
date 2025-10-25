#include "LogicSystem.h"

LogicSystem::LogicSystem():_b_stop(false)
{
	RegisterCallBack();          //��������Ա������Ҫ��ָ��
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

 //�Ӷ�����ȡ�����ݲ�����msg_id���ö�Ӧ�ص�����
void LogicSystem::DealMsg()
{
	std::unique_lock<std::mutex> unique_lk(_mutex);
	while (true)
	{
		while (_msg_que.empty() && !_b_stop) {
			//wait���������ͷ������������߳̿��Խ��������Ϣ������Ϣ�����ἤ������´�����������Ϣ
			_consumer.wait(unique_lk);
		}
		//�ж��Ƿ�Ϊ�ر�״̬���������߼�ִ��������˳�ѭ��
		if (_b_stop)
		{
			while (!_msg_que.empty())
			{
				//1��ȡ������
				auto msg_node = _msg_que.front();
				std::cout << "recv msg id is " << msg_node->_recvnode->_msg_id << std::endl;
				//2������msg_id�ҵ���Ӧ�ص�����
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
		//���û��ͣ����˵����������
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
	//����Ϣ�����ˣ������߳�
	if (_msg_que.size() == 1)
	{
		_consumer.notify_one();
	}

}

LogicSystem::~LogicSystem()
{
	_b_stop = true;

}


