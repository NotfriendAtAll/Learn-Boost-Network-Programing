#include <boost/asio.hpp>
#include "Server.h"
#include <iostream>
int main()
{
	try
	{
		boost::asio::io_context ioc;
		Server server(ioc, 10086);
		ioc.run();
	}
	catch (const std::exception& e)
	{
		std::cout << "the exception is" << e.what();
	}
	std::cout << "Hello World!\n";
}