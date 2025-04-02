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
#define PROTOCOL_NAME "ws-protocol"

ServerState server_state;

static int websocket_callback(struct lws *wsi, enum lws_callback_reasons reason,
                             void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED: {
            // aceptar la conexión sin validar el nombre de usuario
            log_event("Connection", "Nueva conexión WebSocket establecida");
            std::cout << "Nueva conexión establecida. Esperando mensaje de registro..." << std::endl;
            break;
        }

        case LWS_CALLBACK_RECEIVE: {
            if (len < 1) break;

            uint8_t message_type = ((uint8_t *)in)[0];
            
            // Para mensajes sin datos adicionales (como tipo 1 - listar usuarios)
            if (len == 1) {
                handle_received_message(&server_state, wsi, message_type, NULL, 0);
                break;
            }
            
            // Para mensajes con datos adicionales
            if (len < 2) {
                log_event("Protocol Error", "Tamaño de mensaje inválido");
                break;
            }
            
            uint8_t data_size = ((uint8_t *)in)[1];
            
            if (len < static_cast<size_t>(2 + data_size)) {
                log_event("Protocol Error", "Tamaño de mensaje inválido");
                return 1; // Cerramos conexión
            }

            uint8_t *payload = (uint8_t *)in + 2;
            
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
        log_event("Server Error", "No se pudo crear contexto de WebSockets");
        return;
    }

    log_event("Server Start", "Servidor WebSockets iniciado en puerto 9000");
    std::cout << "Servidor WebSockets iniciado en puerto " << port << "\n";
    std::cout << "Esperando conexiones...\n";

    while (1) {
        lws_service(context, 50);
    }

    lws_context_destroy(context);
}
