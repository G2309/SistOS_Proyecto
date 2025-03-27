#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "user_management.h"

void init_server_state(ServerState* state) {
    state->user_count = 0;
    pthread_mutex_init(&state->user_mutex, NULL);
}

int register_user(ServerState* state, const char* username, const char* ip_address, int client_socket) {
    pthread_mutex_lock(&state->user_mutex);
    
    if (state->user_count >= MAX_USERS) {
        pthread_mutex_unlock(&state->user_mutex);
        return -1;
    }
    
    for (int i = 0; i < state->user_count; i++) {
        if (strcmp(state->users[i].username, username) == 0) {
            pthread_mutex_unlock(&state->user_mutex);
            return -1;
        }
    }
    
    User* new_user = &state->users[state->user_count];
    strncpy(new_user->username, username, MAX_USERNAME - 1);
    strncpy(new_user->ip_address, ip_address, MAX_IP_LENGTH - 1);
    new_user->socket = client_socket;
    new_user->status = ACTIVO;
    
    state->user_count++;
    
    pthread_mutex_unlock(&state->user_mutex);
    return 0;
}

int unregister_user(ServerState* state, const char* username) {
    pthread_mutex_lock(&state->user_mutex);
    
    for (int i = 0; i < state->user_count; i++) {
        if (strcmp(state->users[i].username, username) == 0) {
            // Eliminar usuario moviendo los siguientes elementos
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

int change_user_status(ServerState* state, const char* username, UserStatus new_status) {
    pthread_mutex_lock(&state->user_mutex);
    
    for (int i = 0; i < state->user_count; i++) {
        if (strcmp(state->users[i].username, username) == 0) {
            state->users[i].status = new_status;
            pthread_mutex_unlock(&state->user_mutex);
            return 0;
        }
    }
    
    pthread_mutex_unlock(&state->user_mutex);
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
		sleep(5)
	}
	return NULL;
}
