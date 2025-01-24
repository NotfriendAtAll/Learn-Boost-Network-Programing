#pragma once
#include <boost/asio.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>
#include <map>
#include <string>

using boost::asio::ip::tcp;
class Server;
class Session : public std::enable_shared_from_this<Session> {
public:
  Session(boost::asio::io_context &ioc, Server *server)
      : _socket(ioc), _server(server) {
    boost::uuids::uuid m_uuid = boost::uuids::random_generator()();
    uuid = boost::uuids::to_string(m_uuid);
  }
  tcp::socket &Socket() { return _socket; }
  std::string &Get_uuid();
  void Start();

private:
  void handle_read(const boost::system::error_code &error,
                   size_t bytes_transfered,
                   std::shared_ptr<Session> new_session);
  void handle_write(const boost::system::error_code &error,
                    std::shared_ptr<Session> new_session);
  tcp::socket _socket;
  enum { max_length = 1024 };
  char _data[max_length];
  std::string uuid;
  Server *_server;
};

class Server {
public:
  Server(boost::asio::io_context &ioc, short port);
  void ClearSession(std::string uuids);

private:
  void start_accept();
  void handle_accept(std::shared_ptr<Session> new_session,
                     const boost::system::error_code &error);
  boost::asio::io_context &_ioc;
  tcp::acceptor _acceptor;
  std::map<std::string, std::shared_ptr<Session>> _sessions;
};
