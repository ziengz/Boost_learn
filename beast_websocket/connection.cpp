#include "connection.h"
#include <iostream>
#include "ConnectionMgr.h"

connection::connection(asio::io_context& ioc):_ioc(ioc),_ws_ptr(std::make_unique<stream<tcp_stream>>(make_strand(ioc)))
{
    boost::uuids::random_generator random_uuid;
    boost::uuids::uuid uuid = random_uuid();
    _uuid = boost::uuids::to_string(uuid);
}

std::string connection::GetUuid()
{
    return _uuid;
}

tcp::socket& connection::GetSocket()
{
    auto& socket = beast::get_lowest_layer(*_ws_ptr).socket();
    return socket;
}

void connection::Async_accept()
{
    auto  self = shared_from_this();
    _ws_ptr->async_accept([self](beast::error_code ec) {
        try {
            if (!ec) {
                ConnectionMgr::GetInstance().AddConnection(self);
                self->start();
            }
            else {
                std::cout << "websocket is failed to accept,exception is " << ec.what() << std::endl;
            }
        }
        catch (std::exception& ec) {
            std::cout << "exception is " << ec.what() << std::endl;
            return;
        }
        
        });
}

void connection::start()
{
    auto self = shared_from_this();
    _ws_ptr->async_read(buffer_, [self](error_code ec, std::size_t bytes_buffer) {
        try {
            if (ec) {
                std::cout << "websocekt read failed£¬read error is " << ec.what() << std::endl;
                ConnectionMgr::GetInstance().RmConnection(self->GetUuid());
                return;
            }
            self->_ws_ptr->text(self->_ws_ptr->got_text());
            std::string recv_data = boost::beast::buffers_to_string(self->buffer_.data());
            self->buffer_.consume(self->buffer_.size());
            std::cout << "websocekt receive msg is " << recv_data << std::endl;
            self->AsyncSend(std::move(recv_data));
            self->start();
            
        }
        catch (std::exception& ec) {
            std::cout << "exception is " << ec.what() << std::endl;
            return;
        }
        });


}

void connection::AsyncSend(std::string data)
{
    {
        {
            std::lock_guard <std::mutex> lock(_send_mutex);
            int length = que_msg.size();
            que_msg.push(data);
            if (length > 0)
            {
                return;
            }
        }
        auto self = shared_from_this();
        _ws_ptr->async_write(asio::buffer(data.c_str(), data.length()),
            [self](error_code ec, std::size_t buffer_byte) {
            try {
                if (ec) {
                    std::cout << "async send msg failed,exception is " << ec.what() << std::endl;
                    ConnectionMgr::GetInstance().RmConnection(self->GetUuid());
                    return;
                }
                std::string send_msg;
                {
                    std::lock_guard<std::mutex>lock(self->_send_mutex);
                    self->que_msg.pop();
                    if (self->que_msg.empty()) {
                        return;
                    }
                    send_msg = self->que_msg.front();
                }
                self->AsyncSend(std::move(send_msg));
                

            }
            catch (std::exception& ec) {
                std::cout << "exception is " << ec.what() << std::endl;
                ConnectionMgr::GetInstance().RmConnection(self->GetUuid());
                return;
            }

            });
    }
}






