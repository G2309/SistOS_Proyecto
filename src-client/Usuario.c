#include "Usuario.h"

Usuario::Usuario() : nombre(""), estado(EstadoUsuario::DISPONIBLE) {}

Usuario::Usuario(const std::string& nombre) : nombre(nombre), estado(EstadoUsuario::DISPONIBLE) {}

void Usuario::setNombre(const std::string& nombre) {
    if (!nombre.empty()) {
        this->nombre = nombre;
    } else {
        std::cerr << "El nombre no puede ser vacío." << std::endl;
    }
}

std::string Usuario::getNombre() const {
    return nombre;
}

void Usuario::setEstado(EstadoUsuario estado) {
    this->estado = estado;
}

EstadoUsuario Usuario::getEstado() const {
    return estado;
}

// Convertir el estado en texto legible
std::string Usuario::estadoToString() const {
    switch (estado) {
        case EstadoUsuario::DISPONIBLE: return "disponible";
        case EstadoUsuario::AUSENTE: return "ausente";
        case EstadoUsuario::OCUPADO: return "ocupado";
        default: return "desconocido";
    }
}

std::string Usuario::toString() const {
    return "Usuario: " + nombre + " | Estado: " + estadoToString();
}
