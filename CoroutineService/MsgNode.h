#pragma once
#include <iostream>
#include "const.h"
#include <boost/asio.hpp>
using namespace boost;

class MsgNode
{
public:
	MsgNode(short max_len) :_total_len(max_len), _cur_len(0) {
		_data = new char[max_len+1];
		_data[_total_len] = '\0';
	}
	~MsgNode() {
		std::cout << "MsgNode is destructed" << std::endl;
		delete[] _data;
	}
	void Clear() {
		memset(_data, 0, _total_len);
		_cur_len = 0;
	}

	short _cur_len;
	short _total_len;
	char* _data;
};

class SendNode:public MsgNode{
public:
	SendNode(const char*data,std::size_t msg_len,short msg_id);
	short _msg_id;
};

class RecvNode :public MsgNode {
public:
	RecvNode(short max_len,short msg_id);

	short _msg_id;
};
