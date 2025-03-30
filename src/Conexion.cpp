#include "Conexion.h"
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>

Conexion::Conexion() : socket_fd(-1) {}

Conexion::~Conexion() {
    cerrar();
}

bool Conexion::conectar(const std::string& ip, int puerto) {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("Error al crear socket");
        return false;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(puerto);

    if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Dirección IP inválida\n";
        return false;
    }

    if (connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar al servidor");
        return false;
    }

    std::cout << "Conectado al servidor " << ip << ":" << puerto << "\n";
    return true;
}

bool Conexion::enviar(const std::string& mensaje) {
    return send(socket_fd, mensaje.c_str(), mensaje.length(), 0) >= 0;
}

std::string Conexion::recibir() {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    int bytes = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
    return (bytes > 0) ? std::string(buffer) : "";
}

void Conexion::cerrar() {
    if (socket_fd != -1) {
        close(socket_fd);
        socket_fd = -1;
    }
}

int Conexion::getSocket() const {
    return socket_fd;
}
