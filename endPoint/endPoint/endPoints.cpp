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

    //���ɶ˿ڲ���
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

//����tcp socket
int create_tcp_socket()
{
    asio::io_context ioc;
    asio::ip::tcp protocal = asio::ip::tcp::v4();
    asio::ip::tcp::socket sock(ioc);
    
    //�°汾����Ҫ����֤��
    //boost::system::error_code ec;
    //sock.open(protocal, ec);
    //if (ec.value() != 0)   //ec.value()���ִ��󷵻ط�0
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
    
    //�ϰ汾
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

//�ͻ�������
int connect_to_end()
{
    std::string raw_ip_address = "192.168.1.124";
    unsigned short port_num = 3333;
    try{
        //endpoint����������ڷ�������˵���ڰ󶨣����ڿͻ�����������
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
    const int BACKLOG_SIZE = 30;   //��������
    unsigned short port_num = 3333;
    asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);
    asio::io_context ios;
    try {
        asio::ip::tcp::acceptor acceptor(ios, ep.protocol());
        acceptor.bind(ep);
        acceptor.listen(BACKLOG_SIZE);   //��ʼ����
        asio::ip::tcp::socket sock(ios); //�ٴ���һ��sock������ͻ���ͨ��
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

//1��
void use_const_buffer()
{
    std::string buf = "hello world";
    asio::const_buffer asio_buf(buf.c_str(), buf.length());
    std::vector<asio::const_buffer> buffers_sequence;
    buffers_sequence.push_back(asio_buf);
}
//2�����򵥷���ʹ��const_buffer
void use_buffer_str()
{
    auto output_buf = asio::buffer("hello world");
}

//3��ʹ��const_buffer�洢����
void use_buffer_array()
{
    const size_t BUF_SIZE_BYTES = 20; //�����С20
    std::unique_ptr<char[]> buf(new char[BUF_SIZE_BYTES]); //ʹ������ָ���������
    //��Ϊbuffer��һ������ʹ��void*������ʹ��static_cast<void*>ǿת
    auto input_buf = asio::buffer(static_cast<void*>(buf.get()), BUF_SIZE_BYTES);
}

//1��ͬ��д
void write_to_socket(asio::ip::tcp::socket& sock)
{
    std::string buf = "hello world";
    std::size_t total_bytes = 0;   //�ܹ�д���ֽ�
    //ѭ������
    while (total_bytes != buf.length())
    {
        total_bytes += sock.write_some(asio::buffer(buf.c_str() + total_bytes,
            buf.length() - total_bytes));
    }
}
//2��
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
//ͬ��д
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
        int length = sock.send(buf.c_str(), buf.length());  //send�������ģ���һֱ�ȴ�����
        if (length <= 0)  //send��������ֵ������˵���Զ˹رգ�С����˵���������������������ֵ����buf����
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
//ͬ��д
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

//1��ͬ����
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
//2��
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

//ֱ��ʹ��receive��ȡ
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

//ֱ��ʹ��read��ȡ
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