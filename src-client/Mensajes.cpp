#include "Mensajes.h"
#include <iostream>
#include <vector>
#include <cstring> 

Mensajes::Mensajes(Conexion* conn) : conexion(conn) {}

std::vector<uint8_t> Mensajes::formatearMensaje(uint8_t tipo, const std::vector<uint8_t>& contenido) {
    std::vector<uint8_t> mensaje;
    mensaje.push_back(tipo); 

    uint8_t tamaño = static_cast<uint8_t>(contenido.size());
    mensaje.push_back(tamaño);

    mensaje.insert(mensaje.end(), contenido.begin(), contenido.end());

    return mensaje;
}

bool Mensajes::enviarNombreUsuario(const std::string& nombreUsuario) {
	std::vector<uint8_t> contenido(nombreUsuario.begin(), nombreUsuario.end());
	std::vector<uint8_t> msgBinario = formatearMensaje(0x01, contenido);
	return conexion-> enviar(msgBinario);
}

bool Mensajes::enviarMensaje(const std::string& mensaje) {
    std::vector<uint8_t> contenido(mensaje.begin(), mensaje.end());
    std::vector<uint8_t> msgBinario = formatearMensaje(0x01, contenido);
    return conexion->enviar(msgBinario);
}

bool Mensajes::enviarPrivado(const std::string& destinatario, const std::string& mensaje) {
    std::vector<uint8_t> contenido;
    
    // Destinatario 
    contenido.push_back(static_cast<uint8_t>(destinatario.size()));
    contenido.insert(contenido.end(), destinatario.begin(), destinatario.end());

    // Mensaje 
    contenido.push_back(static_cast<uint8_t>(mensaje.size()));
    contenido.insert(contenido.end(), mensaje.begin(), mensaje.end());

    std::vector<uint8_t> msgBinario = formatearMensaje(0x02, contenido);
    return conexion->enviar(msgBinario);
}

bool Mensajes::cambiarEstado(const std::string& estado) {
    std::vector<uint8_t> contenido(estado.begin(), estado.end());
    std::vector<uint8_t> msgBinario = formatearMensaje(0x03, contenido);
    return conexion->enviar(msgBinario);
}

void Mensajes::solicitarListaUsuarios() {
    std::vector<uint8_t> msgBinario = { 0x04 };  // Solo el código de mensaje
    conexion->enviar(msgBinario);
}

void Mensajes::solicitarInfoUsuario(const std::string& nombre) {
    std::vector<uint8_t> contenido(nombre.begin(), nombre.end());
    std::vector<uint8_t> msgBinario = formatearMensaje(0x05, contenido);
    conexion->enviar(msgBinario);
}

