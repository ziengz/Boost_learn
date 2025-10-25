#pragma once
#include "Singleton.h"
#include "const.h"
#include "Session.h"
#include <queue>
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include <functional>
#include <mutex>
#include <thread>
#include <map>


typedef std::function<void(std::shared_ptr<Session>, short msg_id, std::string msg_data)> FunCallBack;

class LogicSystem:public Singleton<LogicSystem>
{
	friend Singleton<LogicSystem>;
public:
	//��Ҫ����������������ΪSingleton��_instance = shared_ptr<T>(new T)�У����������LogicSystem����������
	~LogicSystem();   
	void PostMsgToQue(std::shared_ptr<LogicNode> msg);
private:
	LogicSystem();
	void RegisterCallBack();       //����msg_id��ӵ�map�лص�����
	void HelloWorldCallBack(std::shared_ptr<Session>,short msg_id,std::string msg_data);
	void DealMsg();        //ȡ�����ݲ����ûص���������


	std::queue<std::shared_ptr<LogicNode>> _msg_que;
	std::mutex _mutex;
	std::condition_variable _consumer;//ʹ������������Ŀ����Ϊ�˷�ֹ��ʹ�߼�����û�������ݣ��������תռ��cpu
	std::thread _worker_thread;
	bool _b_stop;          //��ʾ�յ��ⲿ��ֹͣ�źţ��߼���Ҫ��ֹ�����̲߳������˳���
	std::map<short, FunCallBack> _fun_callbacks;
};

