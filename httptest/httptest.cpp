// httptest.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <json/value.h>
#include <json/reader.h>
#include <json/json.h>
#include <memory>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <chrono>
#include <ctime>

namespace beast = boost::beast;
namespace http = beast::http;
using tcp = boost::asio::ip::tcp;
using namespace boost;

namespace my_program_state {
    std::size_t request_count(){
        static std::size_t count = 0;
    return ++count;
    }
    std::time_t now() {
        return std::time(0);
    }
}

class http_connect:public std::enable_shared_from_this<http_connect> 
{
public:
    http_connect(tcp::socket socket):socket_(std::move(socket)) {
            
    }

    void start() {
        read_request();
        check_deadline();
    }
private:
    tcp::socket socket_;
    beast::flat_buffer buffer_{ 8192 };
    asio::steady_timer deadline{ socket_.get_executor(),std::chrono::seconds(60) };
    http::request<http::dynamic_body>request_;
    http::response<http::dynamic_body>response_;
    
    void read_request() {
        auto self = shared_from_this();
        http::async_read(socket_, buffer_, response_,
            [self](beast::error_code& ec,std::size_t bytes_transferred) {
                boost::ignore_unused(bytes_transferred);
                if (!ec) {
                    self->process_request();
                }
            });
    }
    void process_request() {
        response_.version(request_.version());
        response_.keep_alive(false);
        switch (request_.method()) {
        case http::verb::get:
            response_.result(http::status::ok);
            response_.set(http::field::server, "Beast");
            create_response();
            break;
        case http::verb::post:
            response_.result(http::status::ok);
            response_.set(http::field::server, "Beast");
            create_post_response();
            break;
        default:
            response_.set(http::field::content_type, "text/plain");
            response_.result(http::status::bad_request);
            beast::ostream(response_.body())
                << "Invalid request method '"
                << std::string(request_.method_string())
                << "'";
            break;
        }
        write_response();
    }

    void create_response() {
        if (request_.target() == "/count") {
            response_.set(http::field::content_type, "text/html");
            beast::ostream(response_.body())
                << "<html>\n"
                << "<head><title>Request count</title></head>\n"
                << "<body>\n"
                << "<h1>Request count</h1>\n"
                << "<p>There have been "
                << my_program_state::request_count()
                << " requests so far.</p>\n"
                << "</body>\n"
                << "</html>\n";
        }
        else if (request_.target() == "/time") {
            response_.set(http::field::content_type, "text/html");
            beast::ostream(response_.body())
                << "<html>\n"
                << "<head><title>Request time</title></head>\n"
                << "<body>\n"
                << "<h1>Current time</h1>\n"
                << "<p>The current time is "
                << my_program_state::now()
                << " seconds since the epoch</p>\n"
                << "</body>\n"
                << "</html>\n";
        }
        else {
            response_.result(http::status::not_found);
            response_.set(http::field::content_type, "text/plain");
            beast::ostream(response_.body())
                << "File is not found\n";
        }
    }

    void create_post_response()
    {
        if (request_.target() == "/email")
        {
            auto body = request_.body();
            auto body_str = beast::buffers_to_string(body.data());
            std::cout << "recv email is " << body_str << "." << std::endl;
            response_.set(http::field::content_type, "text/json");
            Json::Reader reader;
            Json::Value root;
            Json::Value src_root;
            bool parse_success = reader.parse(body_str, src_root);
            if (!parse_success) {
                std::cout << "failed to parse Json data" << std::endl;
                root["error"] = 1001;
                std::string jsonstr = root.toStyledString();
                beast::ostream(response_.body()) << jsonstr;
            }
            root["email"] = src_root["email"];
            root["error"] = 0;
            root["msg"] = "receive email post success";
            std::string jsonstr = root.toStyledString();
            beast::ostream(response_.body()) << jsonstr;
        }
        else {
            response_.result(http::status::not_found);
            response_.set(http::field::content_type, "text/plain");
            beast::ostream(response_.body())
                << "File is not found\r\n";
        }
    }

    void write_response() {
        auto self = shared_from_this();
        response_.content_length(request_.body().size());
        http::async_write(socket_, response_, [self](beast::error_code& ec,std::size_t) {
                if (!ec) {
                    self->socket_.shutdown(tcp::socket::shutdown_send, ec);
                    self->deadline.cancel();
                }
            });
    }
    void check_deadline()
    {
        auto self = shared_from_this();
        deadline.async_wait([self](beast::error_code& ec) {
            if (!ec) {
                self->socket_.close();
            }
            });
    }
};

void http_server(tcp::acceptor& acception, tcp::socket& socket) {
    acception.async_accept(socket, [&](beast::error_code& ec) {
        if (!ec) {
            std::make_shared<http_connect>(std::move(socket))->start();
        }
        });
    http_server(acception, socket);
}

int main()
{
    try {
        auto address = asio::ip::make_address("0.0.0.0");
        unsigned short port = static_cast<unsigned short>(8080);
        asio::io_context ioc{ 1 };
        tcp::socket socket{ ioc };
        tcp::acceptor acception{ ioc,{address,port} };
        http_server(acception, socket);
        ioc.run();


    }
    catch (std::exception& e) {
        std::cout << "exception is " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
