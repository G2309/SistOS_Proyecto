#include "Mensajes.h"
#include <iostream>

Mensajes::Mensajes(Conexion* conn) : conexion(conn) {}

bool Mensajes::enviarMensaje(const std::string& mensaje) {
    std::string mensajeFormateado = formatearMensaje("MSG", mensaje);
    return conexion->enviar(mensajeFormateado);
}

bool Mensajes::enviarPrivado(const std::string& destinatario, const std::string& mensaje) {
    std::string contenido = destinatario + ":" + mensaje;
    std::string mensajeFormateado = formatearMensaje("PRIV", contenido);
    return conexion->enviar(mensajeFormateado);
}

bool Mensajes::cambiarEstado(const std::string& estado) {
    std::string mensajeFormateado = formatearMensaje("ESTADO", estado);
    return conexion->enviar(mensajeFormateado);
}

std::string Mensajes::recibirMensaje() {
    return conexion->recibir();
}

std::string Mensajes::formatearMensaje(const std::string& tipo, const std::string& contenido) {
    // Formato simple: [TIPO]|contenido
    return "[" + tipo + "]|" + contenido;
}

void Mensajes::solicitarListaUsuarios() {
    conexion->enviar("/list");
}

void Mensajes::solicitarInfoUsuario(const std::string& nombre) {
    conexion->enviar("/info " + nombre);
}
