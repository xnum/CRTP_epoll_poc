#include "net.hpp"

#include <cstdio>
#include <cstring>

static int fake_fd = 3;

mock_event_loop::mock_event_loop() {
  // 3 is the upstream server to handle request.
  events_.push_back({.fd = 3, .type = Connect});

  // 4 is public interface to let users submit request.
  events_.push_back({.fd = 4, .type = Accept, .new_fd = 5});
  events_.push_back({.fd = 4, .type = Accept, .new_fd = 6});
  events_.push_back({.fd = 4, .type = Accept, .new_fd = 7});

  // they may failed.
  events_.push_back({.fd = 5, .type = Hup});
  events_.push_back({.fd = 6, .type = Err});

  // or sends request.
  events_.push_back({.fd = 7, .type = Read, .data = "there is my request"});

  // once the upstream server completes request, it sends notification to us.
  events_.push_back({.fd = 3, .type = Read, .data = "completed"});
}

int mock_event_loop::wait(event* eve) {
  if (events_.empty()) return 0;

  event e = events_.front();
  events_.pop_front();
  *eve = e;
  data_ = e.data;

  return 1;
}

size_t mock_event_loop::recv_data(int fd, char* buf, size_t sz) {
  strncpy(buf, data_.c_str(), sz);

  return sz;
}

bool net_io::listen(size_t visit_idx, const char* interface,
                    const char* server_ip, uint16_t server_port) {
  int fd = fake_fd++;

  printf("net_io::listen(): create socket(fd = %d) and bind to interface(%s)\n",
         fd, interface);
  printf("net_io::listen(): listen on (%s:%d) and adds to epoll\n", server_ip,
         server_port);

  add_mapping(fd, visit_idx);

  return true;
}

bool net_io::connect(size_t visit_idx, const char* interface,
                     const char* server_ip, uint16_t server_port,
                     uint16_t local_port) {
  int fd = fake_fd++;

  printf(
      "net_io::connect(): create socket(fd = %d) and bind to "
      "interface(%s:%d)\n",
      fd, interface, local_port);

  printf("net_io::connect(): init connection to (%s:%d) add adds to epoll\n",
         server_ip, server_port);

  add_mapping(fd, visit_idx);

  return true;
}

void net_io::close(int fd) {
  printf("close(fd = %d)\n", fd);

  remove_mapping(fd);
}

int net_io::write(int fd, const char* buf, size_t sz) {
  printf("write to fd(%d): %s\n", fd, buf);

  return 0;
}

void net_io::handle_accept(int listen_fd, int fd) {
  size_t idx = fd_mapping_[listen_fd];
  add_mapping(fd, idx);
}

void net_io::add_mapping(int fd, size_t idx) { fd_mapping_[fd] = idx; }

void net_io::remove_mapping(int fd) { fd_mapping_.erase(fd); }
