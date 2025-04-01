#ifndef MENSAJES_H
#define MENSAJES_H

#include "Conexion.h"
#include <string>
#include <vector>

class Mensajes {
private:
    Conexion* conexion;
    
    std::vector<uint8_t> formatearMensaje(uint8_t tipo, const std::vector<uint8_t>& contenido);
    
public:
    Mensajes(Conexion* conn);
    
    // Métodos según el protocolo
    bool solicitarListaUsuarios();
    bool obtenerUsuario(const std::string& nombreUsuario);
    bool cambiarEstado(uint8_t nuevoEstado);
    bool enviarMensaje(const std::string& mensaje);
    bool enviarMensajeA(const std::string& destinatario, const std::string& mensaje);
    bool solicitarHistorial(const std::string& chat);
    
    // Método especial para registro (no en el protocolo original)
    bool registrar(const std::string& nombreUsuario);
};

#endif
