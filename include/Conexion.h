#ifndef CONEXION_H
#define CONEXION_H

#include <string>
#include <thread>
#include <vector>
#include <libwebsockets.h>

class Conexion {
private:
    std::string servidor;
    int puerto;
    std::string usuario;
    
    struct lws_context* contexto;
    struct lws* websocket;
    bool conectado;
    bool establecido;
    std::thread hilo_escucha;
    
    void escuchar();
    
public:
    Conexion();
    ~Conexion();
    
    bool conectar(const std::string& ip, int puerto, const std::string& username);
    bool enviar(const std::vector<uint8_t>& mensaje);
    void cerrar();
    bool estaConectado() const;
    
    void setConectado(bool estado);
    void setEstablecido(bool estado);
};

#endif
