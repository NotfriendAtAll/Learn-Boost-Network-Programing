#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <set>

const size_t max_length = 1024;
using sock_ptr = std::shared_ptr<boost::asio::ip::tcp::socket>;
std::set<std::shared_ptr<std::thread>> thread_set;

void Seesion(sock_ptr sock) {
  try {
    for (;;) {
      char data[max_length];
      memset(data, '\0', max_length);
      boost::system::error_code error;
      size_t length =
          sock->read_some(boost::asio::buffer(data, max_length), error);
      if (error == boost::asio::error::eof) {
        std::cout << "Connection failed by peer" << std::endl;
        break;
      } else if (error) {
        throw boost::system::error_code(error);
      }
      std::cout << "Remote IP is"
                << sock->remote_endpoint().address().to_string() << '\n';
      std::cout << " Recieve data is" << data << '\n';
      boost::asio::write(*sock, boost::asio::buffer(data, length));
    }
  } catch (std::exception &e) {
    std::cout << "the code is" << e.what() << std::endl;
  }
}

void Server(boost::asio::io_context &ioc, unsigned short portnum) {
  boost::asio::ip::tcp::acceptor accept(
      ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), portnum));
  for (;;) {
    sock_ptr sock(new boost::asio::ip::tcp::socket(ioc));
    accept.accept(*sock);
    auto t = std::make_shared<std::thread>(Seesion, sock);
    thread_set.insert(t);
  }
}

int main() {
  try {
    boost::asio::io_context ioc;
    Server(ioc, 10086);
    for (auto &it : thread_set) {
      it->join();
    }
  } catch (std::exception &e) {
    std::cout << "the code is" << e.what();
  }
  std::cout << "Hello World!\n";
}
