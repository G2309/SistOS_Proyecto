#ifndef HANDLERS_H
#define HANDLERS_H

#include <libwebsockets.h>
#include <stdint.h>
#include <vector>

void handleReceivedMessage(struct lws *wsi, uint8_t messageType, const std::vector<uint8_t>& data);
void handleClientDisconnection(struct lws *wsi);

#endif
