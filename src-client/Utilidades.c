#include "Utilidades.h"
#include <sstream>
#include <ctime>
#include <cctype>

std::vector<std::string> Utilidades::dividirCadena(const std::string& str, char delimitador) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string item;

    while (std::getline(ss, item, delimitador)) {
        tokens.push_back(item);
    }

    return tokens;
}

std::string Utilidades::limpiarEspacios(const std::string& str) {
    size_t inicio = str.find_first_not_of(" \t\n\r\f\v");
    size_t fin = str.find_last_not_of(" \t\n\r\f\v");

    if (inicio == std::string::npos || fin == std::string::npos) {
        return ""; 
    }

    return str.substr(inicio, fin - inicio + 1);
}


bool Utilidades::esNombreValido(const std::string& nombre) {
    if (nombre.empty() || nombre.length() > 20) return false;

    for (char c : nombre) {
        if (!std::isalnum(c) && c != '_' && c != '.') {
            return false;
        }
    }
    return true;
}

std::string Utilidades::obtenerTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm* t = std::localtime(&now);
    
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", t);
    
    return std::string(buffer);
}

void mostrarAyuda() {
    std::cout << "\n=== Ayuda del Cliente ===\n";
    std::cout << "1. /all <mensaje>       → Enviar mensaje general\n";
    std::cout << "2. /pm <user> <mensaje> → Enviar mensaje privado\n";
    std::cout << "3. /estado <estado>     → Cambiar estado (ACTIVO, OCUPADO, INACTIVO)\n";
    std::cout << "4. /list                → Ver usuarios conectados\n";
    std::cout << "5. /info <user>         → Ver información de un usuario\n";
    std::cout << "6. /ayuda               → Mostrar esta ayuda\n";
    std::cout << "7. /salir               → Cerrar cliente\n";
}