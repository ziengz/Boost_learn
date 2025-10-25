#include "endPoint.h"
#include <boost/asio.hpp>
#include <string>
#include <iostream>
using namespace boost;

int client_end_point()
{
    std::string raw_ip_address = "127.4.8.1";
    unsigned short port_num = 3333;
    boost::system::error_code ec;
    asio::ip::address ip_address = boost::asio::ip::make_address(raw_ip_address, ec);
    if (ec.value() != 0)
    {
        std::cout << "failed to parse the IP address.Error code = " << ec.value() << ".Message is " << ec.message() << std::endl;
        return ec.value();
    }

    //生成端口操作
    asio::ip::tcp::endpoint ep(ip_address, port_num);
    return 0;
}

int server_end_point()
{
    asio::ip::address ip_address = asio::ip::address_v6::any();
    unsigned short port_num = 3333;
    asio::ip::tcp::endpoint ep(ip_address, port_num);
    return 0;
}

//创建tcp socket
int create_tcp_socket()
{
    asio::io_context ioc;
    asio::ip::tcp protocal = asio::ip::tcp::v4();
    asio::ip::tcp::socket sock(ioc);
    
    //新版本不需要打开验证了
    //boost::system::error_code ec;
    //sock.open(protocal, ec);
    //if (ec.value() != 0)   //ec.value()出现错误返回非0
    //{
    //    std::cout << "failed to parse the IP address.Error code = " << ec.value() 
    //        << ".Message is " << ec.message() << std::endl;
    //    return ec.value();
    //}

    return 0;
}

int create_accepcor_socket()
{
    asio::io_context ios;
    
    //老版本
    //asio::ip::tcp::acceptor acceptor(ios);
    //asio::ip::tcp protocal = asio::ip::tcp::v4();
    //boost::system::error_code ec;
    //acceptor.open(protocal, ec);
    //if (ec.value() != 0)
    //{
    //    std::cout << "failed to parse the IP address.Error code = " 
    //        << ec.value() << ".Message is " 
    //        << ec.message() << std::endl;
    //    return ec.value();
    //}
    
    asio::ip::tcp::acceptor a(ios, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 3333));
    return 0;
}

int bind_acceptor_socket()
{
    unsigned short port_num = 3333;
    asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);
    asio::io_context ios;
    asio::ip::tcp::acceptor accept(ios, ep.protocol());
    boost::system::error_code ec;
    accept.bind(ep, ec);
    if (ec.value() != 0)
    {
        std::cout << "failed to parse the IP address.Error code = " << ec.value() << ".Message is " << ec.message() << std::endl;
        return ec.value();
    }
    return 0;
}

//客户端链接
int connect_to_end()
{
    std::string raw_ip_address = "192.168.1.124";
    unsigned short port_num = 3333;
    try{
        //endpoint这个函数对于服务器来说用于绑定，对于客户端用于链接
        asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address), port_num);
        asio::io_context ios;
        asio::ip::tcp::socket sock(ios, ep.protocol());
        sock.connect(ep);
    }
    catch(system::system_error& e){
        std::cout << "Error occured! Error code = " << e.code()
            << ".Message = " << e.what();
        return e.code().value();
    }
    

    return 0;
}

int accept_new_connection()
{
    const int BACKLOG_SIZE = 30;   //最大监听数
    unsigned short port_num = 3333;
    asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);
    asio::io_context ios;
    try {
        asio::ip::tcp::acceptor acceptor(ios, ep.protocol());
        acceptor.bind(ep);
        acceptor.listen(BACKLOG_SIZE);   //开始监听
        asio::ip::tcp::socket sock(ios); //再创建一个sock用于与客户端通信
        acceptor.accept(sock);        
    }
    catch (system::system_error& e)
    {
        std::cout << "Error occured! Error code = " << e.code()
            << ".Message = " << e.what();
        return e.code().value();
    }
    return 0;
}

//1、
void use_const_buffer()
{
    std::string buf = "hello world";
    asio::const_buffer asio_buf(buf.c_str(), buf.length());
    std::vector<asio::const_buffer> buffers_sequence;
    buffers_sequence.push_back(asio_buf);
}
//2、更简单方法使用const_buffer
void use_buffer_str()
{
    auto output_buf = asio::buffer("hello world");
}

//3、使用const_buffer存储数组
void use_buffer_array()
{
    const size_t BUF_SIZE_BYTES = 20; //数组大小20
    std::unique_ptr<char[]> buf(new char[BUF_SIZE_BYTES]); //使用智能指针管理数组
    //因为buffer第一个参数使用void*，所以使用static_cast<void*>强转
    auto input_buf = asio::buffer(static_cast<void*>(buf.get()), BUF_SIZE_BYTES);
}

