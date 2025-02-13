#include "Server.h"
#include <iostream>
using boost::asio::ip::tcp;
Server::Server(boost::asio::io_context& ioc, short port) :_ioc(ioc),
_acceptor(ioc, tcp::endpoint(tcp::v4(), port)) {
    start_accept();
}

void Server::start_accept() {
    auto new_session = std::make_shared <Session>(_ioc, this);
    _acceptor.async_accept(new_session->Socket(),
        std::bind(&Server::handle_accept, this, new_session, std::placeholders::_1));
}
void Server::handle_accept(std::shared_ptr<Session> new_session, const boost::system::error_code& error) {
    if (!error) {
        new_session->Start();
        _sessions.insert(std::make_pair(new_session->Get_uuid(), new_session));
    }
    else {
        delete this;
    }

    start_accept();
}
void Server::ClearSession(std::string uuids) {
    _sessions.erase(uuids);
}

