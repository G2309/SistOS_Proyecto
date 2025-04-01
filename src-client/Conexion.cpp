#include "Conexion.h"
#include <iostream>
#include <cstring>
#include <vector>

static int callback_websocket(struct lws* wsi, enum lws_callback_reasons reason, void* user,
                              void* in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            std::cout << "Conexión establecida con el servidor WebSocket.\n";
            break;
        case LWS_CALLBACK_CLIENT_RECEIVE:
            std::cout << "Mensaje recibido: " << std::string((char*)in, len) << "\n";
            break;
        case LWS_CALLBACK_CLIENT_CLOSED:
            std::cout << "Conexión cerrada.\n";
            break;
        default:
            break;
    }
    return 0;
}

Conexion::Conexion() : contexto(nullptr), websocket(nullptr), conectado(false) {}

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
    contexto = lws_create_context(&info);
    if (!contexto) {
        std::cerr << "Error al crear el contexto de WebSockets." << std::endl;
        return false;
    }
    
    std::string url = "/?name=" + username;
    struct lws_client_connect_info ccinfo = {};
    ccinfo.context = contexto;
    ccinfo.address = ip.c_str();
    ccinfo.port = puerto;
    ccinfo.path = url.c_str();
    ccinfo.host = ccinfo.address;
    ccinfo.origin = ccinfo.address;
    ccinfo.protocol = protocols[0].name;
    ccinfo.ietf_version_or_minus_one = -1;
    ccinfo.userdata = this;
    
    websocket = lws_client_connect_via_info(&ccinfo);
    if (!websocket) {
        std::cerr << "Error al conectar con el servidor WebSocket." << std::endl;
        return false;
    }
    conectado = true;
    
    hilo_escucha = std::thread([this]() { escuchar(); });
    return true;
}

bool Conexion::enviar(const std::vector<uint8_t>& mensaje) {
    if (!conectado || !websocket) return false;
    
    size_t tam = mensaje.size();
    unsigned char buffer[LWS_PRE + tam];
    std::memcpy(&buffer[LWS_PRE], mensaje.data(), tam);
    
    return lws_write(websocket, &buffer[LWS_PRE], tam, LWS_WRITE_BINARY) > 0;
}


void Conexion::cerrar() {
    if (conectado) {
        conectado = false;
        lws_context_destroy(contexto);
        if (hilo_escucha.joinable()) hilo_escucha.join();
    }
}

bool Conexion::estaConectado() const {
    return conectado;
}

void Conexion::escuchar() {
    while (conectado) {
        lws_service(contexto, 50);
    }
}

