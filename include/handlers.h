#ifndef HANDLERS_H
#define HANDLERS_H

#include <libwebsockets.h>
#include <stdint.h>
#include <vector>
#include "user_management.h"

void handle_received_message(ServerState *state, struct lws *wsi, uint8_t message_type, uint8_t *data, size_t len);
void handle_client_disconnection(ServerState *state, struct lws *wsi);

#endif
