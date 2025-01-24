#include "Session.h"
std::string &Session::Get_uuid() { return uuid; }

void Session::Start() {
  memset(_data, 0, max_length);
  _socket.async_read_some(boost::asio::buffer(_data, max_length),
                          std::bind(&Session::handle_read, this,
                                    std::placeholders::_1,
                                    std::placeholders::_2, shared_from_this()));
}
void Session::handle_read(const boost::system::error_code &error,
                          size_t bytes_transfered,
                          std::shared_ptr<Session> new_session) {
  if (!error) {
    std::cout << "server receive data is " << _data << std::endl;
    boost::asio::async_write(_socket,
                             boost::asio::buffer(_data, bytes_transfered),
                             std::bind(&Session::handle_write, this,
                                       std::placeholders::_1, new_session));
  } else {
    _server->ClearSession(new_session->Get_uuid());
  }
}
void Session::handle_write(const boost::system::error_code &error,
                           std::shared_ptr<Session> new_session) {
  if (!error) {
    memset(_data, 0, max_length);
    _socket.async_read_some(boost::asio::buffer(_data, max_length),
                            std::bind(&Session::handle_read, this,
                                      std::placeholders::_1,
                                      std::placeholders::_2, new_session));
  } else {
    _server->ClearSession(new_session->Get_uuid());
  }
}
Server::Server(boost::asio::io_context &ioc, short port)
    : _ioc(ioc), _acceptor(ioc, tcp::endpoint(tcp::v4(), port)) {
  start_accept();
}

void Server::start_accept() {
  auto new_session = std::make_shared<Session>(_ioc, this);
  _acceptor.async_accept(new_session->Socket(),
                         std::bind(&Server::handle_accept, this, new_session,
                                   std::placeholders::_1));
}
void Server::handle_accept(std::shared_ptr<Session> new_session,
                           const boost::system::error_code &error) {
  if (!error) {
    new_session->Start();
    _sessions.insert(std::make_pair(new_session->Get_uuid(), new_session));
  } else {
    delete this;
  }

  start_accept();
}
void Server::ClearSession(std::string uuids) { _sessions.erase(uuids); }
