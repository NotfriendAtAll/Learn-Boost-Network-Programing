#include "Session.h"
#include "Server.h"
#include <iostream>
std::string& Session::Get_uuid() {
    return uuid;
}

void Session::Start() {
    memset(_data, 0, max_length);
    _socket.async_read_some(boost::asio::buffer(_data, max_length),
        std::bind(&Session::handle_read, this, std::placeholders::_1,std::placeholders::_2,shared_from_this() )
    );
}
void Session::handle_read(const boost::system::error_code& error, size_t  bytes_transferred, std::shared_ptr<Session> _self_shared) {
    if (!error) {
       std::cout << "read data is " << _data <<std::endl;
        //·¢ËÍÊý¾Ý
        Send(_data, bytes_transferred);
        memset(_data, 0, max_length);
        _socket.async_read_some(boost::asio::buffer(_data, max_length), std::bind(&Session::handle_read, this,
            std::placeholders::_1, std::placeholders::_2, _self_shared));
    }
    else {
        std::cout << "handle read failed, error is " << error.what() << std::endl;
        _server->ClearSession(uuid);
    }
}

void Session::handle_write(const boost::system::error_code& error, std::shared_ptr<Session> new_session) {
    if (!error) {
        std::lock_guard<std::mutex> lock(_send_lock);
        _send_que.pop();
        if (!_send_que.empty()) {
            auto & msgnodes = _send_que.front();
            boost::asio::async_write(_socket, boost::asio::buffer(msgnodes->_data, msgnodes->_max_len),
                std::bind(&Session::handle_write, this, std::placeholders::_1, new_session));
        }
    }
    else {
        std::cout << "handle write failed, error is " << error.what() << std::endl;
        _server->ClearSession(uuid);
    }
}
void Session::Send(char* msg, int max_length) {
    bool pending = false;
    std::lock_guard<std::mutex> lock(_send_lock);
    if (_send_que.size() > 0) {
        pending = true;
    }
    _send_que.push(std::make_shared<MsgNode>(msg, max_length));
    if (pending) {
        return;
    }
    boost::asio::async_write(_socket, boost::asio::buffer(msg, max_length),
        std::bind(&Session::handle_write, this, std::placeholders::_1, shared_from_this()));
}