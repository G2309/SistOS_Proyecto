#ifndef USER_MANAGEMENT_H
#define USER_MANAGEMENT_H

#include <pthread.h>
#include <arpa/inet.h>

#define MAX_USERS 100
#define USERNAME_MAX_LEN 32
#define PASSWORD_MAX_LEN 32
#define TIMEOUT_SECONDS 60

typedef enum {
	ACTIVO,
	OCUPADO,
	INACTIVO
} UserStatus;

typedef struct {
    int user_id;
    char username[USERNAME_MAX_LEN];
    char password[PASSWORD_MAX_LEN];
	char ip_address[INET_ADDRSTRLEN];
	time_t last_active;
    UserStatus status;
    int socket_fd;
} User;

typedef struct {
    User users[MAX_USERS];
    int user_count;
    pthread_mutex_t user_mutex;
} ServerState;

int register_user(ServerState *state, const char *username, const char *password, int socket_fd);
User *find_user_by_name(ServerState *state, const char *username);
int change_user_status(ServerState *state, const char *username, UserStatus new_status);
UserStatus get_user_status(ServerState *state, const char *username);
int get_all_users(ServerState *state, User *buffer, int max_count);
void logout_user(ServerState *state, const char *username);
int login_user(ServerState *state, const char *username, const char *password, int socket_fd);
int unregister_user(ServerState *state, const char *username);
void list_users(ServerState *state, char *buffer, size_t buffer_size);
void* monitor_inactivity(void *arg);


#endif

