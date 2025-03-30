#ifndef MENSAJES_H
#define MENSAJES_H

#include <string>
#include "Conexion.h"

class Mensajes {
private:
    Conexion* conexion;  // Referencia a la conexión activa
public:
    Mensajes(Conexion* conn);

    bool enviarMensaje(const std::string& mensaje);
    bool enviarPrivado(const std::string& destinatario, const std::string& mensaje);
    bool cambiarEstado(const std::string& estado);
    std::string recibirMensaje();

    std::string formatearMensaje(const std::string& tipo, const std::string& contenido);
};

#endif
