#include <iostream>
#include <boost/asio.hpp>
#include <memory>
#include <set>
using namespace boost;
const int max_length = 1024;
typedef std::shared_ptr<asio::ip::tcp::socket> socket_ptr; //用于存储socket的智能指针
std::set<std::shared_ptr<std::thread>> thread_set;  //用于存储线程的集合

//
void session(socket_ptr sock)
{
    try {
        while (true)
        {
            char data[max_length];
            memset(data, '\0', max_length);
            system::error_code ec;
            //使用read读取需要读取整个max_length，所以使用read_some更合适
            //size_t length = asio::read(sock, asio::buffer(data, max_length));
            size_t length = sock->read_some(asio::buffer(data, max_length),ec);
            if (ec == asio::error::eof)
            {
                std::cout << "connection close by peer" << std::endl;
                break;
            }
            else if (ec)
            {
                throw system::system_error(ec);
            }
            std::cout << "receive from " << sock->remote_endpoint().address() << std::endl;
            std::cout << "receive context is " << data << std::endl;
            //回传
            asio::write(*sock, asio::buffer(data, length));
        }
    }
    catch (system::system_error& e)
    {
        std::cout<<"Error occured!Error code = " << e.code()
            << ".Message = " << e.what();
    }
}

//链接客户端
void server(asio::io_context& ioc,unsigned short port)
{
    std::cout << "开启服务,等待客户端接入。。。" << std::endl;
    asio::ip::tcp::acceptor a(ioc, asio::ip::tcp::endpoint(asio::ip::tcp::v4(),port));
    while (1)
    {
        socket_ptr sock(new asio::ip::tcp::socket(ioc));
        a.accept(*sock);
        //创建一个线程处理接受来自客户端的信息
        auto t = std::make_shared<std::thread>(session, sock);
        thread_set.insert(t); //防止线程未结束回收
    }
}

int main()
{
    try {
        asio::io_context ioc;
        server(ioc, 10086);
        for (auto& t : thread_set)
        {
            t->join();  //防止子线程未结束，主线程退出
        }
    }
    catch (std::exception& e)
    {
        std::cout <<"Exception: " << e.what();
    }
}
