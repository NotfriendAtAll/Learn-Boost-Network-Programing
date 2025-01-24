
#include <boost/asio.hpp>
#include <iostream>
const int MAX_SIZE = 1024;

int main() {
  try {
    boost::asio::io_context ios;
    boost::asio::ip::tcp::endpoint romote_ip(
        boost::asio::ip::address::from_string("127.0.0.1"), 10086);
    boost::asio::ip::tcp::socket sock(ios);
    boost::system::error_code error = boost::asio::error::host_not_found;
    sock.connect(romote_ip, error);

    if (error) {
      std::cout << "Connection failed,the error_code is" << error.value()
                << std::endl;
      return 0;
    } else {
      std::cout << "Connection sucessfully . Plase enter massage" << std::endl;
    }
    for (;;) {
      char requst[MAX_SIZE];
      std::cin.getline(requst, MAX_SIZE);
      size_t requstlength = strlen(requst);
      boost::asio::write(sock, boost::asio::buffer(requst, requstlength));

      char reply[MAX_SIZE];
      size_t replylength =
          boost::asio::read(sock, boost::asio::buffer(reply, requstlength));
      std::cout << "the reply massage is" << std::endl;
      std::cout.write(reply, replylength);
    }
  } catch (std::exception &e) {
    std::cout << "The error is" << e.what() << std::endl;
  }
  return 0;
}