#ifndef CONSOLA_INTERCEPTOR_H
#define CONSOLA_INTERCEPTOR_H

#include <iostream>
#include <sstream>
#include <string>
#include <mutex>
#include <queue>
#include <regex>
#include <wx/wx.h>

// Declaración del evento personalizado
wxDECLARE_EVENT(wxEVT_MENSAJE_CONSOLA, wxCommandEvent);

// Clase para interceptar la salida estándar
class InterceptorConsola : public std::streambuf {
private:
    std::streambuf* m_sbuf;
    std::stringstream m_buffer;
    wxEvtHandler* m_receptor;
    std::mutex m_mutex;
    
    // Búfer para acumular líneas
    std::string m_lineaActual;
    
    // Procesamiento de mensajes
    void ProcesarMensaje(const std::string& mensaje) {
        // Procesar líneas individuales
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Crear evento de mensaje para la GUI
        wxCommandEvent evento(wxEVT_MENSAJE_CONSOLA);
        evento.SetString(wxString(mensaje));
        
        // Enviar evento a la GUI
        if (m_receptor) {
            wxPostEvent(m_receptor, evento);
        }
    }
    
    // Implementación de std::streambuf
    int overflow(int c) override {
        if (c == EOF) {
            return EOF;
        }
        
        if (c == '\n') {
            // Fin de línea, procesar mensaje completo
            ProcesarMensaje(m_lineaActual);
            m_lineaActual.clear();
        } else {
            // Acumular caracteres para la línea actual
            m_lineaActual += static_cast<char>(c);
        }
        
        // Pasar al búfer original
        return m_sbuf->sputc(c);
    }
    
public:
    InterceptorConsola(wxEvtHandler* receptor) 
        : m_sbuf(std::cout.rdbuf()), m_receptor(receptor) {
        std::cout.rdbuf(this);
    }
    
    ~InterceptorConsola() {
        // Restaurar el búfer original
        std::cout.rdbuf(m_sbuf);
    }
};

#endif // CONSOLA_INTERCEPTOR_H
