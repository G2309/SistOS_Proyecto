#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "logging.h"

#define LOG_FILE "logs/server.log"
static FILE* log_file = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_logging() {
    log_file = fopen(LOG_FILE, "a");
    if (!log_file) {
        perror("Error abriendo archivo de log");
        exit(1);
    }
}

void log_event(const char* event, const char* details) {
    time_t now;
    time(&now);
    char* date = ctime(&now);
    date[strlen(date) - 1] = '\0';  

    pthread_mutex_lock(&log_mutex);
    
    if (!log_file) {
        init_logging();
    }
    
    fprintf(log_file, "[%s] %s: %s\n", date, event, details);
    fflush(log_file);
    
    pthread_mutex_unlock(&log_mutex);
}

void close_logging() {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}
