#include "handlers.h"
#include "user_management.h"
#include "logging.h"
#include "sockets.h"
#include <iostream>
#include <string>
#include <vector>
#include <regex.h> // Cambiado de <regex> a <regex.h>

// Función para enviar un mensaje binario a un cliente específico
void send_binary_message(struct lws *wsi, uint8_t message_type, const std::vector<uint8_t>& data) {
    std::vector<uint8_t> buffer;
    buffer.push_back(message_type);
    
    if (!data.empty()) {
        buffer.push_back(data.size());
        buffer.insert(buffer.end(), data.begin(), data.end());
    }
    
    unsigned char out[BUFFER_SIZE];
    memcpy(out + LWS_PRE, buffer.data(), buffer.size());
    int n = lws_write(wsi, out + LWS_PRE, buffer.size(), LWS_WRITE_BINARY);
    if (n < 0) {
        std::cerr << "Error al enviar mensaje binario.\n";
    }
}

// Envía un mensaje a todos los clientes conectados
void broadcast_message(ServerState *state, uint8_t message_type, const std::vector<uint8_t>& data, struct lws *exclude_wsi = nullptr) {
    pthread_mutex_lock(&state->user_mutex);
    
    for (int i = 0; i < state->user_count; i++) {
        if (state->users[i].status != DESACTIVADO && 
            state->users[i].wsi != nullptr &&
            (!exclude_wsi || state->users[i].wsi != exclude_wsi)) {
            send_binary_message(state->users[i].wsi, message_type, data);
        }
    }
    
    pthread_mutex_unlock(&state->user_mutex);
}

