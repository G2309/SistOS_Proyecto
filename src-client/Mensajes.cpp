#include "Mensajes.h"
#include <iostream>
#include <vector>
#include <cstring> 

Mensajes::Mensajes(Conexion* conn) : conexion(conn) {}

std::vector<uint8_t> Mensajes::formatearMensaje(uint8_t tipo, const std::vector<uint8_t>& contenido) {
    std::vector<uint8_t> mensaje;
    mensaje.push_back(tipo); 

    if (!contenido.empty()) {
        mensaje.push_back(static_cast<uint8_t>(contenido.size()));
        mensaje.insert(mensaje.end(), contenido.begin(), contenido.end());
    }

    return mensaje;
}

bool Mensajes::solicitarListaUsuarios() {
    std::vector<uint8_t> msgBinario = { 0x01 };  // Tipo 1 para listar usuarios
    return conexion->enviar(msgBinario);
}

bool Mensajes::obtenerUsuario(const std::string& nombreUsuario) {
    std::vector<uint8_t> contenido(nombreUsuario.begin(), nombreUsuario.end());
    std::vector<uint8_t> msgBinario = formatearMensaje(0x02, contenido);
    return conexion->enviar(msgBinario);
}

bool Mensajes::cambiarEstado(uint8_t nuevoEstado) {
    if (nuevoEstado > 3) {
        std::cerr << "Estado inválido. Debe ser 0 (DESACTIVADO), 1 (ACTIVO), 2 (OCUPADO), o 3 (INACTIVO)" << std::endl;
        return false;
    }
    
    // En el protocolo, debemos enviar primero el nombre del usuario
    std::vector<uint8_t> datos;
    
    // Obtenemos nuestro nombre de usuario desde la conexión
    std::string nombreUsuario = conexion->getUsuario();
    
    // Añadimos el nombre de usuario
    datos.push_back(static_cast<uint8_t>(nombreUsuario.size()));
    datos.insert(datos.end(), nombreUsuario.begin(), nombreUsuario.end());
    
    // Añadimos el estado
    datos.push_back(nuevoEstado);
    
    // Enviar mensaje tipo 3 (cambiar estado)
    std::vector<uint8_t> msgBinario;
    msgBinario.push_back(0x03); // Tipo 3 para cambiar estado
    
    // NO añadir tamaño del contenido, enviamos directamente los datos
    msgBinario.insert(msgBinario.end(), datos.begin(), datos.end());
    
    return conexion->enviar(msgBinario);
}

bool Mensajes::enviarMensaje(const std::string& mensaje) {
    // Enviamos al chat general (~)
    return enviarMensajeA("~", mensaje);
}

bool Mensajes::enviarMensajeA(const std::string& destinatario, const std::string& mensaje) {
    std::vector<uint8_t> contenido;
    
    // Añadimos el destinatario
    contenido.push_back(static_cast<uint8_t>(destinatario.size()));
    contenido.insert(contenido.end(), destinatario.begin(), destinatario.end());
    
    // Añadimos el mensaje
    contenido.push_back(static_cast<uint8_t>(mensaje.size()));
    contenido.insert(contenido.end(), mensaje.begin(), mensaje.end());
    
    std::vector<uint8_t> msgBinario = formatearMensaje(0x04, contenido);
    return conexion->enviar(msgBinario);
}

bool Mensajes::solicitarHistorial(const std::string& chat) {
    std::vector<uint8_t> contenido(chat.begin(), chat.end());
    std::vector<uint8_t> msgBinario = formatearMensaje(0x05, contenido);
    return conexion->enviar(msgBinario);
}

// Método de registro (este es especial, no está en el protocolo original)
bool Mensajes::registrar(const std::string& nombreUsuario) {
    std::vector<uint8_t> contenido(nombreUsuario.begin(), nombreUsuario.end());
    std::vector<uint8_t> msgBinario = formatearMensaje(0x0A, contenido); // 0x0A = 10 en decimal
    return conexion->enviar(msgBinario);
}

bool Mensajes::registrarUsuario(const std::string& username) {
    std::vector<uint8_t> contenido(username.begin(), username.end());
    std::vector<uint8_t> msgBinario = formatearMensaje(0x0A, contenido); // 0x0A = 10 (Código para registro)
    return conexion->enviar(msgBinario);
}
