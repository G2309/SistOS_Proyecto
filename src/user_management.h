#ifndef USER_MANAGEMENT_H
#define USER_MANAGEMENT_H

#include <pthread.h>

#define MAX_USERS 100
#define USERNAME_MAX_LEN 32
#define PASSWORD_MAX_LEN 32

typedef enum {
    STATUS_DISCONNECTED,
    STATUS_CONNECTED,
    STATUS_AWAY,
} UserStatus;

typedef struct {
    int user_id;
    char username[USERNAME_MAX_LEN];
    char password[PASSWORD_MAX_LEN];
    UserStatus status;
    int socket_fd;
} User;

typedef struct {
    User users[MAX_USERS];
    int user_count;
    pthread_mutex_t mutex;
} ServerState;

int register_user(ServerState *state, const char *username, const char *password, int socket_fd);
User *find_user_by_name(ServerState *state, const char *username);
int change_user_status(ServerState *state, const char *username, UserStatus new_status);
UserStatus get_user_status(ServerState *state, const char *username);
int get_all_users(ServerState *state, User *buffer, int max_count);
void logout_user(ServerState *state, const char *username);

#endif

