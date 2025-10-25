#pragma once
#include "connection.h"
#include "boost/unordered_map.hpp"
#include <iostream>


class ConnectionMgr
{
public:
	static ConnectionMgr& GetInstance();
	ConnectionMgr& operator= (const ConnectionMgr&) = delete;
	ConnectionMgr (const ConnectionMgr&) = delete;
	void AddConnection(std::shared_ptr<connection>);
	void RmConnection(std::string uuid);
private:
	ConnectionMgr();
	boost::unordered_map<std::string, std::shared_ptr<connection>> _map;
};

