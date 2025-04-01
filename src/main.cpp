#include "sockets.h"
#include "logging.h"
#include "user_management.h"
#include <pthread.h>

// Declarar que server_state es una variable definida en otro archivo (sockets.cpp)
extern ServerState server_state;

int main() {
    // Inicializar el sistema de logs
    init_logging();
    log_event("Server Start", "Iniciando servidor de chat");
    
    // Inicializar el estado del servidor
    init_server_state(&server_state);
    
    // Iniciar el hilo de monitoreo de inactividad
    pthread_t monitor_thread;
    pthread_create(&monitor_thread, NULL, monitor_inactivity, &server_state);
    
    // Iniciar el servidor WebSocket en el puerto 9000
    int port = 9000;
    start_server(port);
    
    // Cerrar el sistema de logs (esto nunca se ejecutará a menos que el servidor se cierre)
    close_logging();
    
    return 0;
}
