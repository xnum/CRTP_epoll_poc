#include "server_handler.hpp"

#include <iostream>

using namespace std;

server_handler::server_handler(net_io* io) : io_(io) {}

void server_handler::init(size_t visit_idx) {
  visit_idx_ = visit_idx;

  io_->listen(visit_idx_, "srv_lan_eth0", "192.168.8.8", 12345);
}

void server_handler::on_accecpted(int fd) {
  cout << "server_handler: on_accecpted " << fd << endl;
}

void server_handler::on_connected(int fd) {
  cout << "server_handler: on_connected " << fd << endl;
}

void server_handler::on_disconnected(int fd) {
  cout << "server_handler: on_disconnected " << fd << endl;
}

void server_handler::on_failed(int fd) {
  cout << "server_handler: on_failed " << fd << endl;
}

int server_handler::on_read(int client_fd, const char* buf, size_t size) {
  cout << "server_handler: serving request from client " << client_fd << endl;
  cout << "    the request is processing: " << buf << endl;

  // service_->queueRequest(request_id, client_fd);
  return size;
}
