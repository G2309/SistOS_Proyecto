#ifndef MENSAJES_H
#define MENSAJES_H

#include "Conexion.h"
#include <string>
#include <vector>
#include <queue>
#include <pthread.h>
#include <atomic>
#include <memory>

// Estructura para almacenar mensajes en la cola
struct MensajeTarea {
    uint8_t tipo;
    std::vector<uint8_t> contenido;
    std::atomic<bool>* completado;
    std::atomic<bool>* resultado;
};

class Mensajes {
private:
    Conexion* conexion;
    
    // Cola de mensajes pendientes
    std::queue<std::shared_ptr<MensajeTarea>> colaMensajes;
    
    // Variables para threads
    pthread_t threadEnvio;
    pthread_mutex_t mutexCola;
    pthread_cond_t condCola;
    std::atomic<bool> threadActivo;
    
    // Método para formatear mensajes
    std::vector<uint8_t> formatearMensaje(uint8_t tipo, const std::vector<uint8_t>& contenido);
    
    // Método de thread worker
    static void* threadTrabajador(void* arg);
    
    // Añadir mensaje a la cola y esperar resultado
    bool encolarMensaje(uint8_t tipo, const std::vector<uint8_t>& contenido);
    
public:
    Mensajes(Conexion* conn);
    ~Mensajes();
    
    // Iniciar y detener thread
    bool iniciarThread();
    void detenerThread();
    
    // Métodos según el protocolo
    bool solicitarListaUsuarios();
    bool obtenerUsuario(const std::string& nombreUsuario);
    bool cambiarEstado(uint8_t nuevoEstado);
    bool enviarMensaje(const std::string& mensaje);
    bool enviarMensajeA(const std::string& destinatario, const std::string& mensaje);
    bool solicitarHistorial(const std::string& chat);
    bool registrarUsuario(const std::string& username);
    
    // Método especial para registro (no en el protocolo original)
    bool registrar(const std::string& nombreUsuario);
};

#endif
