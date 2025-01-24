#pragma once
#include <boost/asio.hpp>
#include <boost/uuid/basic_random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <iostream>
#include <map>
#include <string>

using boost::asio::ip::tcp;

class Session {
public:
  explicit Session(boost::asio::io_context &ioc) : _socket(ioc) {}
  tcp::socket &Socket() { return _socket; }
  void Start();
  std::string &Get_uuid() { auto uuid = boost::uuids:: }

private:
  void HandRead(boost::system::error_code error, size_t bit,
                std::shared_ptr<Session> shared_Session);
  void HandWrite(boost::system::error_code error,
                 std::shared_ptr<Session> shared_Session);

  static const int maxsize = 1024;
  char data[maxsize];
  tcp::socket _socket;
};

class Server {
public:
  Server(boost::asio::io_context &ioc, short port);

private:
  void Start_accept();
  void Handle_accept(std::shared_ptr<Session> new_session,
                     boost::system::error_code error);
  boost::asio::io_context &_ioc;
  tcp::acceptor _acceptor;
  std::map<std::string, std::shared_ptr<Session>> _mq;
};
