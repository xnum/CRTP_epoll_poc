#pragma once

#include "net.hpp"

class user_handler : public socket_handler<user_handler> {
 public:
  user_handler(net_io*);
  void init(size_t visit_idx);
  void on_connected(int fd);
  void on_disconnected(int fd);
  void on_failed(int fd);
  int on_read(int conn_id, const char* buf, size_t size);

 private:
  net_io* io_;
  size_t visit_idx_;
};
