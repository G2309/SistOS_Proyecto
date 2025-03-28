#include "handlers.h"
#include "user_management.h"
#include "logging.h"
#include "sockets.h"
#include <iostream>

void handleReceivedMessage(struct lws *wsi, uint8_t messageType, const std::vector<uint8_t>& data) {
    // Aquí procesas los diferentes tipos de mensajes (registro, chat, estado, etc.)
    switch (messageType) {
        case 1: { // Registro de usuario
            std::string username(data.begin(), data.end());
            if (register_user(wsi, username.c_str()) == 0) {
                std::cout << "Usuario registrado: " << username << "\n";
                //logInfo("Usuario registrado exitosamente");
            } else {
                std::cout << "Error al registrar usuario: " << username << "\n";
            }
            break;
        }

        // Otros tipos de mensajes aquí...
    }
}

void handleClientDisconnection(struct lws *wsi) {
    std::cout << "Cliente desconectado.\n";
    remove_user(wsi);  // Asumiendo que tienes una función así en user_management
}

