#ifndef CONEXION_H
#define CONEXION_H

#include <string>

class Conexion {
private:
    int socket_fd;
public:
    Conexion();
    ~Conexion();

    bool conectar(const std::string& ip, int puerto);
    bool enviar(const std::string& mensaje);
    std::string recibir();
    void cerrar();

    int getSocket() const;
};

#endif
