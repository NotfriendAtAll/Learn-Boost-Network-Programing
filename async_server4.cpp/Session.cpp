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

void Session::handle_read(const boost::system::error_code& error, size_t  bytes_transferred, std::shared_ptr<Session> shared_self) {
    if (!error) {
        //已经移动的字符数
        int copy_len = 0;
        while (bytes_transferred > 0) {
            if (!_b_head_parse) {
                //收到的数据不足头部大小
                if (bytes_transferred + _recv_head_node->_cur_len < HEAD_LENGTH) {
                    memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, bytes_transferred);
                    _recv_head_node->_cur_len += bytes_transferred;
                    ::memset(_data, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                        std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, shared_self));
                    return;
                }
                //收到的数据比头部多
                //头部剩余未复制的长度
                int head_remain = HEAD_LENGTH - _recv_head_node->_cur_len;
                memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, head_remain);
                //更新已处理的data长度和剩余未处理的长度
                copy_len += head_remain;
                bytes_transferred -= head_remain;
                //获取头部数据
                short data_len = 0;
                memcpy(&data_len, _recv_head_node->_data, HEAD_LENGTH);
                std::cout << "data_len is " << data_len << std::endl;
                //头部长度非法
                if (data_len > MAX_LENGTH) {
                    std::cout << "invalid data length is " << data_len << std::endl;
                    _server->ClearSession(uuid);
                    return;
                }
                _recv_msg_node = std::make_shared<MsgNode>(data_len);

                //消息的长度小于头部规定的长度，说明数据未收全，则先将部分消息放到接收节点里
                if (bytes_transferred < data_len) {
                    memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
                    _recv_msg_node->_cur_len += bytes_transferred;
                    ::memset(_data, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                        std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, shared_self));
                    //头部处理完成
                    _b_head_parse = true;
                    return;
                }

                memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, data_len);
                _recv_msg_node->_cur_len += data_len;
                copy_len += data_len;
                bytes_transferred -= data_len;
                _recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
                std:: cout << "receive data is " << _recv_msg_node->_data << std::endl;
                //此处可以调用Send发送测试
                Send(_recv_msg_node->_data, _recv_msg_node->_total_len);
                //继续轮询剩余未处理数据
                _b_head_parse = false;
                _recv_head_node->Clear();
                if (bytes_transferred <= 0) {
                    ::memset(_data, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                        std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, shared_self));
                    return;
                }
                continue;
            }

            //已经处理完头部，处理上次未接受完的消息数据
            //接收的数据仍不足剩余未处理的
            int remain_msg = _recv_msg_node->_total_len - _recv_msg_node->_cur_len;
            if (bytes_transferred < remain_msg) {
                memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
                _recv_msg_node->_cur_len += bytes_transferred;
                ::memset(_data, 0, MAX_LENGTH);
                _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                    std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, shared_self));
                return;
            }
            memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, remain_msg);
            _recv_msg_node->_cur_len += remain_msg;
            bytes_transferred -= remain_msg;
            copy_len += remain_msg;
            _recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
            std:: cout << "receive data is " << _recv_msg_node->_data <<std::endl;
            //此处可以调用Send发送测试
            Send(_recv_msg_node->_data, _recv_msg_node->_total_len);
            //继续轮询剩余未处理数据
            _b_head_parse = false;
            _recv_head_node->Clear();
            if (bytes_transferred <= 0) {
                ::memset(_data, 0, MAX_LENGTH);
                _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                    std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, shared_self));
                return;
            }
            continue;
        }
    }
    else {
        std::cout << "handle read failed, error is " << error.what() << std::endl;
        Close();
        _server->ClearSession(uuid);
    }
}
void Session::handle_write(const boost::system::error_code& error, std::shared_ptr<Session> shared_self) {

    if (!error) {
        std::lock_guard<std::mutex> lock(_send_lock);
        std::cout << "send data " << _send_que.front()->_data + HEAD_LENGTH << std::endl;
        _send_que.pop();
        if (!_send_que.empty()) {
            auto& msgnode = _send_que.front();
            boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
                std::bind(&Session::handle_write, this, std::placeholders::_1, shared_self));
        }
    }
    else {
        std::cout << "handle write failed, error is " << error.what() << std::endl;
        Close();
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