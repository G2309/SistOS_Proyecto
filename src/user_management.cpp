#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "user_management.h"
#include "logging.h"
#include <unistd.h>
#include <ctime>

void init_server_state(ServerState* state) {
    state->user_count = 0;
    pthread_mutex_init(&state->user_mutex, NULL);
}

int login_user(ServerState* state, const char* username, const char* password, int socket_fd) {
    pthread_mutex_lock(&state->user_mutex);
    for (int i = 0; i < state->user_count; i++) {
        if (strcmp(state->users[i].username, username) == 0 &&
            strcmp(state->users[i].password, password) == 0) {
            state->users[i].socket_fd = socket_fd;
            state->users[i].status = ACTIVO; 
			state->users[i].last_active = time(NULL);

            pthread_mutex_unlock(&state->user_mutex);
            char msg[128];
            snprintf(msg, sizeof(msg), "Usuario %s inició sesión", username);
            log_event("Login Success", msg);
            return 0;
        }
    }
    pthread_mutex_unlock(&state->user_mutex);
    log_event("Login Failed", "Usuario o contraseña inválidos");
    return -1;
}


void logout_user(ServerState *state, const char *username) {
    pthread_mutex_lock(&state->user_mutex);

    for (int i = 0; i < state->user_count; i++) {
        if (strcmp(state->users[i].username, username) == 0) {
            state->users[i].status = INACTIVO; 
            char log_msg[128];
            snprintf(log_msg, sizeof(log_msg), "Usuario %s se desconectó", username);
            log_event("Logout", log_msg);
            break;
        }
    }

    pthread_mutex_unlock(&state->user_mutex);
}

int register_user(ServerState *state, const char *username, const char *password, int socket_fd) {
    pthread_mutex_lock(&state->user_mutex);

    for (int i = 0; i < state->user_count; i++) {
        if (strcmp(state->users[i].username, username) == 0) {
            pthread_mutex_unlock(&state->user_mutex);
            log_event("Register Failed", "Usuario ya existe");
            return -1;  // usuario ya existe
        }
    }

    if (state->user_count >= MAX_USERS) {
        pthread_mutex_unlock(&state->user_mutex);
        log_event("Register Failed", "Máximo número de usuarios alcanzado");
        return -2;
    }

    User *new_user = &state->users[state->user_count++];
    new_user->user_id = state->user_count;
    strncpy(new_user->username, username, USERNAME_MAX_LEN);
    strncpy(new_user->password, password, PASSWORD_MAX_LEN);
    new_user->status = ACTIVO;
    new_user->socket_fd = socket_fd;

    pthread_mutex_unlock(&state->user_mutex);

    char log_msg[128];
    snprintf(log_msg, sizeof(log_msg), "Usuario %s registrado con ID %d", username, new_user->user_id);
    log_event("Register Success", log_msg);

    return 0;
}


int unregister_user(ServerState* state, const char* username) {
    pthread_mutex_lock(&state->user_mutex);
    
    for (int i = 0; i < state->user_count; i++) {
        if (strcmp(state->users[i].username, username) == 0) {
            for (int j = i; j < state->user_count - 1; j++) {
                state->users[j] = state->users[j + 1];
            }
            state->user_count--;
            
            pthread_mutex_unlock(&state->user_mutex);
            return 0;
        }
    }
    
    pthread_mutex_unlock(&state->user_mutex);
    return -1;
}

User* find_user_by_name(ServerState* state, const char* username) {
    pthread_mutex_lock(&state->user_mutex);
    
    for (int i = 0; i < state->user_count; i++) {
        if (strcmp(state->users[i].username, username) == 0) {
            pthread_mutex_unlock(&state->user_mutex);
            return &state->users[i];
        }
    }
    
    pthread_mutex_unlock(&state->user_mutex);
    return NULL;
}

void list_users(ServerState* state, char* buffer, size_t buffer_size) {
    pthread_mutex_lock(&state->user_mutex);
    
    buffer[0] = '\0';
    for (int i = 0; i < state->user_count; i++) {
        char user_info[256];
        const char* status_str;
        
        switch (state->users[i].status) {
            case ACTIVO: status_str = "ACTIVO"; break;
            case OCUPADO: status_str = "OCUPADO"; break;
            case INACTIVO: status_str = "INACTIVO"; break;
            default: status_str = "DESCONOCIDO";
        }
        
        snprintf(user_info, sizeof(user_info), "Usuario: %s, IP: %s, Status: %s\n", 
                 state->users[i].username, 
                 state->users[i].ip_address, 
                 status_str);
        
        strncat(buffer, user_info, buffer_size - strlen(buffer) - 1);
    }
    
    pthread_mutex_unlock(&state->user_mutex);
}

int change_user_status(ServerState *state, const char *username, UserStatus new_status) {
    pthread_mutex_lock(&state->user_mutex);

    for (int i = 0; i < state->user_count; i++) {
        if (strcmp(state->users[i].username, username) == 0) {
            state->users[i].status = new_status;

            char log_msg[128];
            snprintf(log_msg, sizeof(log_msg), "Usuario %s cambió estado a %d", username, new_status);
            log_event("Status Change", log_msg);

            pthread_mutex_unlock(&state->user_mutex);
            return 0;
        }
    }

    pthread_mutex_unlock(&state->user_mutex);
    log_event("Status Change Failed", "Usuario no encontrado");
    return -1;
}


// Para cuando se haga el main, recordar iniciar el hilo de monitoreo:
// <pthread_t monitor_thread;
// pthread_create(&monitor_thread, NULL, monitor_inactivity, &server_state);

void* monitor_inactivity(void* arg) {
	ServerState* state = (ServerState*)arg;

	while(1) {
		pthread_mutex_lock(&state->user_mutex);
		time_t now = time(NULL);

		for (int i=0; i < state->user_count; i++){
			double diff = difftime(now, state->users[i].last_active);

			if (state->users[i].status == ACTIVO && diff >= TIMEOUT_SECONDS) {
				state->users[i].status = INACTIVO;
				// Para debuggear
				printf("Usuario %s está INACTIVO (%.0f segundos sin actividad)\n", state->users[i].username, diff);
			}

		}
		pthread_mutex_unlock(&state->user_mutex);
		sleep(5);
	}
	return NULL;
}
