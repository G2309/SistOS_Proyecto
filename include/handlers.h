#ifndef HANDLERS_H
#define HANDLERS_H
#include <libwebsockets.h>
#include <stdint.h>
#include <vector>
#include "user_management.h"

// Necesitamos estas declaraciones con nombres consistentes
void sendBinaryMessage(struct lws *wsi, uint8_t message_type, const std::vector<uint8_t>& data);
void broadcast_message(ServerState *state, uint8_t message_type, const std::vector<uint8_t>& data, struct lws *exclude_wsi = nullptr);

// Esta debe coincidir con la implementación
void handle_received_message(ServerState *state, struct lws *wsi, uint8_t message_type, uint8_t *data, size_t len);
void handle_client_disconnection(ServerState *state, struct lws *wsi);


#endif
