#include <iostream>
#include <string>
#include "Conexion.h"
#include "Mensajes.h"
#include "Usuario.h"
#include "Utilidades.h"

int main() {
    // Crear la conexión y el objeto Mensajes
    Conexion conexion;
    Mensajes mensajes(&conexion);
    Usuario usuario;
    
    std::string ip = "127.0.0.1";  // IP del servidor (puedes cambiarla)
    int puerto = 8080;  // Puerto (puedes cambiarlo)

    // Intentar conectarse al servidor
    if (!conexion.conectar(ip, puerto)) {
        std::cerr << "Error al conectar con el servidor.\n";
        return -1;
    }

    // Solicitar nombre de usuario
    std::string nombreUsuario;
    std::cout << "Introduce tu nombre de usuario: ";
    std::getline(std::cin, nombreUsuario);

    while (!Utilidades::esNombreValido(nombreUsuario)) {
        std::cout << "Nombre no válido. Debe ser alfanumérico o contener guiones bajos, máximo 20 caracteres.\n";
        std::cout << "Introduce un nombre válido: ";
        std::getline(std::cin, nombreUsuario);
    }
    
    usuario.setNombre(nombreUsuario);
    
    // Enviar nombre al servidor (esto podría ser parte del protocolo de conexión)
    if (!mensajes.enviarMensaje("Nombre de usuario: " + nombreUsuario)) {
        std::cerr << "Error al enviar el nombre al servidor.\n";
        return -1;
    }

    std::cout << "Conectado como " << usuario.getNombre() << "\n";

    // Ciclo principal
    while (true) {
        std::cout << "\n=== Menú ===\n";
        std::cout << "1. Chatear con todos (broadcast)\n";
        std::cout << "2. Enviar mensaje privado\n";
        std::cout << "3. Cambiar estado\n";
        std::cout << "4. Listar usuarios conectados\n";
        std::cout << "5. Ver información de un usuario\n";
        std::cout << "6. Ver quién está conectado\n";
        std::cout << "7. Ayuda\n";
        std::cout << "8. Salir\n";
        std::cout << "Selecciona una opción: ";

        int opcion;
        std::cin >> opcion;
        std::cin.ignore();  // Limpiar el buffer

        switch (opcion) {
            case 1: {
                // Chatear con todos (broadcast)
                std::string mensaje;
                std::cout << "Escribe el mensaje para todos: ";
                std::getline(std::cin, mensaje);
                if (!mensajes.enviarMensaje(mensaje)) {
                    std::cerr << "Error al enviar mensaje.\n";
                }
                break;
            }
            case 2: {
                // Enviar mensaje privado
                std::string destinatario, mensaje;
                std::cout << "Introduce el destinatario: ";
                std::getline(std::cin, destinatario);
                std::cout << "Escribe el mensaje privado: ";
                std::getline(std::cin, mensaje);
                if (!mensajes.enviarPrivado(destinatario, mensaje)) {
                    std::cerr << "Error al enviar mensaje privado.\n";
                }
                break;
            }
            case 3: {
                // Cambiar estado
                std::string estado;
                std::cout << "Selecciona tu estado (ACTIVO, OCUPADO, INACTIVO): ";
                std::getline(std::cin, estado);
                if (estado == "ACTIVO" || estado == "OCUPADO" || estado == "INACTIVO") {
                    if (!mensajes.cambiarEstado(estado)) {
                        std::cerr << "Error al cambiar estado.\n";
                    } else {
                        usuario.setEstado(estado);
                        std::cout << "Estado actualizado a: " << estado << "\n";
                    }
                } else {
                    std::cerr << "Estado no válido.\n";
                }
                break;
            }
            case 4: {
                // Listar usuarios conectados
                mensajes.solicitarListaUsuarios();
                std::cout << "Usuarios conectados:\n";
                std::string lista = mensajes.recibirMensaje();
                std::cout << lista << "\n";
                break;
            }
            case 5: {
                // Ver información de un usuario
                std::string nombreUsuario;
                std::cout << "Introduce el nombre de usuario para ver su información: ";
                std::getline(std::cin, nombreUsuario);
                mensajes.solicitarInfoUsuario(nombreUsuario);
                std::string info = mensajes.recibirMensaje();
                std::cout << info << "\n";
                break;
            }
            case 6: {
                // Ver quién está conectado
                std::cout << "Conectado como: " << usuario.getNombre() << "\n";
                std::cout << "IP del servidor: " << ip << "\n";
                break;
            }
            case 7: {
                // Mostrar ayuda
                mostrarAyuda();
                break;
            }
            case 8: {
                // Salir
                std::cout << "Saliendo del cliente...\n";
                conexion.cerrar();
                return 0;
            }
            default: {
                std::cout << "Opción no válida.\n";
                break;
            }
        }
    }

    return 0;
}
