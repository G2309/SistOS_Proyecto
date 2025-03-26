#ifndef USER_MANAGEMENT_H
#define USER_MANAGEMENT_H

#include <netinet/in.h>
#define MAX_USERS 100
#define MAX_USERNAME 50
#define MAX_IP_LENGTH 50

// Enum para estados de usuario
typedef enum {
    ACTIVO,
    OCUPADO,
    INACTIVO
} UserStatus;

// Estructura de usuario
typedef struct {
    char username[MAX_USERNAME];
    char ip_address[MAX_IP_LENGTH];
    int socket;
    UserStatus status;
} User;

// Estructura global de estado del servidor
typedef struct {
    User users[MAX_USERS];
    int user_count;
    pthread_mutex_t user_mutex;
} ServerState;

// Funciones para gestion de usuarios
int register_user(ServerState* state, const char* username, const char* ip_address, int client_socket);
int unregister_user(ServerState* state, const char* username);
User* find_user_by_name(ServerState* state, const char* username);
void list_users(ServerState* state, char* buffer, size_t buffer_size);
int change_user_status(ServerState* state, const char* username, UserStatus new_status);

// Inicio del estado del server
void init_server_state(ServerState* state);

#endif 
