#include "Mensajes.h"
#include <iostream>
#include <vector>
#include <cstring> 
#include <unistd.h>

Mensajes::Mensajes(Conexion* conn) : conexion(conn), threadActivo(false) {
    // Inicializar mutex y condition variable
    pthread_mutex_init(&mutexCola, nullptr);
    pthread_cond_init(&condCola, nullptr);
}

Mensajes::~Mensajes() {
    // Asegurar que el thread se detenga adecuadamente
    detenerThread();
    
    // Destruir mutex y condition variable
    pthread_mutex_destroy(&mutexCola);
    pthread_cond_destroy(&condCola);
}

bool Mensajes::iniciarThread() {
    // Si el thread ya está activo, no hacer nada
    if (threadActivo.load()) {
        return true;
    }
    
    // Establecer flag para mantener thread activo
    threadActivo.store(true);
    
    // Crear thread de trabajo
    if (pthread_create(&threadEnvio, nullptr, &Mensajes::threadTrabajador, this) != 0) {
        std::cerr << "Error al crear thread para mensajes" << std::endl;
        threadActivo.store(false);
        return false;
    }
    
    return true;
}

void Mensajes::detenerThread() {
    // Si el thread no está activo, no hacer nada
    if (!threadActivo.load()) {
        return;
    }
    
    // Indicar al thread que debe terminar
    threadActivo.store(false);
    
    // Señalizar al thread en caso de que esté esperando
    pthread_mutex_lock(&mutexCola);
    pthread_cond_signal(&condCola);
    pthread_mutex_unlock(&mutexCola);
    
    // Esperar a que el thread termine
    pthread_join(threadEnvio, nullptr);
}

void* Mensajes::threadTrabajador(void* arg) {
    Mensajes* self = static_cast<Mensajes*>(arg);
    
    // Bucle principal del thread
    while (self->threadActivo.load()) {
        std::shared_ptr<MensajeTarea> tarea = nullptr;
        
        // Obtener tarea de la cola
        pthread_mutex_lock(&self->mutexCola);
        
        // Si la cola está vacía, esperar señal
        while (self->colaMensajes.empty() && self->threadActivo.load()) {
            pthread_cond_wait(&self->condCola, &self->mutexCola);
        }
        
        // Si el thread debe terminar y no hay tareas, salir
        if (!self->threadActivo.load() && self->colaMensajes.empty()) {
            pthread_mutex_unlock(&self->mutexCola);
            break;
        }
        
        // Obtener tarea de la cola
        if (!self->colaMensajes.empty()) {
            tarea = self->colaMensajes.front();
            self->colaMensajes.pop();
        }
        
        pthread_mutex_unlock(&self->mutexCola);
        
        // Procesar tarea
        if (tarea) {
            // Obtener mensaje formateado
            std::vector<uint8_t> mensaje;
            mensaje.push_back(tarea->tipo);
            
            // Si hay contenido, añadirlo
            if (!tarea->contenido.empty()) {
                mensaje.push_back(static_cast<uint8_t>(tarea->contenido.size()));
                mensaje.insert(mensaje.end(), tarea->contenido.begin(), tarea->contenido.end());
            }
            
            // Enviar mensaje
            bool res = self->conexion->enviar(mensaje);
            
            // Actualizar resultado y flag de completado
            if (tarea->resultado) {
                tarea->resultado->store(res);
            }
            if (tarea->completado) {
                tarea->completado->store(true);
            }
        }
    }
    
    return nullptr;
}

bool Mensajes::encolarMensaje(uint8_t tipo, const std::vector<uint8_t>& contenido) {
    // Crear flags para rastrear el estado de la tarea
    std::atomic<bool> completado(false);
    std::atomic<bool> resultado(false);
    
    // Crear tarea
    auto tarea = std::make_shared<MensajeTarea>();
    tarea->tipo = tipo;
    tarea->contenido = contenido;
    tarea->completado = &completado;
    tarea->resultado = &resultado;
    
    // Añadir tarea a la cola
    pthread_mutex_lock(&mutexCola);
    colaMensajes.push(tarea);
    pthread_cond_signal(&condCola);
    pthread_mutex_unlock(&mutexCola);
    
    // Esperar a que la tarea se complete
    while (!completado.load()) {
        // Pequeña pausa para no saturar la CPU
        usleep(1000);
    }
    
    return resultado.load();
}

