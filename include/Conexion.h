#ifndef CONEXION_H
#define CONEXION_H

#include <libwebsockets.h>
#include <string>
#include <thread>
#include <atomic>
#include <vector>

class Conexion {
private:
    struct lws_context* contexto;
    struct lws* websocket;
    std::string servidor;
    int puerto;
    std::string usuario;
    std::atomic<bool> conectado;
    std::thread hilo_escucha;
    
    static int callback(struct lws* wsi, enum lws_callback_reasons reason, void* user,
                        void* in, size_t len);
    void escuchar();

public:
    Conexion();
    ~Conexion();

    bool conectar(const std::string& ip, int puerto, const std::string& username);
	bool enviar(const std::vector<uint8_t>& mensaje);
    void cerrar();
    bool estaConectado() const;
};

#endif 
