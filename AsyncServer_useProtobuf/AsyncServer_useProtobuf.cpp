#include <iostream>
#include "Session.h"
#include <boost/asio.hpp>

int main()
{
    asio::io_context ioc;

    try {
        Server server(ioc, 10086);
        ioc.run();
    }
    catch (system::error_code& e)
    {
        std::cout << "error is " << e.what() << std::endl;
    }
}
