#ifndef LOGGING_H
#define LOGGING_H

#include <time.h>

// Función para registrar eventos
void log_event(const char* event, const char* details);

// Funciones para logging
void init_logging();
void close_logging();

#endif 
