#pragma once

#include <cstdint>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

template <typename T>
class socket_handler {
 public:
  void on_accecpted(int fd) { return as_underlying().on_accecpted(fd); }

  void on_connected(int fd) { return as_underlying().on_connected(fd); }

  void on_disconnected(int fd) { return as_underlying().on_disconnected(fd); }

  void on_failed(int fd) { return as_underlying().on_failed(fd); }

  int on_read(int fd, const char *buf, size_t size) {
    return as_underlying().on_read(fd, buf, size);
  }

 private:
  friend T;
  socket_handler() = default;

  inline T &as_underlying() { return static_cast<T &>(*this); }
};

template <typename... Ts>
class handler_visitor {
 public:
  template <typename T>
  size_t add_handler(T h) {
    handlers_.push_back({h});
    return handlers_.size() - 1;
  }

  void emit_accecpted(size_t idx, int fd) {
    auto &handler = handlers_[idx];

    std::visit([&](auto &h) { h->on_accecpted(fd); }, handler);
  }

  void emit_connected(size_t idx, int fd) {
    auto &handler = handlers_[idx];

    std::visit([&](auto &h) { h->on_connected(fd); }, handler);
  }

  void emit_disconnected(size_t idx, int fd) {
    auto &handler = handlers_[idx];

    std::visit([&](auto &h) { h->on_disconnected(fd); }, handler);
  }

  void emit_failed(size_t idx, int fd) {
    auto &handler = handlers_[idx];

    std::visit([&](auto &h) { h->on_failed(fd); }, handler);
  }

  void emit_read(size_t idx, int fd, const char *buf, size_t sz) {
    auto &handler = handlers_[idx];

    std::visit([&](auto &h) { h->on_read(fd, buf, sz); }, handler);
  }

 private:
  using poly_T = std::variant<Ts...>;
  std::vector<poly_T> handlers_;
};

enum Type { Accept, Connect, Read, Hup, Err };

struct event {
  int fd;
  int type;

  int new_fd;
  // pretending buf of NIC to let caller recv.
  std::string data;
};

class mock_event_loop {
 public:
  mock_event_loop();

  // waits only one event per call.
  int wait(event *);
  size_t recv_data(int fd, char *buf, size_t sz);

 private:
  std::deque<event> events_;
  std::string data_;
};

class net_io {
 public:
  bool listen(size_t visit_idx, const char *interface, const char *server_ip,
              uint16_t server_port);
  bool connect(size_t visit_idx, const char *interface, const char *server_ip,
               uint16_t server_port, uint16_t local_port = 0);
  void close(int fd);
  int write(int fd, const char *buf, size_t size);

  template <typename Visitor>
  void run(Visitor &visitor) {
    mock_event_loop loop;
    while (1) {
      event e;
      if (!loop.wait(&e)) break;

      printf("event fd:%d type:%d\n", e.fd, e.type);

      if (e.type == Accept) {
        handle_accept(e.fd, e.new_fd);

        auto idx = fd_mapping_[e.fd];
        visitor.emit_accecpted(idx, e.new_fd);
      }

      if (e.type == Connect) {
        auto idx = fd_mapping_[e.fd];
        visitor.emit_connected(idx, e.fd);
      }

      if (e.type == Read) {
        auto sz = loop.recv_data(e.fd, buf_, 256);
        auto idx = fd_mapping_[e.fd];
        visitor.emit_read(idx, e.fd, buf_, sz);
      }

      if (e.type == Hup) {
        auto idx = fd_mapping_[e.fd];
        visitor.emit_disconnected(idx, e.fd);
      }

      if (e.type == Err) {
        auto idx = fd_mapping_[e.fd];
        visitor.emit_failed(idx, e.fd);
      }
    }
  }

 private:
  void handle_accept(int listen_fd, int fd);
  void add_mapping(int fd, size_t idx);
  void remove_mapping(int fd);

  char buf_[256]{};
  std::map<int, size_t> fd_mapping_;
};

