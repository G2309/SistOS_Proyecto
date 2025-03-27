#ifndef SOCKETS_H
#define SOCKETS_H

#include <libwebsockets.h>
#include <stdint.h>
#include <vector>

#define BUFFER_SIZE 1024

void startServer(int port);
void sendBinaryMessage(struct lws *wsi, uint8_t type, const std::vector<uint8_t>& data);

#endif

