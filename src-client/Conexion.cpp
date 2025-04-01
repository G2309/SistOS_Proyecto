#include "Conexion.h"
#include <iostream>
#include <cstring>
#include <vector>

static int callback_websocket(struct lws* wsi, enum lws_callback_reasons reason, void* user,
                            void* in, size_t len) {
    Conexion* conn = static_cast<Conexion*>(user);
    
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            std::cout << "Conexión establecida con el servidor WebSocket.\n";
            if (conn) conn->setEstablecido(true);
            break;
            
        case LWS_CALLBACK_CLIENT_RECEIVE: {
            if (!conn || len < 1) return 0;
            
            uint8_t message_type = ((uint8_t *)in)[0];
            
            std::cout << "Mensaje recibido. Tipo: " << (int)message_type << "\n";
            
            // Procesar los diferentes tipos de mensajes según el protocolo
            switch (message_type) {
                case 50: { // ERROR
                    if (len < 2) break;
                    uint8_t error_code = ((uint8_t *)in)[1];
                    std::string error_msg;
                    
                    switch (error_code) {
                        case 1: error_msg = "El usuario que intentas obtener no existe"; break;
                        case 2: error_msg = "El estatus enviado es inválido"; break;
                        case 3: error_msg = "¡El mensaje está vacío!"; break;
                        case 4: error_msg = "El mensaje fue enviado a un usuario desconectado"; break;
                        default: error_msg = "Error desconocido";
                    }
                    
                    std::cout << "Error: " << error_msg << "\n";
                    break;
                }
                
                case 51: { // Respuesta: Listar usuarios
                    if (len < 2) break;
                    uint8_t user_count = ((uint8_t *)in)[1];
                    std::cout << "Usuarios conectados (" << (int)user_count << "):\n";
                    
                    size_t offset = 2;
                    for (int i = 0; i < user_count && offset < len; i++) {
                        uint8_t username_len = ((uint8_t *)in)[offset++];
                        if (offset + username_len >= len) break;
                        
                        std::string username((char*)in + offset, username_len);
                        offset += username_len;
                        
                        if (offset >= len) break;
                        uint8_t status = ((uint8_t *)in)[offset++];
                        
                        std::string status_str;
                        switch (status) {
                            case 0: status_str = "DESACTIVADO"; break;
                            case 1: status_str = "ACTIVO"; break;
                            case 2: status_str = "OCUPADO"; break;
                            case 3: status_str = "INACTIVO"; break;
                            default: status_str = "DESCONOCIDO";
                        }
                        
                        std::cout << "- " << username << " (" << status_str << ")\n";
                    }
                    break;
                }
                
                case 52: { // Respuesta: Obtener usuario por nombre
                    if (len < 2) break;
                    uint8_t username_len = ((uint8_t *)in)[1];
					if (2 + static_cast<size_t>(username_len) >= len) break;
                    
                    std::string username((char*)in + 2, username_len);
                    uint8_t status = ((uint8_t *)in)[2 + username_len];
                    
                    std::string status_str;
                    switch (status) {
                        case 0: status_str = "DESACTIVADO"; break;
                        case 1: status_str = "ACTIVO"; break;
                        case 2: status_str = "OCUPADO"; break;
                        case 3: status_str = "INACTIVO"; break;
                        default: status_str = "DESCONOCIDO";
                    }
                    
                    std::cout << "Información de usuario: " << username << " (" << status_str << ")\n";
                    break;
                }
                
                case 53: { // Usuario registrado
                    if (len < 2) break;
                    uint8_t username_len = ((uint8_t *)in)[1];
					if (2 + static_cast<size_t>(username_len) >= len) break;
                    
                    std::string username((char*)in + 2, username_len);
                    uint8_t status = ((uint8_t *)in)[2 + username_len];
                    
                    std::string status_str;
                    switch (status) {
                        case 0: status_str = "DESACTIVADO"; break;
                        case 1: status_str = "ACTIVO"; break;
                        case 2: status_str = "OCUPADO"; break;
                        case 3: status_str = "INACTIVO"; break;
                        default: status_str = "DESCONOCIDO";
                    }
                    
                    std::cout << "¡Nuevo usuario registrado! " << username << " (" << status_str << ")\n";
                    break;
                }
                
                case 54: { // Usuario cambió estado
                    if (len < 2) break;
                    uint8_t username_len = ((uint8_t *)in)[1];
					if (2 + static_cast<size_t>(username_len) >= len) break;
                    
                    std::string username((char*)in + 2, username_len);
                    uint8_t status = ((uint8_t *)in)[2 + username_len];
                    
                    std::string status_str;
                    switch (status) {
                        case 0: status_str = "DESACTIVADO"; break;
                        case 1: status_str = "ACTIVO"; break;
                        case 2: status_str = "OCUPADO"; break;
                        case 3: status_str = "INACTIVO"; break;
                        default: status_str = "DESCONOCIDO";
                    }
                    
                    std::cout << "Usuario " << username << " cambió estado a " << status_str << "\n";
                    break;
                }
                
                case 55: { // Recibió mensaje
                    if (len < 2) break;
                    uint8_t username_len = ((uint8_t *)in)[1];
					if (2 + static_cast<size_t>(username_len) >= len) break;
                    
                    std::string username((char*)in + 2, username_len);
                    
					if (2 + static_cast<size_t>(username_len) >= len) break;
                    uint8_t message_len = ((uint8_t *)in)[2 + username_len];
                    
                    std::string message((char*)in + 2 + username_len + 1, message_len);
                    
                    std::cout << username << ": " << message << "\n";
                    break;
                }
                
                case 56: { // Historial de mensajes
                    if (len < 2) break;
                    uint8_t message_count = ((uint8_t *)in)[1];
                    std::cout << "Historial de mensajes (" << (int)message_count << " mensajes):\n";
                    
                    size_t offset = 2;
                    for (int i = 0; i < message_count && offset < len; i++) {
                        uint8_t username_len = ((uint8_t *)in)[offset++];
                        if (offset + username_len >= len) break;
                        
                        std::string username((char*)in + offset, username_len);
                        offset += username_len;
                        
                        if (offset >= len) break;
                        uint8_t message_len = ((uint8_t *)in)[offset++];
                        if (offset + message_len > len) break;
                        
                        std::string message((char*)in + offset, message_len);
                        offset += message_len;
                        
                        std::cout << username << ": " << message << "\n";
                    }
                    break;
                }
                
                default:
                    std::cout << "Mensaje de tipo desconocido: " << (int)message_type << "\n";
            }
            break;
        }
            
        case LWS_CALLBACK_CLIENT_CLOSED:
            std::cout << "Conexión cerrada.\n";
            if (conn) conn->setConectado(false);
            break;
            
        default:
            break;
    }
    return 0;
}

