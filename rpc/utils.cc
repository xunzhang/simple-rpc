#include <utility>

#include <fcntl.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"

using namespace std;

namespace rpc {

int set_nonblocking(int fd, bool nonblocking) {
    int ret = fcntl(fd, F_GETFL, 0);
    if (ret != -1) {
        if (nonblocking) {
            ret = fcntl(fd, F_SETFL, ret | O_NONBLOCK);
        } else {
            ret = fcntl(fd, F_SETFL, ret & ~O_NONBLOCK);
        }
    }
    return ret;
}

int find_open_port() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);

  addrinfo *local_addr;

  addrinfo hints;
  bzero(&hints, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if (getaddrinfo("localhost", NULL, NULL, &local_addr) != 0) {
    Log_fatal("Failed to getaddrinfo");
  }

  int port = -1;

  for (int i = 1024; i < 65000; ++i) {
    ((sockaddr_in*)local_addr->ai_addr)->sin_port = i;
    if (::bind(fd, local_addr->ai_addr, local_addr->ai_addrlen) != 0) {
      continue;
    }

    sockaddr_in addr;
    socklen_t addrlen;
    if (getsockname(fd, (sockaddr*)&addr, &addrlen) != 0) {
      Log_fatal("Failed to get socket address");
    }

    port = i;
    break;
  }

  freeaddrinfo(local_addr);
  ::close(fd);

  if (port != -1) {
    Log_info("Found open port: %d", port);
    return port;
  }

  Log_fatal("Failed to find open port.");
  return -1;
}

std::string get_host_name() {
  char buffer[1024];
  if (gethostname(buffer, 1024) != 0) {
    Log_fatal("Failed to get hostname.");
  }
  return std::string(buffer);
}

}
