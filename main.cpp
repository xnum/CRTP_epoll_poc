#include "net.hpp"
#include "server_handler.hpp"
#include "user_handler.hpp"

int main() {
  handler_visitor<user_handler*, server_handler*> visitor;

  net_io io;

  user_handler uh(&io);
  server_handler sh(&io);

  auto idx = visitor.add_handler(&uh);
  uh.init(idx);

  idx = visitor.add_handler(&sh);
  sh.init(idx);

  io.run(visitor);

  return 0;
}
