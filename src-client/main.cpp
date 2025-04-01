#include "Conexion.h"
#include "Mensajes.h"
#include <iostream>
#include <thread>
#include <vector>
#include <string>

void mostrarMenu() {
    std::cout << "\n--- MENÚ ---\n";
    std::cout << "1. Ver usuarios conectados\n";
    std::cout << "2. Obtener información de un usuario\n";
    std::cout << "3. Cambiar mi estado\n";
    std::cout << "4. Enviar mensaje al chat general\n";
    std::cout << "5. Enviar mensaje privado\n";
    std::cout << "6. Ver historial de mensajes\n";
    std::cout << "7. Salir\n";
    std::cout << "Opción: ";
}

void manejarEntrada(Conexion& conexion, Mensajes& mensajes, const std::string& username) {
    while (conexion.estaConectado()) {
        mostrarMenu();
        int opcion;
        std::cin >> opcion;
        std::cin.ignore(); // Limpiar el buffer
        
        switch (opcion) {
            case 1: {
                // Ver usuarios conectados
                if (!mensajes.solicitarListaUsuarios()) {
                    std::cerr << "Error al solicitar lista de usuarios" << std::endl;
                }
                break;
            }
            
            case 2: {
                // Obtener información de un usuario
                std::string nombreUsuario;
                std::cout << "Ingrese el nombre del usuario: ";
                std::getline(std::cin, nombreUsuario);
                
                if (!mensajes.obtenerUsuario(nombreUsuario)) {
                    std::cerr << "Error al obtener información del usuario" << std::endl;
                }
                break;
            }
            
            case 3: {
                // Cambiar mi estado
                std::cout << "Estados disponibles:\n";
                std::cout << "1. ACTIVO\n";
                std::cout << "2. OCUPADO\n";
                std::cout << "3. INACTIVO\n";
                std::cout << "Seleccione el nuevo estado: ";
                
                uint8_t nuevoEstado;
                std::cin >> nuevoEstado;
                std::cin.ignore(); // Limpiar el buffer
                
                if (nuevoEstado < 1 || nuevoEstado > 3) {
                    std::cerr << "Estado inválido" << std::endl;
                    break;
                }
                
                if (!mensajes.cambiarEstado(nuevoEstado)) {
                    std::cerr << "Error al cambiar estado" << std::endl;
                }
                break;
            }
            
            case 4: {
                // Enviar mensaje al chat general
                std::string mensaje;
                std::cout << "Ingrese el mensaje: ";
                std::getline(std::cin, mensaje);
                
                if (mensaje.empty()) {
                    std::cerr << "El mensaje no puede estar vacío" << std::endl;
                    break;
                }
                
                if (!mensajes.enviarMensaje(mensaje)) {
                    std::cerr << "Error al enviar el mensaje" << std::endl;
                }
                break;
            }
            
            case 5: {
                // Enviar mensaje privado
                std::string destinatario;
                std::cout << "Ingrese el nombre del destinatario: ";
                std::getline(std::cin, destinatario);
                
                std::string mensaje;
                std::cout << "Ingrese el mensaje: ";
                std::getline(std::cin, mensaje);
                
                if (mensaje.empty()) {
                    std::cerr << "El mensaje no puede estar vacío" << std::endl;
                    break;
                }
                
                if (!mensajes.enviarMensajeA(destinatario, mensaje)) {
                    std::cerr << "Error al enviar el mensaje" << std::endl;
                }
                break;
            }
            
            case 6: {
                // Ver historial de mensajes
                std::string chat;
                std::cout << "Ingrese el nombre del chat (o ~ para el chat general): ";
                std::getline(std::cin, chat);
                
                if (!mensajes.solicitarHistorial(chat)) {
                    std::cerr << "Error al solicitar historial" << std::endl;
                }
                break;
            }
            
            case 7: {
                // Salir
                std::cout << "Cerrando conexión..." << std::endl;
                conexion.cerrar();
                return;
            }
            
            default:
                std::cout << "Opción inválida" << std::endl;
        }
        
        // Pequeña pausa para ver las respuestas
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

int main() {
    //std::string ip = "3.137.199.200";
    std::string ip = "127.0.0.1";
    int puerto = 9000;
    std::string username;
    
    std::cout << "Introduce tu nombre de usuario: ";
    std::getline(std::cin, username);
    
    // Validar nombre de usuario
    if (username.empty() || username == "~" || username.length() < 3 || username.length() > 16) {
        std::cerr << "Nombre de usuario inválido. Debe tener entre 3 y 16 caracteres y no puede ser '~'." << std::endl;
        return -1;
    }
    
    Conexion conexion;
    if (!conexion.conectar(ip, puerto, username)) {
        std::cerr << "No se pudo conectar al servidor WebSocket." << std::endl;
        return -1;
    }
    
    std::cout << "Conectado al servidor WebSocket." << std::endl;

    Mensajes mensajes(&conexion);
    
    // Iniciar el hilo de entrada del usuario
    manejarEntrada(conexion, mensajes, username);
    
    // Cerrar la conexión
    conexion.cerrar();
    std::cout << "Conexión cerrada." << std::endl;
    
    return 0;
}
