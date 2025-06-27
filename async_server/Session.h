#pragma once
#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <string>

using boost::asio::ip::tcp;

// 单个客户端会话，负责异步读写
class Session : public std::enable_shared_from_this<Session> {
 public:
  explicit Session(boost::asio::io_context& ioc) : _socket(ioc) {}
  tcp::socket& Socket() { return _socket; }
  void Start(); // 启动异步读
  // 生成并返回一个新的uuid字符串
  std::shared_ptr<std::string> Get_uuid() {
    boost::uuids::random_generator gen;
    auto uuids = gen();
    auto res = boost::uuids::to_string(uuids);
    return std::make_shared<std::string>(res);
  }

 private:
  // 异步读回调
  void HandRead(boost::system::error_code error, size_t bit,
                std::shared_ptr<Session> shared_Session);
  // 异步写回调
  void HandWrite(boost::system::error_code error, std::shared_ptr<Session> shared_Session);

  static const int maxsize = 1024;
  char data[maxsize];  // 数据缓冲区
  tcp::socket _socket; // 会话socket
};

// 简单异步服务器，负责监听和管理会话
class Server {
 public:
  Server(boost::asio::io_context& ioc, short port);

 private:
  void Start_accept(); // 开始异步接受连接
  void Handle_accept(std::shared_ptr<Session> new_session, boost::system::error_code error);

  boost::asio::io_context& _ioc;
  tcp::acceptor _acceptor;
  std::map<std::string, std::shared_ptr<Session>> _mq; // 会话管理
};
