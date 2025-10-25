#pragma once
#include "connection.h"
#include "boost/unordered_map.hpp"

class ConnectionMgr
{
public:
	ConnectionMgr(const ConnectionMgr&) = delete;
	ConnectionMgr& operator= (const ConnectionMgr&) = delete;
	static ConnectionMgr& GetInstance();
	void AddConnection(std::shared_ptr<connection>);
	void RMConnection(std::string uuid);

private:
	ConnectionMgr();
	unordered_map<std::string, std::shared_ptr<connection>>_map;

};

