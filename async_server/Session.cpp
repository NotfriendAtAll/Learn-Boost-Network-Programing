#include "Session.h"
void Session::Start() {
  memset(data, '\0', maxsize);
  _socket.async_read_some(boost::asio::buffer(data, maxsize),
                          std::bind(&Session::HandRead, this,
                                    std::placeholders::_1,
                                    std::placeholders::_2));
}
void Session::HandRead(boost::system::error_code error, size_t bit,
                       std::shared_ptr<Session> shared_Session) {
  if (!error) {
    std::cout << "the reply data is" << data << std::endl;
    _socket.async_write_some(boost::asio::buffer(data, maxsize),
                             std::bind(&Session::HandWrite, this,
                                       std::placeholders::_1, shared_Session));
  } else {
    std::cout << "Read Error" << '\n';
    delete this;
  }
}
void Session::HandWrite(boost::system::error_code error,
                        std::shared_ptr<Session> shared_Session) {
  if (!error) {
    memset(data, '\0', maxsize);
    _socket.async_read_some(boost::asio::buffer(data, maxsize),
                            std::bind(&Session::HandRead, this,
                                      std::placeholders::_1,
                                      std::placeholders::_2, shared_Session));
  } else {
    std::cout << "Read Error" << '\n';
    delete this;
  }
}

Server::Server(boost::asio::io_context &ioc, short port)
    : _ioc(ioc), _acceptor(_ioc, tcp::endpoint(tcp::v4(), port)) {
  Start_accept();
}

void Server::Start_accept() {
  std::shared_ptr<Session> new_session = std::make_shared<Session>(_ioc);
  // Session* new_session = new	Session(_ioc);
  _acceptor.async_accept(new_session->Socket(),
                         std::bind(&Server::Handle_accept, this, new_session,
                                   std::placeholders::_1));
}
void Server::Handle_accept(std::shared_ptr<Session> new_session,
                           boost::system::error_code error) {
  if (!error) {
    new_session->Start();
  } else {
    delete new_session;
    std::cout << "Connect failed";
  }
  Start_accept();
}
