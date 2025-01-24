
#include "Session.h"
#include <boost/asio.hpp>
#include <iostream>

int main() {
  try {
    boost::asio::io_context ioc;
    Server server(ioc, 10086);
    ioc.run();
  } catch (const std::exception &e) {
    std::cout << "the error is" << e.what();
  }

  return 0;
}
