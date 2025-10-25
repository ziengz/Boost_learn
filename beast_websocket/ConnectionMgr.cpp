#include "ConnectionMgr.h"

ConnectionMgr& ConnectionMgr::GetInstance()
{
    static ConnectionMgr instance;
    return instance;
}

void ConnectionMgr::AddConnection(std::shared_ptr<connection> conptr)
{
    _map[conptr->GetUuid()] = conptr;
}

void ConnectionMgr::RmConnection(std::string uuid)
{
    _map.erase(uuid);
}

ConnectionMgr::ConnectionMgr()
{
}