std::vector<uint8_t> Mensajes::formatearMensaje(uint8_t tipo, const std::vector<uint8_t>& contenido) {
    std::vector<uint8_t> mensaje;
    mensaje.push_back(tipo); 

    if (!contenido.empty()) {
        mensaje.push_back(static_cast<uint8_t>(contenido.size()));
        mensaje.insert(mensaje.end(), contenido.begin(), contenido.end());
    }

    return mensaje;
}

bool Mensajes::solicitarListaUsuarios() {
    // Verificar que el thread está activo
    if (!threadActivo.load() && !iniciarThread()) {
        return false;
    }
    
    // Tipo 1 para listar usuarios (sin contenido)
    return encolarMensaje(0x01, std::vector<uint8_t>());
}

bool Mensajes::obtenerUsuario(const std::string& nombreUsuario) {
    // Verificar que el thread está activo
    if (!threadActivo.load() && !iniciarThread()) {
        return false;
    }
    
    std::vector<uint8_t> contenido(nombreUsuario.begin(), nombreUsuario.end());
    return encolarMensaje(0x02, contenido);
}

bool Mensajes::cambiarEstado(uint8_t nuevoEstado) {
    // Verificar que el thread está activo
    if (!threadActivo.load() && !iniciarThread()) {
        return false;
    }
    
    if (nuevoEstado > 3) {
        std::cerr << "Estado inválido. Debe ser 0 (DESACTIVADO), 1 (ACTIVO), 2 (OCUPADO), o 3 (INACTIVO)" << std::endl;
        return false;
    }
    
    // En el protocolo, debemos enviar primero el nombre del usuario
    std::vector<uint8_t> datos;
    
    // Obtenemos nuestro nombre de usuario desde la conexión
    std::string nombreUsuario = conexion->getUsuario();
    
    // Añadimos el nombre de usuario
    datos.push_back(static_cast<uint8_t>(nombreUsuario.size()));
    datos.insert(datos.end(), nombreUsuario.begin(), nombreUsuario.end());
    
    // Añadimos el estado
    datos.push_back(nuevoEstado);
    
    // No se usa el formato estándar para este mensaje
    std::vector<uint8_t> msgBinario;
    msgBinario.push_back(0x03); // Tipo 3 para cambiar estado
    
    // NO añadir tamaño del contenido, enviamos directamente los datos
    msgBinario.insert(msgBinario.end(), datos.begin(), datos.end());
    
    // Envío directo sin formatear
    return conexion->enviar(msgBinario);
}

bool Mensajes::enviarMensaje(const std::string& mensaje) {
    // Enviamos al chat general (~)
    return enviarMensajeA("~", mensaje);
}

bool Mensajes::enviarMensajeA(const std::string& destinatario, const std::string& mensaje) {
    // Verificar que el thread está activo
    if (!threadActivo.load() && !iniciarThread()) {
        return false;
    }
    
    std::vector<uint8_t> contenido;
    
    // Añadimos el destinatario
    contenido.push_back(static_cast<uint8_t>(destinatario.size()));
    contenido.insert(contenido.end(), destinatario.begin(), destinatario.end());
    
    // Añadimos el mensaje
    contenido.push_back(static_cast<uint8_t>(mensaje.size()));
    contenido.insert(contenido.end(), mensaje.begin(), mensaje.end());
    
    return encolarMensaje(0x04, contenido);
}

bool Mensajes::solicitarHistorial(const std::string& chat) {
    // Verificar que el thread está activo
    if (!threadActivo.load() && !iniciarThread()) {
        return false;
    }
    
    std::vector<uint8_t> contenido(chat.begin(), chat.end());
    return encolarMensaje(0x05, contenido);
}

bool Mensajes::registrarUsuario(const std::string& username) {
    // Verificar que el thread está activo
    if (!threadActivo.load() && !iniciarThread()) {
        return false;
    }
    
    std::vector<uint8_t> contenido(username.begin(), username.end());
    return encolarMensaje(0x0A, contenido); // 0x0A = 10 (Código para registro)
}

// Método de registro (este es especial, no está en el protocolo original)
bool Mensajes::registrar(const std::string& nombreUsuario) {
    // Mantenemos este método para compatibilidad, redirigimos al nuevo método
    return registrarUsuario(nombreUsuario);
}
