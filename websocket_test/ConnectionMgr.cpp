#include "ConnectionMgr.h"

ConnectionMgr& ConnectionMgr::GetInstance()
{
    static ConnectionMgr instance;
    return instance;
}

void ConnectionMgr::AddConnection(std::shared_ptr<connection>conn)
{
    _map[conn->GetUuid()] = conn;
}

void ConnectionMgr::RMConnection(std::string uuid)
{
    _map.erase(uuid);
}

ConnectionMgr::ConnectionMgr()
{
}
