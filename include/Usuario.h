#ifndef USUARIO_H
#define USUARIO_H

#include <string>

enum class EstadoUsuario {
    DISPONIBLE,
    AUSENTE,
    OCUPADO
};

class Usuario {
private:
    std::string nombre;
    EstadoUsuario estado; 

public:
    Usuario();
    Usuario(const std::string& nombre);

    // Métodos de acceso
    void setNombre(const std::string& nombre);
    std::string getNombre() const;

    // Métodos de estado
    void setEstado(EstadoUsuario estado);
    EstadoUsuario getEstado() const; 

    std::string toString() const;

    std::string estadoToString() const;
};

#endif
