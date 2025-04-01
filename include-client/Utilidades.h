#ifndef UTILIDADES_H
#define UTILIDADES_H

#include <string>
#include <vector>

class Utilidades {
public:
    static std::vector<std::string> dividirCadena(const std::string& str, char delimitador);
    static std::string limpiarEspacios(const std::string& str);
    static bool esNombreValido(const std::string& nombre);
    static std::string obtenerTimestamp();
};

#endif
