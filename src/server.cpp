#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libwebsockets.h>
#include <regex.h>
#include "user_management.h"
#include "logging.h"
#include "handlers.h"
#include <iostream>

#define MAX_PAYLOAD 1024
#define BUFFER_SIZE 2048
#define PROTOCOL_NAME "chat-protocol"

ServerState server_state;

// Funcion del protocolo para envio de mensajes (bin)
void sendBinaryMessage(struct lws *wsi, uint8_t type, const std::vector<uint8_t>& data) {
    std::vector<uint8_t> buffer;
    buffer.push_back(type);
    buffer.push_back(data.size());
    buffer.insert(buffer.end(), data.begin(), data.end());

    unsigned char out[BUFFER_SIZE];
    int n = lws_write(wsi, out + LWS_PRE, buffer.size(), LWS_WRITE_BINARY);
    if (n < 0) {
        std::cerr << "Error al enviar mensaje binario.\n";
    }
}

static int websocket_callback(struct lws *wsi, enum lws_callback_reasons reason,
                              void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            log_event("Connection", "Nueva conexión WebSocket establecida.");
            break;

        case LWS_CALLBACK_RECEIVE: {
            if (len < 2) break;

            uint8_t message_type = ((uint8_t *)in)[0];
            uint8_t data_size = ((uint8_t *)in)[1];

			if (len < static_cast<size_t>(2 + data_size)) {
                log_event("Protocol Error", "Tamaño de mensaje inválido.");
                return 1; // Cerramos conexión
            }

            uint8_t *payload = (uint8_t *)in + 2;

            // Validación de nombre de usuario si es tipo 1 (registro)
            if (message_type == 1) {
                char username[USERNAME_MAX_LEN];
                strncpy(username, (char *)payload, data_size);
                username[data_size] = '\0';

                regex_t regex;
                int reti = regcomp(&regex, "^[A-Za-z0-9_-]{3,16}$", REG_EXTENDED);
                if (reti || regexec(&regex, username, 0, NULL, 0) != 0 || strcmp(username, "~") == 0) {
                    log_event("Validation", "Nombre de usuario inválido.");
                    uint8_t error_code = 1;
                    uint8_t response[] = {50, 1, error_code};
                    lws_write(wsi, response, sizeof(response), LWS_WRITE_BINARY);
                    regfree(&regex);
                    return 1;
                }
                regfree(&regex);
            }

            handle_received_message(&server_state, wsi, message_type, payload, data_size);
            break;
        }

        case LWS_CALLBACK_CLOSED:
            handle_client_disconnection(&server_state, wsi);
            break;

        default:
            break;
    }

    return 0;
}

static struct lws_protocols protocols[] = {
    {
        PROTOCOL_NAME,
        websocket_callback,
        0,
        BUFFER_SIZE,
    },
    { NULL, NULL, 0, 0 }
};

void start_server(int port) {
    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));

    info.port = port;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;

    struct lws_context *context = lws_create_context(&info);
    if (!context) {
        log_event("Server Error", "No se pudo crear contexto de WebSockets.");
        return;
    }

    log_event("Server Start", "Servidor WebSockets iniciado.");

    while (1) {
        lws_service(context, 50);
    }

    lws_context_destroy(context);
}

