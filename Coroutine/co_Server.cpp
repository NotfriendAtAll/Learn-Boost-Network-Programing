#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>

using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
using boost::asio::ip::tcp;
namespace this_coro = boost::asio::this_coro;

// 协程：处理单个客户端的echo逻辑
awaitable<void> echo(tcp::socket socket) {
  try {
    char data[1024];
    for (;;) {
      // 异步读取数据
      std::size_t n = co_await socket.async_read_some(boost::asio::buffer(data), use_awaitable);
      // 异步写回收到的数据
      co_await async_write(socket, boost::asio::buffer(data, n), use_awaitable);
    }
  } catch (std::exception& e) {
    std::printf("echo Exception: %s\n", e.what());
  }
}

// 协程：监听端口并接受新连接
awaitable<void> listener() {
  auto executor = co_await this_coro::executor;
  tcp::acceptor acceptor(executor, {tcp::v4(), 10086});
  for (;;) {
    // 异步接受新连接
    tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
    // 为每个连接启动一个echo协程
    co_spawn(executor, echo(std::move(socket)), detached);
  }
}

int main() {
  try {
    boost::asio::io_context io_context(1);

    // 信号处理，支持优雅退出
    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto) { io_context.stop(); });

    // 启动监听协程
    co_spawn(io_context, listener(), detached);

    io_context.run(); // 事件循环
  } catch (std::exception& e) {
    std::printf("Exception: %s\n", e.what());
  }
}