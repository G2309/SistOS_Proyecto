#include "handlers.h"
#include "user_management.h"
#include "logging.h"
#include "sockets.h"
#include <iostream>
#include <string>

void handle_received_message(ServerState *state, struct lws *wsi, uint8_t message_type, uint8_t *data, size_t len) {
    switch (message_type) {
		// Registro de usuario
        case 1: { 
            std::string username(reinterpret_cast<char*>(data), len);
			// Recordar agregar luego el manejo de contraseña :p
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

