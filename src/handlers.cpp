#include "handlers.h"
#include "user_management.h"
#include "logging.h"
#include "sockets.h"
#include <iostream>
#include <string>

// ¡Ahora usamos ServerState y raw data como espera server.cpp!
void handle_received_message(ServerState *state, struct lws *wsi, uint8_t message_type, uint8_t *data, size_t len) {
    switch (message_type) {
        case 1: { // Registro de usuario
            std::string username(reinterpret_cast<char*>(data), len);

            // ⚠️ Como no estás manejando password aún, lo pasamos vacío y -1 como socket_fd
            if (register_user(state, username.c_str(), "", -1) == 0) {
                std::cout << "Usuario registrado: " << username << "\n";
            } else {
                std::cout << "Error al registrar usuario: " << username << "\n";
            }
            break;
        }

    }
}

void handle_client_disconnection(ServerState *state, struct lws *wsi) {
    std::cout << "Cliente desconectado.\n";

}

