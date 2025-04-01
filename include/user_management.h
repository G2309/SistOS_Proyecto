#ifndef USER_MANAGEMENT_H
#define USER_MANAGEMENT_H

#include <pthread.h>
#include <libwebsockets.h>

#define MAX_USERS 100
#define USERNAME_MAX_LEN 16
#define PASSWORD_MAX_LEN 32
#define TIMEOUT_SECONDS 300 // 5 minutos de inactividad

typedef enum {
    DESACTIVADO = 0,
    ACTIVO = 1,
    OCUPADO = 2,
    INACTIVO = 3
} UserStatus;

typedef struct {
    int user_id;
    char username[USERNAME_MAX_LEN + 1];
    char password[PASSWORD_MAX_LEN + 1];
    char ip_address[16]; 
    UserStatus status;
    struct lws *wsi;  
    time_t last_active;
    int needs_status_notification; // Flag para notificar cambios de estado
} User;

typedef struct {
    User users[MAX_USERS];
    int user_count;
    pthread_mutex_t user_mutex;
} ServerState;

void init_server_state(ServerState* state);
int login_user(ServerState* state, const char* username, const char* password, struct lws *wsi);
void logout_user(ServerState* state, const char* username);
int register_user(ServerState* state, const char* username, const char* password, struct lws *wsi);
int unregister_user(ServerState* state, const char* username);
User* find_user_by_name(ServerState* state, const char* username);
void list_users(ServerState* state, char* buffer, size_t buffer_size);
int change_user_status(ServerState* state, const char* username, UserStatus new_status);
void* monitor_inactivity(void* arg);

#endif
