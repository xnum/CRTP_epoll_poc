#include "user_handler.hpp"

#include <iostream>

using namespace std;

user_handler::user_handler(net_io* io) : io_(io) {}

void user_handler::init(size_t visit_idx) {
  visit_idx_ = visit_idx;

  io_->connect(visit_idx_, "user_lan_eth0", "1.2.3.4", 8080, 9999);
}

void user_handler::on_connected(int fd) {
  cout << "user_handler: on_connected " << fd << endl;
}

void user_handler::on_disconnected(int fd) {
  cout << "user_handler: on_disconnected " << fd << endl;
}

void user_handler::on_failed(int fd) {
  cout << "user_handler: on_failed " << fd << endl;
}

int user_handler::on_read(int conn_id, const char* buf, size_t size) {
  cout << "user_handler: on_read " << conn_id << endl;
  cout << "    response from upstream: " << buf << endl;

  int client_fd = 7;
  const char response[] = "succeed";
  // service_->queryRequestBelongsToWhichClient(conn_id, buf, &client_fd,
  // &response);

  io_->write(client_fd, response, sizeof(response));

  return size;
}
