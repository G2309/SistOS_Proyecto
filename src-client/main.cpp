#include "Conexion.h"
#include "Mensajes.h"
#include <iostream>
#include <thread>
#include <vector>
#include <string>

void manejarEntrada(Conexion& conexion, Mensajes& mensajes) {
    std::string mensaje;
    while (conexion.estaConectado()) {
        std::cout << "Introduce un mensaje para enviar: ";
        std::getline(std::cin, mensaje);
        if (!mensaje.empty()) {
			if (!mensajes.enviarMensaje(mensaje)) {
				std::cerr << "Error al enviar el mensaje [3]" << std::endl ;
			} else {
				std::cout << "Mensaje enviado" << std::endl;
			}
        }
    }
}

int main() {
    std::string ip = "3.137.199.200";
    int puerto = 9000;
    std::string username;
    
	std::cout << "Introduce tu nombre de usuario: ";
    std::getline(std::cin, username);
    
    Conexion conexion;
    if (!conexion.conectar(ip, puerto, username)) {
        std::cerr << "No se pudo conectar al servidor WebSocket." << std::endl;
        return -1;
    }
    
    std::cout << "Conectado al servidor WebSocket." << std::endl;

	Mensajes mensajes(&conexion);

	if (!mensajes.enviarNombreUsuario(username)) {
		std::cerr << "Error al enviar el nombre del usuario" << std::endl;
		return -1;
	}
    
    // Iniciar el hilo de entrada del usuario
    std::thread hiloEntrada(manejarEntrada, std::ref(conexion), std::ref(mensajes));
    
    // Esperar a que el hilo termine (cuando se cierre la conexión)
    hiloEntrada.join();
    
    // Cerrar la conexión
    conexion.cerrar();
    std::cout << "Conexión cerrada." << std::endl;
    
    return 0;
}
