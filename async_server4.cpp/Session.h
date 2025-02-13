#pragma once
#include <boost/asio.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <map>
#include <string>
#include <iostream>
#include <queue>
using boost::asio::ip::tcp;
const short MAX_LENGTH = 1024 * 2;
const short HEAD_LENGTH=2;


class Server;
class MsgNode;
class MsgNode
{
    friend class Session;
public:
    MsgNode(char* msg, short max_len) :_total_len(max_len + HEAD_LENGTH), _cur_len(0) {
        _data = new char[_total_len + 1]();
        memcpy(_data, &max_len, HEAD_LENGTH);
        memcpy(_data + HEAD_LENGTH, msg, max_len);
        _data[_total_len] = '\0';
    }

    MsgNode(short max_len) :_total_len(max_len), _cur_len(0) {
        _data = new char[_total_len + 1]();
    }

    ~MsgNode() {
        delete[] _data;
    }

    void Clear() {
        ::memset(_data, 0, _total_len);
        _cur_len = 0;
    }
private:
    short _cur_len;
    short _total_len;
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
    void Close() {
        _socket.close();
        _b_close = true;
    }
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
    //收到的消息结构
    std::shared_ptr<MsgNode> _recv_msg_node;
    bool _b_head_parse;
    //收到的头部结构
    bool _b_close;
    std::shared_ptr<MsgNode> _recv_head_node;
};



