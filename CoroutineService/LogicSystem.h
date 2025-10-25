#pragma once
#include <iostream>
#include <queue>
#include <mutex>
#include <thread>
#include "const.h"
#include "CSession.h"
#include <map>
#include <functional>
#include <json/reader.h>
#include <json/value.h>
#include <json/json.h>

typedef std::function<void(std::shared_ptr<CSession> session, const short& msg_id, std::string msg_data)> FunCallBack;

class LogicSystem
{
public:
	~LogicSystem();
	LogicSystem& operator=(const LogicSystem&) = delete;
	LogicSystem(const LogicSystem&) = delete;
	static LogicSystem& GetInstance();
	void PostMsgToQue(std::shared_ptr<LogicNode> node);

private:
	LogicSystem();
	void RegisterCallBack();
	void HelloWorldCallBack(std::shared_ptr<CSession> session, short msg_id, std::string data);
	void DealMsg();

	std::queue<std::shared_ptr<LogicNode>>_msg_que;
	bool _b_close;
	std::thread work_thread;
	std::mutex _mutex;
	std::condition_variable _consumer;
	std::map<short, FunCallBack> _fun_callbacks;
	
};