void handle_received_message(ServerState *state, struct lws *wsi, uint8_t message_type, uint8_t *data, size_t len) {
    switch (message_type) {
        // Registro de usuario (código 10)
        case 10: {
            std::string username(reinterpret_cast<char*>(data), len);
            std::cout << "Intento de registro para usuario: " << username << "\n";
            log_event("Register Attempt", username.c_str());
            
            // Validar el nombre de usuario
            regex_t regex;
            int reti = regcomp(&regex, "^[A-Za-z0-9_-]{3,16}$", REG_EXTENDED);
            
            if (reti) {
                std::cout << "Error compilando expresión regular\n";
                regfree(&regex);
                
                // Enviar error genérico
                std::vector<uint8_t> error_response = {1}; // Código de error 1
                send_binary_message(wsi, 50, error_response);
                return;
            }
            
            if (regexec(&regex, username.c_str(), 0, NULL, 0) != 0) {
                std::cout << "Nombre de usuario no cumple con el patrón: " << username << "\n";
                regfree(&regex);
                
                // Enviar error de validación
                std::vector<uint8_t> error_response = {1}; // Código de error 1
                send_binary_message(wsi, 50, error_response);
                return;
            }
            
            if (username == "~") {
                std::cout << "Nombre de usuario reservado (~)\n";
                regfree(&regex);
                
                // Enviar error de nombre reservado
                std::vector<uint8_t> error_response = {3}; // Código de error 3
                send_binary_message(wsi, 50, error_response);
                return;
            }
            
            regfree(&regex);
            
            // Intentar registrar usuario
            int result = register_user(state, username.c_str(), "", wsi);
            std::cout << "Resultado del registro: " << result << "\n";
            
            if (result < 0) {
                uint8_t error_code;
                
                if (result == -1) {
                    error_code = 1; // Usuario ya existe
                    std::cout << "Error: Usuario ya existe\n";
                } else if (result == -2) {
                    error_code = 2; // Límite de usuarios
                    std::cout << "Error: Límite de usuarios alcanzado\n";
                } else if (result == -3) {
                    error_code = 3; // Nombre reservado
                    std::cout << "Error: Nombre de usuario reservado\n";
                } else {
                    error_code = 1; // Error genérico
                    std::cout << "Error desconocido: " << result << "\n";
                }
                
                // Enviar error al cliente
                std::vector<uint8_t> error_response = {error_code};
                send_binary_message(wsi, 50, error_response);
            } else {
                std::cout << "Usuario registrado correctamente: " << username << "\n";
                log_event("Register Success", username.c_str());
                
                // Enviar confirmación al cliente que se registró
                std::vector<uint8_t> confirmation;
                
                // Agregar nombre de usuario (para saber que se registró correctamente)
                confirmation.push_back(username.length());
                for (size_t i = 0; i < username.length(); i++) {
                    confirmation.push_back(username[i]);
                }
                
                // Agregar estado (activo)
                confirmation.push_back(ACTIVO);
                
                // Enviar confirmación solo al usuario que se registró
                send_binary_message(wsi, 53, confirmation);
                
                // Notificar a los demás usuarios sobre el nuevo usuario
                broadcast_message(state, 53, confirmation, wsi);
            }
            break;
        }
        
        // El resto del código permanece igual
        // ...
        
        // Listar usuarios conectados
        case 1: {
            // Verificar que el usuario esté registrado
            bool user_registered = false;
            std::string username;
            
            pthread_mutex_lock(&state->user_mutex);
            for (int i = 0; i < state->user_count; i++) {
                if (state->users[i].wsi == wsi) {
                    user_registered = true;
                    username = state->users[i].username;
                    break;
                }
            }
            pthread_mutex_unlock(&state->user_mutex);
            
            if (!user_registered) {
                std::cout << "Usuario no registrado intentando listar usuarios\n";
                
                // Enviar error de usuario no registrado
                std::vector<uint8_t> error_response = {5}; // Código de error 5: Usuario no registrado
                send_binary_message(wsi, 50, error_response);
                break;
            }
            
            std::cout << "Usuario " << username << " solicitó lista de usuarios\n";
            log_event("List Users", "Solicitud de lista de usuarios");
            
            // Preparar respuesta con lista de usuarios (tipo 51)
            std::vector<uint8_t> response;
            response.push_back(state->user_count); // Número de usuarios
            
            pthread_mutex_lock(&state->user_mutex);
            for (int i = 0; i < state->user_count; i++) {
                // Añadir nombre de usuario
                size_t username_len = strlen(state->users[i].username);
                response.push_back(username_len);
                for (size_t j = 0; j < username_len; j++) {
                    response.push_back(state->users[i].username[j]);
                }
                // Añadir estado
                response.push_back(state->users[i].status);
            }
            pthread_mutex_unlock(&state->user_mutex);
            
            send_binary_message(wsi, 51, response);
            break;
        }
        
        // Obtener usuario por nombre
        case 2: {
            // Verificar que el usuario esté registrado
            bool user_registered = false;
            
            pthread_mutex_lock(&state->user_mutex);
            for (int i = 0; i < state->user_count; i++) {
                if (state->users[i].wsi == wsi) {
                    user_registered = true;
                    break;
                }
            }
            pthread_mutex_unlock(&state->user_mutex);
            
            if (!user_registered) {
                std::cout << "Usuario no registrado intentando obtener información\n";
                
                // Enviar error de usuario no registrado
                std::vector<uint8_t> error_response = {5}; // Código de error 5: Usuario no registrado
                send_binary_message(wsi, 50, error_response);
                break;
            }
            
            std::string username(reinterpret_cast<char*>(data), len);
            std::cout << "Petición para obtener información del usuario: " << username << "\n";
            log_event("Get User", username.c_str());
            
            User* user = find_user_by_name(state, username.c_str());
            
            if (user) {
                // Usuario encontrado - preparar respuesta (tipo 52)
                std::vector<uint8_t> response;
                
                // Añadir nombre de usuario
                response.push_back(strlen(user->username));
                for (size_t i = 0; i < strlen(user->username); i++) {
                    response.push_back(user->username[i]);
                }
                
                // Añadir estado
                response.push_back(user->status);
                
                send_binary_message(wsi, 52, response);
            } else {
                // Usuario no encontrado - enviar error (tipo 50)
                uint8_t error_code = 1; // El usuario que intentas obtener no existe
                std::vector<uint8_t> error_response = {error_code};
                send_binary_message(wsi, 50, error_response);
            }
            break;
        }
        
        // Resto de los casos sin cambios...
        case 3: // Cambiar estado de usuario
        case 4: // Enviar mensaje
        case 5: // Obtener historial de mensajes
            // Código existente para estos casos...
            break;
            
        default: {
            std::cout << "Tipo de mensaje desconocido: " << (int)message_type << "\n";
            log_event("Unknown Message", "Tipo de mensaje no soportado");
            break;
        }
    }
}

void handle_client_disconnection(ServerState *state, struct lws *wsi) {
    std::cout << "Cliente desconectado.\n";
    
    pthread_mutex_lock(&state->user_mutex);
    
    bool found = false;
    std::string username;
    
    for (int i = 0; i < state->user_count; i++) {
        if (state->users[i].wsi == wsi) {
            username = state->users[i].username;
            state->users[i].status = DESACTIVADO;
            state->users[i].wsi = nullptr;
            found = true;
            break;
        }
    }
    
    pthread_mutex_unlock(&state->user_mutex);
    
    if (found) {
        std::cout << "Usuario " << username << " desconectado.\n";
        log_event("Disconnect", username.c_str());
        
        // Notificar a todos sobre el cambio de estado (tipo 54)
        std::vector<uint8_t> notification;
        
        // Añadir nombre de usuario
        notification.push_back(username.length());
        for (size_t i = 0; i < username.length(); i++) {
            notification.push_back(username[i]);
        }
        
        // Añadir estado (desactivado)
        notification.push_back(DESACTIVADO);
        
        broadcast_message(state, 54, notification, nullptr);
    } else {
        std::cout << "Desconexión de cliente no registrado.\n";
    }
}
