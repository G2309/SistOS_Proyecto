#include "Conexion.h"
#include <iostream>
#include <cstring>
#include <vector>
#include <iomanip>

// Función para depurar mensajes binarios
void debug_binary_message(uint8_t* data, size_t len) {
    std::cout << "DEBUG - Mensaje binario (" << len << " bytes): ";
    for (size_t i = 0; i < len; i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]) << " ";
    }
    std::cout << std::dec << std::endl;
    
    // Intentar mostrar versión ASCII para detectar cadenas
    std::cout << "DEBUG - ASCII: ";
    for (size_t i = 0; i < len; i++) {
        if (isprint(data[i])) {
            std::cout << (char)data[i];
        } else {
            std::cout << ".";
        }
    }
    std::cout << std::endl;
}

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
            
            // Depurar el mensaje completo (para todos los tipos)
            debug_binary_message((uint8_t *)in, len);
            
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
    				std::cout << "DEBUG - Decodificando mensaje tipo 51 (lista usuarios)" << std::endl;
    				
    				// Señalizar inicio de lista de usuarios para limpiar la UI
    				std::cout << "INICIO_LISTA_USUARIOS" << std::endl;
    				
    				// Si el mensaje es demasiado corto, terminamos
    				if (len < 3) {
        				std::cout << "Error: Mensaje tipo 51 demasiado corto" << std::endl;
        				std::cout << "FIN_LISTA_USUARIOS" << std::endl;
        				break;
    				}
    				
    				// Según el formato visto en la depuración:
    				// byte 0: tipo de mensaje (51)
    				// byte 1: longitud de los datos (no es el número de usuarios)
    				// byte 2: número real de usuarios
    				uint8_t data_len = ((uint8_t *)in)[1];
    				uint8_t user_count = ((uint8_t *)in)[2];
    				
    				std::cout << "Usuarios conectados (" << (int)user_count << "):" << std::endl;
    				
    				// Inicio de datos de usuarios (después del número de usuarios)
    				size_t offset = 3;
    				
    				// Procesamos cada usuario
    				for (int i = 0; i < user_count && offset < len; i++) {
        				// Obtener longitud del nombre de usuario
        				if (offset >= len) break;
        				uint8_t username_len = ((uint8_t *)in)[offset++];
        				
        				// Verificar que hay suficientes bytes para el nombre
        				if (offset + username_len > len) {
            				std::cout << "Error: No hay suficientes bytes para el nombre del usuario " << i+1 << std::endl;
            				break;
        				}
        				
        				// Extraer el nombre de usuario como string
        				std::string username((char*)in + offset, username_len);
        				offset += username_len;
        				
        				// Verificar que hay un byte más para el estado
        				if (offset >= len) {
            				std::cout << "Error: No hay suficientes bytes para el estado del usuario " << i+1 << std::endl;
            				break;
        				}
        				
        				uint8_t status = ((uint8_t *)in)[offset++];
        				
        				// Convertir estado a texto
        				std::string status_str;
        				switch (status) {
            				case 0: status_str = "DESACTIVADO"; break;
            				case 1: status_str = "ACTIVO"; break;
            				case 2: status_str = "OCUPADO"; break;
            				case 3: status_str = "INACTIVO"; break;
            				default: status_str = "DESCONOCIDO";
        				}
        				
        				// Mostrar información del usuario
        				std::cout << "- " << username << " (" << status_str << ")" << std::endl;
    				}
    				
    				// Señalizar fin de lista de usuarios
    				std::cout << "FIN_LISTA_USUARIOS" << std::endl;
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
    				// Hacer el parseo más simple y directo
    				if (len < 3) break;
    				
    				// Análisis de la estructura del mensaje
    				uint8_t username_len = ((uint8_t *)in)[2];
    				
    				if (3 + username_len >= len) break;
    				
    				// Extraer el nombre de usuario
    				std::string username((char*)in + 3, username_len);
    				
    				if (3 + username_len + 1 >= len) break;
    				
    				// Extraer la longitud del mensaje
    				uint8_t message_len = ((uint8_t *)in)[3 + username_len];
    				
    				if (3 + username_len + 1 + message_len > len) break;
    				
    				// Extraer el mensaje
    				std::string message((char*)in + 3 + username_len + 1, message_len);
    				
    				// Imprimir una línea clara y simple para que la UI la procese
    				std::cout << "MENSAJE_CHAT: " << username << ": " << message << std::endl;
    				break;
				}

				case 56: { // Historial de mensajes
    				std::cout << "DEBUG - Procesando mensaje tipo 56 (historial de mensajes)" << std::endl;
    				debug_binary_message((uint8_t *)in, len);
    				
    				if (len < 2) break;
    				uint8_t message_count = ((uint8_t *)in)[1];
    				std::cout << "Historial de mensajes (" << (int)message_count << " mensajes):\n";
    				
    				size_t offset = 2;
    				for (int i = 0; i < message_count && offset < len; i++) {
        				if (offset >= len) break;
        				uint8_t username_len = ((uint8_t *)in)[offset++];
        				
        				if (offset + username_len > len) {
            				std::cout << "ERROR - Historial: datos insuficientes para el nombre de usuario" << std::endl;
            				break;
        				}
        				
        				std::string username((char*)in + offset, username_len);
        				offset += username_len;
        				
        				if (offset >= len) {
            				std::cout << "ERROR - Historial: datos insuficientes para la longitud del mensaje" << std::endl;
            				break;
        				}
        				
        				uint8_t message_len = ((uint8_t *)in)[offset++];
        				
        				if (offset + message_len > len) {
            				std::cout << "ERROR - Historial: datos insuficientes para el contenido del mensaje" << std::endl;
            				break;
        				}
        				
        				std::string message((char*)in + offset, message_len);
        				offset += message_len;
        				
        				// Mostrar información detallada
        				std::cout << "HISTORIAL: " << username << ": " << message << "\n";
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
    // protocolo
    struct lws_protocols protocols[] = {
        {"ws-protocol", callback_websocket, 0, 4096},
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
    // string path
    std::string path = "?name=" + username;
    std::cout << "Path a enviar: " << path << std::endl;
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
    int timeout = 30; 
    while (timeout > 0 && !establecido) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        timeout--;
    }

    if (!establecido) {
        std::cerr << "Tiempo de espera agotado para establecer la conexión." << std::endl;
        cerrar();
        return false;
    }
    
    // Ahora registramos el usuario enviando un mensaje tipo 10
    std::vector<uint8_t> contenido(username.begin(), username.end());
    std::vector<uint8_t> mensajeRegistro;
    mensajeRegistro.push_back(10); // Tipo 10 = registro
    mensajeRegistro.push_back(username.length());
    mensajeRegistro.insert(mensajeRegistro.end(), contenido.begin(), contenido.end());
    
    std::cout << "Enviando mensaje de registro para usuario: " << username << std::endl;
    if (!enviar(mensajeRegistro)) {
        std::cerr << "Error al enviar mensaje de registro" << std::endl;
        cerrar();
        return false;
    }
    
    std::cout << "Mensaje de registro enviado." << std::endl;
    
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

std::string Conexion::getUsuario() const {
    return usuario;
}

void Conexion::escuchar() {
    while (conectado) {
        lws_service(contexto, 50);
    }
}
