#pragma once
#include "const.h"
#include <iostream>
#include <boost/asio.hpp>

class MsgNode
{
	friend class Session;
public:
	MsgNode(short max_len) :_max_len(max_len), _cur_len(0) {
		_data = new char[max_len + 1];
		_data[max_len] = '\0';
	}
	~MsgNode() {
		std::cout << "destruct MsgNode" << std::endl;
		delete[] _data;
	}
	void Clear()
	{
		memset(_data, 0, _max_len);
		_cur_len = 0;
	}
	short _cur_len;   //已读长度
	short _max_len; //总长度
	char* _data;
};

class RecvNode :public MsgNode
{
	friend class LogicSystem;
public:
	RecvNode(short max_len, short msg_id);
private:
	short _msg_id;
};

class SendNode :public MsgNode
{
	friend class LogicSystem;
public:
	SendNode(const char* msg, size_t msg_len, short msg_id);
private:
	short _msg_id;
};