//1、同步写
void write_to_socket(asio::ip::tcp::socket& sock)
{
    std::string buf = "hello world";
    std::size_t total_bytes = 0;   //总共写入字节
    //循环发送
    while (total_bytes != buf.length())
    {
        total_bytes += sock.write_some(asio::buffer(buf.c_str() + total_bytes,
            buf.length() - total_bytes));
    }
}
//2、
int send_data_by_write_some()
{
    std::string raw_ip_address = "192.168.0.1";
    unsigned short port_num = 3333;
    try {
        asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address), port_num);
        asio::io_context ioc;
        asio::ip::tcp::socket sock(ioc, ep.protocol());
        std::string buf = "hello world!";
        sock.connect(ep);
        write_to_socket(sock); 
    }
    catch (system::system_error& e)
    {
        std::cout << "Error occured! Error code = " << e.code()
            << ".Message = " << e.what();
        return e.code().value();
    }
    return 0;
}
//同步写
int send_data_by_send()
{
    std::string raw_ip_address = "192.168.0.1";
    unsigned short port_num = 3333;
    try {
        asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address), port_num);
        asio::io_context ioc;
        asio::ip::tcp::socket sock(ioc, ep.protocol());
        std::string buf = "hello world!";
        sock.connect(ep);
        // write_to_socket(sock); 
        int length = sock.send(buf.c_str(), buf.length());  //send是阻塞的，会一直等待发完
        if (length <= 0)  //send函数返回值等于零说明对端关闭，小于零说明出错，大于零则发送完毕且值等于buf长度
        {
            return 0;
        }
    }
    catch (system::system_error& e)
    {
        std::cout << "Error occured! Error code = " << e.code()
            << ".Message = " << e.what();
        return e.code().value();
    }
    return 0;

}
//同步写
int send_data_by_write()
{
    std::string raw_ip_address = "192.168.0.1";
    unsigned short port_num = 3333;
    try {
        asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address), port_num);
        asio::io_context ioc;
        asio::ip::tcp::socket sock(ioc, ep.protocol());
        std::string buf = "hello world!";
        sock.connect(ep);
        // write_to_socket(sock); 
        int length = asio::write(sock, asio::buffer(buf.c_str(), buf.length()));
        if (length <= 0)  
        {
            return 0;
        }
    }
    catch (system::system_error& e)
    {
        std::cout << "Error occured! Error code = " << e.code()
            << ".Message = " << e.what();
        return e.code().value();
    }
    return 0;

}

//1、同步读
std::string read_from_socket(asio::ip::tcp::socket& sock)
{
    const unsigned char MESSAGE_SIZE = 7;
    char buf[MESSAGE_SIZE];
    std::size_t total_bytes = 0;
    while (total_bytes != MESSAGE_SIZE)
    {
        total_bytes += sock.read_some(asio::buffer(buf + total_bytes, MESSAGE_SIZE - total_bytes));
    }
    return std::string(buf, total_bytes);
}
//2、
int read_data_by_read_some()
{
    std::string raw_ip_address = "192.168.0.1";
    unsigned short port_num = 3333;
    try {
        asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address), port_num);
        asio::io_context ioc;
        asio::ip::tcp::socket sock(ioc, ep.protocol());
        sock.connect(ep);
        read_from_socket(sock);
    }
    catch (system::system_error& e)
    {
        std::cout << "Error occured! Error code = " << e.code()
            << ".Message = " << e.what();
        return e.code().value();
    }
    return 0;
}

//直接使用receive读取
int read_data_by_read_some()
{
    std::string raw_ip_address = "192.168.0.1";
    unsigned short port_num = 3333;
    try {
        asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address), port_num);
        asio::io_context ioc;
        asio::ip::tcp::socket sock(ioc, ep.protocol());
        sock.connect(ep);
        const unsigned char BUFF_SIZE = 7;
        char buffer_receive[BUFF_SIZE];
        int receive = sock.receive(asio::buffer(buffer_receive, BUFF_SIZE));
        if (receive <= 0)
        {
            std::cout << "receive failed" << std::end;
            return 0;
        }
    }
    catch (system::system_error& e)
    {
        std::cout << "Error occured! Error code = " << e.code()
            << ".Message = " << e.what();
        return e.code().value();
    }
    return 0;
}

//直接使用read读取
int read_data_by_read_some()
{
    std::string raw_ip_address = "192.168.0.1";
    unsigned short port_num = 3333;
    try {
        asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address), port_num);
        asio::io_context ioc;
        asio::ip::tcp::socket sock(ioc, ep.protocol());
        sock.connect(ep);
        const unsigned char BUFF_SIZE = 7;
        char buffer_receive[BUFF_SIZE];
        int receive = asio::read(sock, asio::buffer(buffer_receive, BUFF_SIZE));
        if (receive <= 0)
        {
            std::cout << "receive failed" << std::endl;
            return 0;
        }
    }
    catch (system::system_error& e)
    {
        std::cout << "Error occured! Error code = " << e.code()
            << ".Message = " << e.what();
        return e.code().value();
    }
    return 0;
}