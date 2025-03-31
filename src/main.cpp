#include "Conexion.h"
#include <iostream>
#include <thread>
#include <vector>
#include <string>

void manejarEntrada(Conexion& conexion) {
    std::string mensaje;
    while (conexion.estaConectado()) {
        std::cout << "Introduce un mensaje para enviar: ";
        std::getline(std::cin, mensaje);
        if (!mensaje.empty()) {
            // Convertir string a vector<uint8_t>
            std::vector<uint8_t> datosBinarios(mensaje.begin(), mensaje.end());
            
            // Enviar el vector de bytes
            if (!conexion.enviar(datosBinarios)) {
                std::cerr << "Error al enviar el mensaje." << std::endl;
            } else {
                std::cout << "Mensaje enviado." << std::endl;
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
    
    // Iniciar el hilo de entrada del usuario
    std::thread hiloEntrada(manejarEntrada, std::ref(conexion));
    
    // Esperar a que el hilo termine (cuando se cierre la conexión)
    hiloEntrada.join();
    
    // Cerrar la conexión
    conexion.cerrar();
    std::cout << "Conexión cerrada." << std::endl;
    
    return 0;
}
