#include "handlers.h"
#include "user_management.h"
#include "logging.h"
#include "sockets.h"
#include <iostream>
#include <string>
#include <vector>
#include <regex.h> 

// Función para enviar un mensaje binario a un cliente específico
void sendBinaryMessage(struct lws *wsi, uint8_t message_type, const std::vector<uint8_t>& data) {
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

// Función para enviar un mensaje a todos los clientes
void broadcast_message(ServerState *state, uint8_t message_type, const std::vector<uint8_t>& data, struct lws *exclude_wsi) {
    pthread_mutex_lock(&state->user_mutex);
    
    for (int i = 0; i < state->user_count; i++) {
        if (state->users[i].status != DESACTIVADO && 
            state->users[i].wsi != nullptr &&
            (!exclude_wsi || state->users[i].wsi != exclude_wsi)) {
            sendBinaryMessage(state->users[i].wsi, message_type, data);
        }
    }
    
    pthread_mutex_unlock(&state->user_mutex);
}

void notify_status_change(ServerState* state, const char* username, UserStatus new_status) {
    std::vector<uint8_t> notification;
    
    // Añadir nombre de usuario
    size_t username_len = strlen(username);
    notification.push_back(username_len);
    for (size_t i = 0; i < username_len; i++) {
        notification.push_back(username[i]);
    }
    
    // Añadir estado
    notification.push_back(new_status);
    
    broadcast_message(state, 54, notification, nullptr);
}

// Añade esta implementación a handlers.cpp

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

void handle_received_message(ServerState *state, struct lws *wsi, uint8_t message_type, uint8_t *data, size_t len) {
		bool need_status_notification = false;
    	std::string username_for_notification;
    	
    	pthread_mutex_lock(&state->user_mutex);
    	for (int i = 0; i < state->user_count; i++) {
        	if (state->users[i].wsi == wsi) {
            	// Actualizar tiempo de actividad
            	state->users[i].last_active = time(NULL);
            	
            	// Si estaba inactivo, cambiarlo a activo y notificar
            	if (state->users[i].status == INACTIVO) {
                	state->users[i].status = ACTIVO;
                	need_status_notification = true;
                	username_for_notification = state->users[i].username;
            	}
            	break;
        	}
    	}
    	pthread_mutex_unlock(&state->user_mutex);
    	
    	// Si es necesario, notificar el cambio de estado
    	if (need_status_notification) {
        	std::vector<uint8_t> notification;
        	
        	// Añadir nombre de usuario
        	notification.push_back(username_for_notification.length());
        	for (size_t i = 0; i < username_for_notification.length(); i++) {
            	notification.push_back(username_for_notification[i]);
        	}
        	
        	// Añadir estado (ACTIVO = 1)
        	notification.push_back(ACTIVO);
        	
        	broadcast_message(state, 54, notification, nullptr);
    	}
    switch (message_type) {

		// Actualizar tiempo de actividad para ir llevando los cambios de estados automaticos, de activo a ocupado cuando se escribe, etc.

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
                sendBinaryMessage(wsi, 50, error_response);
                return;
            }
            
            if (regexec(&regex, username.c_str(), 0, NULL, 0) != 0) {
                std::cout << "Nombre de usuario no cumple con el patrón: " << username << "\n";
                regfree(&regex);
                
                // Enviar error de validación
                std::vector<uint8_t> error_response = {1}; // Código de error 1
                sendBinaryMessage(wsi, 50, error_response);
                return;
            }
            
            if (username == "~") {
                std::cout << "Nombre de usuario reservado (~)\n";
                regfree(&regex);
                
                // Enviar error de nombre reservado
                std::vector<uint8_t> error_response = {3}; // Código de error 3
                sendBinaryMessage(wsi, 50, error_response);
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
                sendBinaryMessage(wsi, 50, error_response);
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
                sendBinaryMessage(wsi, 53, confirmation);
                
                // Notificar a los demás usuarios sobre el nuevo usuario
                broadcast_message(state, 53, confirmation, wsi);
            }
            break;
        }
        
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
                sendBinaryMessage(wsi, 50, error_response);
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
            
            sendBinaryMessage(wsi, 51, response);
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
                sendBinaryMessage(wsi, 50, error_response);
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
                
                sendBinaryMessage(wsi, 52, response);
            } else {
                // Usuario no encontrado - enviar error (tipo 50)
                uint8_t error_code = 1; // El usuario que intentas obtener no existe
                std::vector<uint8_t> error_response = {error_code};
                sendBinaryMessage(wsi, 50, error_response);
            }
            break;
        }
        
        // Cambiar estado de usuario
        case 3: {
            if (len < 2) {
                log_event("Protocol Error", "Datos insuficientes para cambiar estado");
                break;
            }
            
            size_t username_len = data[0];
            if (username_len + 1 > len) {
                log_event("Protocol Error", "Tamaño de mensaje inválido en cambio de estado");
                break;
            }
            
            std::string username(reinterpret_cast<char*>(data + 1), username_len);
            uint8_t new_status = data[username_len + 1];
            
            std::cout << "Petición para cambiar estado del usuario: " << username << " a " << (int)new_status << "\n";
            
            if (new_status > 3) { // Verificar que el estado sea válido por que solo tenemos 4 estados
                // Estado inválido - enviar error (tipo 50) [por el protocolo]
                uint8_t error_code = 2; // Estado inválido
                std::vector<uint8_t> error_response = {error_code};
                sendBinaryMessage(wsi, 50, error_response);
                break;
            }
            
            int result = change_user_status(state, username.c_str(), (UserStatus)new_status);
            
            if (result == 0) {
                // Éxito - notificar a todos los usuarios (tipo 54)
                std::vector<uint8_t> notification;
                
                // Añadir nombre de usuario
                notification.push_back(username.length());
                for (size_t i = 0; i < username.length(); i++) {
                    notification.push_back(username[i]);
                }
                
                // Añadir nuevo estado
                notification.push_back(new_status);
                
                broadcast_message(state, 54, notification, nullptr); // Notificar a todos
            } else {
                // Error - usuario no encontrado
                uint8_t error_code = 1; // El usuario no existe
                std::vector<uint8_t> error_response = {error_code};
                sendBinaryMessage(wsi, 50, error_response);
            }
            break;
        }
        
        // Enviar mensaje
        case 4: {
            if (len < 1) {
                log_event("Protocol Error", "Datos insuficientes para enviar mensaje");
                break;
            }
            
            size_t dest_username_len = data[0];
            if (dest_username_len + 1 >= len) {
                log_event("Protocol Error", "Tamaño de mensaje inválido en envío de mensaje");
                break;
            }
            
            std::string dest_username(reinterpret_cast<char*>(data + 1), dest_username_len);
            
            size_t message_len = data[dest_username_len + 1];
            if (dest_username_len + 2 + message_len > len) {
                log_event("Protocol Error", "Tamaño de mensaje inválido");
                break;
            }
            
            std::string message(reinterpret_cast<char*>(data + dest_username_len + 2), message_len);
            
            std::cout << "Enviar mensaje a: " << dest_username << ", Mensaje: " << message << "\n";
            
            if (message.empty()) {
                // Mensaje vacío - enviar error (tipo 50)
                uint8_t error_code = 3; // El mensaje está vacío
                std::vector<uint8_t> error_response = {error_code};
                sendBinaryMessage(wsi, 50, error_response);
                break;
            }
            
            // Obtener usuario que envía el mensaje
            std::string sender_username;
            pthread_mutex_lock(&state->user_mutex);
            for (int i = 0; i < state->user_count; i++) {
                if (state->users[i].wsi == wsi) {
                    sender_username = state->users[i].username;
                    state->users[i].last_active = time(NULL); // Actualizar tiempo activo
                    break;
                }
            }
            pthread_mutex_unlock(&state->user_mutex);
            
            if (sender_username.empty()) {
                log_event("Protocol Error", "No se pudo determinar el remitente");
                break;
            }
            
            // Chat general (usando "~")
            if (dest_username == "~") {
                // Preparar notificación de mensaje (tipo 55)
                std::vector<uint8_t> notification;
                
                // Añadir nombre del remitente
                notification.push_back(sender_username.length());
                for (size_t i = 0; i < sender_username.length(); i++) {
                    notification.push_back(sender_username[i]);
                }
                
                // Añadir mensaje
                notification.push_back(message.length());
                for (size_t i = 0; i < message.length(); i++) {
                    notification.push_back(message[i]);
                }
                
                broadcast_message(state, 55, notification, nullptr); // Notificar a todos
            } else {
                // Mensaje directo
                User* dest_user = find_user_by_name(state, dest_username.c_str());
                
                if (!dest_user) {
                    // Destinatario no existe - enviar error (tipo 50)
                    uint8_t error_code = 1; // El usuario no existe
                    std::vector<uint8_t> error_response = {error_code};
                    sendBinaryMessage(wsi, 50, error_response);
                    break;
                }
                
                if (dest_user->status == DESACTIVADO) {
                    // Destinatario desconectado - enviar error (tipo 50)
                    uint8_t error_code = 4; // Usuario desconectado
                    std::vector<uint8_t> error_response = {error_code};
                    sendBinaryMessage(wsi, 50, error_response);
                    break;
                }
                
                // Preparar notificación de mensaje (tipo 55)
                std::vector<uint8_t> notification;
                
                // Añadir nombre del remitente
                notification.push_back(sender_username.length());
                for (size_t i = 0; i < sender_username.length(); i++) {
                    notification.push_back(sender_username[i]);
                }
                
                // Añadir mensaje
                notification.push_back(message.length());
                for (size_t i = 0; i < message.length(); i++) {
                    notification.push_back(message[i]);
                }
                
                // Enviar al destinatario
                sendBinaryMessage(dest_user->wsi, 55, notification);
                
                // Enviar también al remitente (confirmación)
                sendBinaryMessage(wsi, 55, notification);
            }
            break;
        }
        
        // Obtener historial de mensajes
        case 5: {
            std::string chat_name(reinterpret_cast<char*>(data), len);
            std::cout << "Petición para obtener historial de mensajes con: " << chat_name << "\n";
            
            // Aquí implementaríamos el acceso al historial de mensajes
            // Por ahora, solo enviamos una respuesta vacía
            std::vector<uint8_t> response = {0}; // 0 mensajes
            sendBinaryMessage(wsi, 56, response);
            break;
        }
            
        default: {
            std::cout << "Tipo de mensaje desconocido: " << (int)message_type << "\n";
            log_event("Unknown Message", "Tipo de mensaje no soportado");
            break;
        }
    }
}