Conexion::Conexion() : contexto(nullptr), websocket(nullptr), conectado(false), establecido(false) {}

Conexion::~Conexion() {
    cerrar();
}

bool Conexion::conectar(const std::string& ip, int puerto, const std::string& username) {
    servidor = ip;
    this->puerto = puerto;
    usuario = username;
    
    struct lws_protocols protocols[] = {
        {"chat-protocol", callback_websocket, 0, 4096},
        {nullptr, nullptr, 0, 0}
    };
    
    struct lws_context_creation_info info = {};
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
    info.user = this;
    contexto = lws_create_context(&info);
    if (!contexto) {
        std::cerr << "Error al crear el contexto de WebSockets." << std::endl;
        return false;
    }
    
    std::string path = "/?name=" + username;
    struct lws_client_connect_info ccinfo = {};
    ccinfo.context = contexto;
    ccinfo.address = ip.c_str();
    ccinfo.port = puerto;
    ccinfo.path = path.c_str();
    ccinfo.host = ccinfo.address;
    ccinfo.origin = ccinfo.address;
    ccinfo.protocol = protocols[0].name;
    ccinfo.userdata = this;
    
    websocket = lws_client_connect_via_info(&ccinfo);
    if (!websocket) {
        std::cerr << "Error al conectar con el servidor WebSocket." << std::endl;
        return false;
    }
    conectado = true;
    
    hilo_escucha = std::thread([this]() { escuchar(); });
    
    // Esperar hasta que la conexión esté establecida o falle
    int timeout = 30; // 3 segundos (100ms * 30)
    while (timeout > 0 && !establecido) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        timeout--;
    }
    
    return establecido;
}

bool Conexion::enviar(const std::vector<uint8_t>& mensaje) {
    if (!conectado || !websocket) return false;
    
    size_t tam = mensaje.size();
    unsigned char buffer[LWS_PRE + tam];
    std::memcpy(&buffer[LWS_PRE], mensaje.data(), tam);
    
    int ret = lws_write(websocket, &buffer[LWS_PRE], tam, LWS_WRITE_BINARY);
    return ret > 0;
}

void Conexion::cerrar() {
    if (conectado) {
        conectado = false;
        establecido = false;
        if (contexto) {
            lws_context_destroy(contexto);
            contexto = nullptr;
        }
        if (hilo_escucha.joinable()) hilo_escucha.join();
    }
}

bool Conexion::estaConectado() const {
    return conectado && establecido;
}

void Conexion::setConectado(bool estado) {
    conectado = estado;
}

void Conexion::setEstablecido(bool estado) {
    establecido = estado;
}

void Conexion::escuchar() {
    while (conectado) {
        lws_service(contexto, 50);
    }
}
