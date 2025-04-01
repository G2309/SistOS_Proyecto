#ifndef MENSAJES_H
#define MENSAJES_H

#include "Conexion.h"
#include <vector>
#include <string>

class Mensajes {
private:
    Conexion* conexion;

public:
    Mensajes(Conexion* conn);

    bool enviarMensaje(const std::string& mensaje); 
    bool enviarNombreUsuario(const std::string& mensaje); 
    bool enviarPrivado(const std::string& destinatario, const std::string& mensaje);
    bool cambiarEstado(const std::string& estado);
    void solicitarListaUsuarios();
    void solicitarInfoUsuario(const std::string& nombre);

private:
    std::vector<uint8_t> formatearMensaje(uint8_t tipo, const std::vector<uint8_t>& contenido);
};

#endif  // MENSAJES_H

