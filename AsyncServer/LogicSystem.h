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
	//需要公有析构函数，因为Singleton在_instance = shared_ptr<T>(new T)中，析构会调用LogicSystem的析构函数
	~LogicSystem();   
	void PostMsgToQue(std::shared_ptr<LogicNode> msg);
private:
	LogicSystem();
	void RegisterCallBack();       //根据msg_id添加到map中回调函数
	void HelloWorldCallBack(std::shared_ptr<Session>,short msg_id,std::string msg_data);
	void DealMsg();        //取出数据并调用回调函数发送


	std::queue<std::shared_ptr<LogicNode>> _msg_que;
	std::mutex _mutex;
	std::condition_variable _consumer;//使用条件变量的目的是为了防止即使逻辑队列没还有数据，在那里空转占用cpu
	std::thread _worker_thread;
	bool _b_stop;          //表示收到外部的停止信号，逻辑类要中止工作线程并优雅退出。
	std::map<short, FunCallBack> _fun_callbacks;
};

