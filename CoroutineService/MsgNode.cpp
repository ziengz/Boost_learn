#include "MsgNode.h"
RecvNode::RecvNode(short max_len, short msg_id) :_msg_id(msg_id), MsgNode(max_len)
{
}
SendNode::SendNode(const char* data, std::size_t msg_len, short msg_id):
	_msg_id(msg_id),MsgNode(msg_len+HEAD_TOTAL_LEN)
{
	short msg_net_id = asio::detail::socket_ops::host_to_network_short(msg_id);
	memcpy(_data ,&msg_net_id, HEAD_ID_LENGTH);
	short msg_net_len = asio::detail::socket_ops::host_to_network_short(msg_len);
	memcpy(_data + HEAD_ID_LENGTH, &msg_net_len,  HEAD_DATA_LEN);
	memcpy(_data + HEAD_TOTAL_LEN, data, msg_len);

}


