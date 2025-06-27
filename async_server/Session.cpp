#include "Session.h"

// 会话开始，启动异步读
void Session::Start() {
  memset(data, 0, maxsize);
  // 绑定 shared_from_this，保证 Session 生命周期
  auto self = std::shared_ptr<Session>(this, [](Session*) {}); // 占位，不实际析构
  _socket.async_read_some(boost::asio::buffer(data, maxsize),
                          std::bind(&Session::HandRead, shared_from_this(), std::placeholders::_1,
                                    std::placeholders::_2, shared_from_this()));
}

// 异步读回调
void Session::HandRead(boost::system::error_code error, size_t bytes,
                       std::shared_ptr<Session> shared_Session) {
  if (!error) {
    std::cout << "the reply data is: " << std::string(data, bytes) << std::endl;
    // 只回写收到的有效数据
    boost::asio::async_write(_socket, boost::asio::buffer(data, bytes),
                             std::bind(&Session::HandWrite, shared_from_this(),
                                       std::placeholders::_1, shared_from_this()));
  } else {
    std::cout << "Read Error: " << error.message() << '\n';
    // 不要delete this，智能指针会自动管理
  }
}

// 异步写回调
void Session::HandWrite(boost::system::error_code error, std::shared_ptr<Session> shared_Session) {
  if (!error) {
    memset(data, 0, maxsize);
    _socket.async_read_some(boost::asio::buffer(data, maxsize),
                            std::bind(&Session::HandRead, shared_from_this(), std::placeholders::_1,
                                      std::placeholders::_2, shared_from_this()));
  } else {
    std::cout << "Write Error: " << error.message() << '\n';
    // 不要delete this，智能指针会自动管理
  }
}

// 服务器构造，初始化监听
Server::Server(boost::asio::io_context& ioc, short port)
    : _ioc(ioc), _acceptor(_ioc, tcp::endpoint(tcp::v4(), port)) {
  Start_accept();
}

// 开始异步接受连接
void Server::Start_accept() {
  auto new_session = std::make_shared<Session>(_ioc);
  _acceptor.async_accept(new_session->Socket(), std::bind(&Server::Handle_accept, this, new_session,
                                                          std::placeholders::_1));
}

// 新连接回调
void Server::Handle_accept(std::shared_ptr<Session> new_session, boost::system::error_code error) {
  if (!error) {
    new_session->Start();
    // 可选：将新会话加入管理队列
    // auto uuid = *(new_session->Get_uuid());
    // _mq[uuid] = new_session;
  } else {
    std::cout << "Connect failed: " << error.message() << std::endl;
  }
  Start_accept();
}
