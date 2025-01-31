#pragma once
#include <boost/asio.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <map>
#include <string>
#include <iostream>
#include <queue>
using boost::asio::ip::tcp;
class Server;
class MsgNode;
class MsgNode
{
    friend class Session;
public:
    MsgNode(char* msg, int max_len) {
        _data = new char[max_len];
        memcpy(_data, msg, max_len);
    }
    ~MsgNode() {
        delete[] _data;
    }
private:
    int _cur_len;
    int _max_len;
    char* _data;
};
class Session :public std::enable_shared_from_this<Session>
{
public:
    Session(boost::asio::io_context& ioc,Server*server) :_socket(ioc),_server(server) {
        boost::uuids::uuid m_uuid = boost::uuids::random_generator()();
        uuid = boost::uuids::to_string(m_uuid);
    }
    tcp::socket& Socket() {
        return _socket;
    }
    std::string& Get_uuid();
    void Start();
private:
    void handle_read(const boost::system::error_code& error, size_t bytes_transfered, std::shared_ptr<Session> new_session);
    void handle_write(const boost::system::error_code& error, std::shared_ptr<Session> new_session);
    void Send(char* msg, int max_length);
    std::queue<std::shared_ptr<MsgNode> > _send_que;
    std::mutex _send_lock;
    tcp::socket _socket;
    enum { max_length = 1024 };
    char _data[max_length];
    std::string uuid;
    Server* _server;
};



