#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/listctrl.h>
#include <wx/statline.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/timer.h>
#include <wx/statusbr.h>
#include <wx/msgdlg.h>
#include <map>
#include <regex>
#include <string>

#include "Conexion.h"
#include "Mensajes.h"
#include "consola_interceptor.h"

// Definición del evento personalizado
wxDEFINE_EVENT(wxEVT_MENSAJE_CONSOLA, wxCommandEvent);

// Declaración de la aplicación
class AplicacionChat : public wxApp {
public:
    virtual bool OnInit();
};

// Declaración de la ventana principal
class VentanaPrincipal : public wxFrame {
public:
    VentanaPrincipal(const wxString& titulo);
    virtual ~VentanaPrincipal();

private:
    // Componentes de la GUI
    wxSplitterWindow* m_splitter;
    wxPanel* m_panelIzquierdo;
    wxPanel* m_panelDerecho;
    wxTextCtrl* m_textChat;
    wxTextCtrl* m_textMensaje;
    wxButton* m_btnEnviar;
    wxListCtrl* m_listaUsuarios;
    wxComboBox* m_comboEstado;
    wxButton* m_btnConectar;
    wxTextCtrl* m_textUsuario;
    wxTextCtrl* m_textServidor;
    wxTextCtrl* m_textPuerto;
    wxNotebook* m_notebook;
    wxPanel* m_tabGeneral;
    wxStatusBar* m_barraEstado;
	wxDateTime m_ultimaActividad;
    bool m_escribiendo;
    uint8_t m_estadoActual;
	static const uint8_t ESTADO_ACTIVO = 1;
    static const uint8_t ESTADO_OCUPADO = 2;
    static const uint8_t ESTADO_INACTIVO = 3;
    
    // Interceptor de consola
    InterceptorConsola* m_interceptor;
    
    // Mapa para almacenar las pestañas de chat privado (nombre usuario -> panel)
    std::map<wxString, wxPanel*> m_pestanasChat;
    std::map<wxString, wxTextCtrl*> m_chatsPrivados;
    
    // Objeto de conexión y mensajes
    Conexion* m_conexion;
    Mensajes* m_mensajes;
    wxString m_nombreUsuario;
    
    // Temporizador para actualizar estado
    wxTimer* m_timer;
    
    // Manejadores de eventos
    void OnConectar(wxCommandEvent& event);
    void OnEnviarMensaje(wxCommandEvent& event);
    void OnCambiarEstado(wxCommandEvent& event);
    void OnSeleccionarUsuario(wxListEvent& event);
    void OnNuevoChat(wxCommandEvent& event);
    void OnCerrarChat(wxCommandEvent& event);
    void OnSalir(wxCommandEvent& event);
    void OnTimer(wxTimerEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnMensajeConsola(wxCommandEvent& event);
    
    // Métodos auxiliares
    void ActualizarListaUsuarios();
    wxTextCtrl* ObtenerChatActual();
    void AgregarMensaje(const wxString& usuario, const wxString& mensaje, const wxString& destinatario = "General");
    wxPanel* CrearPestanaChat(const wxString& titulo);
    void ProcesarMensajeConsola(const wxString& mensaje);
	void RegistrarActividad();
	void OnTextChanged(wxCommandEvent& event);
	static const int ID_TEXT_CHANGED = 1001;
    
    // ID para los eventos
    enum {
        ID_CONECTAR = 1000,
        ID_ENVIAR,
        ID_ESTADO,
        ID_LISTA_USUARIOS,
        ID_NUEVO_CHAT,
        ID_CERRAR_CHAT,
        ID_TIMER
    };
    
    wxDECLARE_EVENT_TABLE();
};

// Implementación de la aplicación
wxIMPLEMENT_APP(AplicacionChat);

bool AplicacionChat::OnInit() {
    VentanaPrincipal* frame = new VentanaPrincipal("Cliente de Chat");
    frame->Show(true);
    return true;
}

// Tabla de eventos
wxBEGIN_EVENT_TABLE(VentanaPrincipal, wxFrame)
    EVT_BUTTON(ID_CONECTAR, VentanaPrincipal::OnConectar)
    EVT_BUTTON(ID_ENVIAR, VentanaPrincipal::OnEnviarMensaje)
    EVT_COMBOBOX(ID_ESTADO, VentanaPrincipal::OnCambiarEstado)
    EVT_LIST_ITEM_SELECTED(ID_LISTA_USUARIOS, VentanaPrincipal::OnSeleccionarUsuario)
    EVT_BUTTON(ID_NUEVO_CHAT, VentanaPrincipal::OnNuevoChat)
    EVT_BUTTON(ID_CERRAR_CHAT, VentanaPrincipal::OnCerrarChat)
    EVT_MENU(wxID_EXIT, VentanaPrincipal::OnSalir)
    EVT_TIMER(ID_TIMER, VentanaPrincipal::OnTimer)
    EVT_COMMAND(wxID_ANY, wxEVT_MENSAJE_CONSOLA, VentanaPrincipal::OnMensajeConsola)
wxEND_EVENT_TABLE()

// Constructor de la ventana principal
VentanaPrincipal::VentanaPrincipal(const wxString& titulo)
    : wxFrame(nullptr, wxID_ANY, titulo, wxDefaultPosition, wxSize(900, 600)),
      m_conexion(nullptr),
	  m_escribiendo(false),
	  m_estadoActual(ESTADO_ACTIVO),
      m_mensajes(nullptr) {
    
    // Inicializar el interceptor de consola
    m_interceptor = new InterceptorConsola(this);

	m_ultimaActividad = wxDateTime::Now();
    
    // Crear menú
    wxMenu* menuArchivo = new wxMenu;
    menuArchivo->Append(wxID_EXIT, "Salir");
    
    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuArchivo, "Archivo");
    SetMenuBar(menuBar);
    
    // Crear barra de estado
    m_barraEstado = CreateStatusBar();
    SetStatusText("Desconectado");
    
    // Crear panel principal
    wxPanel* panelPrincipal = new wxPanel(this);
    
    // Configurar sizer vertical para el panel principal
    wxBoxSizer* sizerPrincipal = new wxBoxSizer(wxVERTICAL);
    panelPrincipal->SetSizer(sizerPrincipal);
    
    // Panel de conexión
    wxPanel* panelConexion = new wxPanel(panelPrincipal);
    wxBoxSizer* sizerConexion = new wxBoxSizer(wxHORIZONTAL);
    panelConexion->SetSizer(sizerConexion);
    
    sizerConexion->Add(new wxStaticText(panelConexion, wxID_ANY, "Usuario:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    m_textUsuario = new wxTextCtrl(panelConexion, wxID_ANY);
    sizerConexion->Add(m_textUsuario, 1, wxALL, 5);
    
    sizerConexion->Add(new wxStaticText(panelConexion, wxID_ANY, "Servidor:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    m_textServidor = new wxTextCtrl(panelConexion, wxID_ANY, "");
    sizerConexion->Add(m_textServidor, 1, wxALL, 5);
    
    sizerConexion->Add(new wxStaticText(panelConexion, wxID_ANY, "Puerto:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    m_textPuerto = new wxTextCtrl(panelConexion, wxID_ANY, "9000");
    sizerConexion->Add(m_textPuerto, 0, wxALL, 5);
    
    m_btnConectar = new wxButton(panelConexion, ID_CONECTAR, "Conectar");
    sizerConexion->Add(m_btnConectar, 0, wxALL, 5);
    
    // Agregar panel de conexión al sizer principal
    sizerPrincipal->Add(panelConexion, 0, wxEXPAND | wxALL, 5);
    
    // Añadir línea separadora
    sizerPrincipal->Add(new wxStaticLine(panelPrincipal), 0, wxEXPAND | wxALL, 5);
    
    // Crear splitter window para dividir la ventana
    m_splitter = new wxSplitterWindow(panelPrincipal, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
    sizerPrincipal->Add(m_splitter, 1, wxEXPAND | wxALL, 5);
    
    // Panel izquierdo: lista de usuarios y estado
    m_panelIzquierdo = new wxPanel(m_splitter);
    wxBoxSizer* sizerIzquierdo = new wxBoxSizer(wxVERTICAL);
    m_panelIzquierdo->SetSizer(sizerIzquierdo);
    
    // Combo para el estado del usuario
    wxBoxSizer* sizerEstado = new wxBoxSizer(wxHORIZONTAL);
    sizerEstado->Add(new wxStaticText(m_panelIzquierdo, wxID_ANY, "Estado:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    
    wxArrayString estadosDisponibles;
    estadosDisponibles.Add("ACTIVO");
    estadosDisponibles.Add("OCUPADO");
    estadosDisponibles.Add("INACTIVO");
    
    m_comboEstado = new wxComboBox(m_panelIzquierdo, ID_ESTADO, "ACTIVO", wxDefaultPosition, wxDefaultSize, estadosDisponibles, wxCB_READONLY);
    sizerEstado->Add(m_comboEstado, 1, wxALL, 5);
    
    sizerIzquierdo->Add(sizerEstado, 0, wxEXPAND | wxALL, 5);
    
    // Lista de usuarios
    m_listaUsuarios = new wxListCtrl(m_panelIzquierdo, ID_LISTA_USUARIOS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    m_listaUsuarios->InsertColumn(0, "Usuario");
    m_listaUsuarios->InsertColumn(1, "Estado");
    m_listaUsuarios->SetColumnWidth(0, 120);
    m_listaUsuarios->SetColumnWidth(1, 80);
    
    sizerIzquierdo->Add(new wxStaticText(m_panelIzquierdo, wxID_ANY, "Usuarios conectados:"), 0, wxALL, 5);
    sizerIzquierdo->Add(m_listaUsuarios, 1, wxEXPAND | wxALL, 5);
    
    // Botón para chat privado
    wxButton* btnNuevoChat = new wxButton(m_panelIzquierdo, ID_NUEVO_CHAT, "Chat Privado");
    sizerIzquierdo->Add(btnNuevoChat, 0, wxEXPAND | wxALL, 5);
    
    // Panel derecho: área de chat
    m_panelDerecho = new wxPanel(m_splitter);
    wxBoxSizer* sizerDerecho = new wxBoxSizer(wxVERTICAL);
    m_panelDerecho->SetSizer(sizerDerecho);
    
    // Notebook para pestañas de chat
    m_notebook = new wxNotebook(m_panelDerecho, wxID_ANY);
    sizerDerecho->Add(m_notebook, 1, wxEXPAND | wxALL, 5);
    
    // Pestaña chat general
    m_tabGeneral = new wxPanel(m_notebook);
    wxBoxSizer* sizerTabGeneral = new wxBoxSizer(wxVERTICAL);
    m_tabGeneral->SetSizer(sizerTabGeneral);
    
    // Área de chat
    m_textChat = new wxTextCtrl(m_tabGeneral, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2);
    sizerTabGeneral->Add(m_textChat, 1, wxEXPAND | wxALL, 5);
    
    // Área de entrada de mensaje
    wxBoxSizer* sizerMensaje = new wxBoxSizer(wxHORIZONTAL);
    m_textMensaje = new wxTextCtrl(m_tabGeneral, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    m_textMensaje->Bind(wxEVT_KEY_DOWN, &VentanaPrincipal::OnKeyDown, this);
    sizerMensaje->Add(m_textMensaje, 1, wxALL, 5);
    
    m_btnEnviar = new wxButton(m_tabGeneral, ID_ENVIAR, "Enviar");
    sizerMensaje->Add(m_btnEnviar, 0, wxALL, 5);
    
    sizerTabGeneral->Add(sizerMensaje, 0, wxEXPAND | wxALL, 5);
    
    m_notebook->AddPage(m_tabGeneral, "General");
    
    // Dividir la ventana
    m_splitter->SplitVertically(m_panelIzquierdo, m_panelDerecho, 220);
    
    // Deshabilitar controles hasta conectar
    m_comboEstado->Disable();
    m_btnEnviar->Disable();
    m_textMensaje->Disable();
    
    // Iniciar temporizador para actualizar usuarios
    m_timer = new wxTimer(this, ID_TIMER);

	m_textMensaje->Bind(wxEVT_TEXT, &VentanaPrincipal::OnTextChanged, this);
    
    // Centrar la ventana
    Centre();
}

VentanaPrincipal::~VentanaPrincipal() {
    if (m_timer) {
        m_timer->Stop();
        delete m_timer;
    }
    
    if (m_conexion) {
        m_conexion->cerrar();
        delete m_conexion;
    }
    
    if (m_mensajes) {
        delete m_mensajes;
    }
    
    // Liberar el interceptor de consola
    delete m_interceptor;
}

void VentanaPrincipal::OnMensajeConsola(wxCommandEvent& event) {
    // Procesar mensaje de la consola
    wxString mensaje = event.GetString();
    ProcesarMensajeConsola(mensaje);
}

void VentanaPrincipal::OnTimer(wxTimerEvent& event) {
    if (m_conexion && m_mensajes && m_conexion->estaConectado()) {
        // Solicitar lista de usuarios cada 10 segundos
        static int contador = 0;
        if (++contador >= 20) { // 20 * 500ms = 10 segundos
            m_mensajes->solicitarListaUsuarios();
            contador = 0;
        }
        
        // Comprobar inactividad (10 segundos sin actividad)
        wxTimeSpan tiempoTranscurrido = wxDateTime::Now() - m_ultimaActividad;
        if (tiempoTranscurrido.GetSeconds() >= 10 && m_estadoActual != ESTADO_INACTIVO && !m_escribiendo) {
            // Cambiar a estado INACTIVO
            m_estadoActual = ESTADO_INACTIVO;
            m_mensajes->cambiarEstado(ESTADO_INACTIVO);
            m_comboEstado->SetValue("INACTIVO");
        }
    } else {
        // Si perdimos la conexión, desconectar
        if (m_conexion && !m_conexion->estaConectado()) {
            wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, ID_CONECTAR);
            wxPostEvent(this, evt);
        }
    }
}

void VentanaPrincipal::RegistrarActividad() {
    // Actualizar la hora de última actividad
    m_ultimaActividad = wxDateTime::Now();
    
    // Si el estado es INACTIVO, cambiarlo a ACTIVO
    if (m_estadoActual == ESTADO_INACTIVO && m_conexion && m_mensajes) {
        m_estadoActual = ESTADO_ACTIVO;
        m_mensajes->cambiarEstado(ESTADO_ACTIVO);
        m_comboEstado->SetValue("ACTIVO");
    }
}

void VentanaPrincipal::OnTextChanged(wxCommandEvent& event) {
    // Registrar actividad
    RegistrarActividad();
    
    // Control que generó el evento
    wxTextCtrl* textCtrl = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
    if (!textCtrl) return;
    
    // Verificar si está vacío o no
    bool estaEscribiendo = !textCtrl->GetValue().IsEmpty();
    
    // Si hay un cambio en el estado de escritura y estamos conectados
    if (m_escribiendo != estaEscribiendo && m_conexion && m_mensajes) {
        m_escribiendo = estaEscribiendo;
        
        // Si está escribiendo, cambiar a OCUPADO
        if (m_escribiendo && m_estadoActual != ESTADO_OCUPADO) {
            m_estadoActual = ESTADO_OCUPADO;
            m_mensajes->cambiarEstado(ESTADO_OCUPADO);
            m_comboEstado->SetValue("OCUPADO");
        }
        // Si dejó de escribir, volver a ACTIVO
        else if (!m_escribiendo && m_estadoActual == ESTADO_OCUPADO) {
            m_estadoActual = ESTADO_ACTIVO;
            m_mensajes->cambiarEstado(ESTADO_ACTIVO);
            m_comboEstado->SetValue("ACTIVO");
        }
    }
    
    // Propagar el evento
    event.Skip();
}

void VentanaPrincipal::ProcesarMensajeConsola(const wxString& mensaje) {
    // Ignorar mensajes de depuración
    if (mensaje.StartsWith("DEBUG")) {
        return;
    }
    
    // Para marcar el inicio de una lista de usuarios
    if (mensaje == "INICIO_LISTA_USUARIOS") {
        m_listaUsuarios->DeleteAllItems();
        return;
    }
    
    // Para marcar el fin de una lista de usuarios
    if (mensaje == "FIN_LISTA_USUARIOS") {
        return;
    }
    
    // Para mensajes de chat (formato especial)
    if (mensaje.StartsWith("MENSAJE_CHAT:")) {
    	wxString contenidoMsg = mensaje.Mid(13); // Saltar "MENSAJE_CHAT: "
    	int posSeparador = contenidoMsg.Find(':');
    	
    	if (posSeparador != wxNOT_FOUND) {
        	wxString remitente = contenidoMsg.Left(posSeparador).Trim();
        	wxString contenido = contenidoMsg.Mid(posSeparador + 1).Trim();
        	
        	std::cout << "Mensaje recibido - De: [" << remitente.ToStdString() 
                  	<< "], Contenido: [" << contenido.ToStdString() << "]" << std::endl;
        	
        	// Si el remitente soy yo, no procesar (ya se mostró al enviar)
        	if (remitente == m_nombreUsuario) {
            	return;
        	}
        	
        	// Buscar una pestaña de chat existente para este usuario
        	bool encontrada = false;
        	int indicePestana = -1;
        	
        	// Buscar por el nombre exacto del remitente
        	for (size_t i = 1; i < m_notebook->GetPageCount(); i++) {
            	wxString nombrePestana = m_notebook->GetPageText(i);
            	if (nombrePestana == remitente) {
                	indicePestana = i;
                	encontrada = true;
                	break;
            	}
        	}
        	
        	if (encontrada) {
            	// Pestaña encontrada, mostrar el mensaje ahí
            	wxString nombrePestana = m_notebook->GetPageText(indicePestana);
            	auto it = m_chatsPrivados.find(nombrePestana);
            	if (it != m_chatsPrivados.end()) {
                	it->second->AppendText(remitente + ": " + contenido + "\n");
                	
                	// Hacer visible la pestaña o destacarla si no está activa
                	if (m_notebook->GetSelection() != indicePestana) {
                    	// Opcional: Destacar la pestaña de alguna manera
                	}
            	}
        	} else {
            	// No se encontró pestaña, crear una nueva
            	wxPanel* panel = CrearPestanaChat(remitente);
            	auto it = m_chatsPrivados.find(remitente);
            	if (it != m_chatsPrivados.end()) {
                	it->second->AppendText(remitente + ": " + contenido + "\n");
            	}
        	}
        	
        	return;
    	}
	}

    // Para una línea de usuario en la lista
    if (mensaje.StartsWith("- ") && mensaje.Contains("(")) {
        wxString contenido = mensaje.Mid(2); // Eliminar "- " del inicio
        int posParentesis = contenido.Find('(');
        
        if (posParentesis != wxNOT_FOUND) {
            wxString nombre = contenido.Left(posParentesis).Trim();
            
            int posCierreParentesis = contenido.Find(')', posParentesis);
            if (posCierreParentesis != wxNOT_FOUND) {
                wxString estado = contenido.Mid(posParentesis + 1, 
                                              posCierreParentesis - posParentesis - 1);
                
                long indice = m_listaUsuarios->InsertItem(m_listaUsuarios->GetItemCount(), nombre);
                m_listaUsuarios->SetItem(indice, 1, estado);
            }
        }
        return;
    }
    
    // Para mensajes de error
    if (mensaje.StartsWith("Error:")) {
        wxString errorMsg = mensaje.AfterFirst(':').Trim();
        wxMessageBox(errorMsg, "Error", wxICON_ERROR);
        return;
    }
    
    // Para cambios de estado de usuario
    if (mensaje.Contains("cambió estado a")) {
        std::regex statusRegex("Usuario ([^ ]+) cambió estado a (.+)");
        std::string mensajeStr = mensaje.ToStdString();
        std::smatch statusMatches;
        
        if (std::regex_search(mensajeStr, statusMatches, statusRegex) && statusMatches.size() > 2) {
            wxString usuario = wxString(statusMatches[1]);
            wxString nuevoEstado = wxString(statusMatches[2]);
            
            // Actualizar la lista de usuarios
            for (int i = 0; i < m_listaUsuarios->GetItemCount(); i++) {
                if (m_listaUsuarios->GetItemText(i) == usuario) {
                    m_listaUsuarios->SetItem(i, 1, nuevoEstado);
                    break;
                }
            }
            
            // Mostrar el mensaje en el chat general
            m_textChat->AppendText("[Sistema] " + usuario + " cambió su estado a " + nuevoEstado + "\n");
        }
        return;
    }
    
    // Para nuevo usuario registrado
    if (mensaje.Contains("¡Nuevo usuario registrado!")) {
        std::regex newUserRegex("¡Nuevo usuario registrado! ([^ ]+) \\((.+)\\)");
        std::string mensajeStr = mensaje.ToStdString();
        std::smatch newUserMatches;
        
        if (std::regex_search(mensajeStr, newUserMatches, newUserRegex) && newUserMatches.size() > 2) {
            wxString usuario = wxString(newUserMatches[1]);
            wxString estado = wxString(newUserMatches[2]);
            
            // Verificar si ya existe el usuario
            bool usuarioExistente = false;
            for (int i = 0; i < m_listaUsuarios->GetItemCount(); i++) {
                if (m_listaUsuarios->GetItemText(i) == usuario) {
                    m_listaUsuarios->SetItem(i, 1, estado);
                    usuarioExistente = true;
                    break;
                }
            }
            
            // Si no existe, añadirlo
            if (!usuarioExistente) {
                long indice = m_listaUsuarios->InsertItem(m_listaUsuarios->GetItemCount(), usuario);
                m_listaUsuarios->SetItem(indice, 1, estado);
            }
            
            // Mostrar en el chat general
            m_textChat->AppendText("[Sistema] ¡Nuevo usuario conectado: " + usuario + " (" + estado + ")!\n");
        }
        return;
    }
    
    // Para historial de mensajes
    if (mensaje.Contains("Historial de mensajes")) {
        return;
    }
    
    // Para conexión cerrada
    if (mensaje.Contains("Conexión cerrada")) {
        SetStatusText("Desconectado");
        m_textChat->AppendText("[Sistema] Conexión cerrada\n");
        return;
    }
    
    // Para otros mensajes imprimibles que no encajen en las categorías anteriores
    bool contieneCaracteresImprimibles = false;
    for (size_t i = 0; i < mensaje.Length(); i++) {
        if (isprint(mensaje[i])) {
            contieneCaracteresImprimibles = true;
            break;
        }
    }
    
    if (contieneCaracteresImprimibles && !mensaje.Contains("DEBUG")) {
        m_textChat->AppendText(mensaje + "\n");
    }
}

void VentanaPrincipal::OnConectar(wxCommandEvent& event) {
    if (!m_conexion) {
        // Estamos desconectados, intentar conectar
        wxString usuario = m_textUsuario->GetValue();
        wxString servidor = m_textServidor->GetValue();
        long puerto;
        m_textPuerto->GetValue().ToLong(&puerto);
        
        if (usuario.IsEmpty()) {
            wxMessageBox("Por favor, introduce un nombre de usuario", "Error", wxICON_ERROR);
            return;
        }
        
        if (usuario == "~" || usuario.length() < 3 || usuario.length() > 16) {
            wxMessageBox("Nombre de usuario inválido. Debe tener entre 3 y 16 caracteres y no puede ser '~'.", "Error", wxICON_ERROR);
            return;
        }
        
        m_nombreUsuario = usuario;
        
        // Mostrar mensaje de intento de conexión
        SetStatusText("Conectando...");
        m_textChat->AppendText("Intentando conectar a " + servidor + ":" + wxString::Format("%ld", puerto) + "...\n");
        
        // Crear conexión
        m_conexion = new Conexion();
        
        // Intentar conectar
        bool conectado = m_conexion->conectar(servidor.ToStdString(), puerto, usuario.ToStdString());
        
        if (!conectado) {
            wxMessageBox("No se pudo conectar al servidor. Asegúrate que el servidor esté en ejecución y que la dirección y puerto sean correctos.", "Error de conexión", wxICON_ERROR);
            SetStatusText("Error de conexión");
            
            // Limpieza
            delete m_conexion;
            m_conexion = nullptr;
            
            m_textChat->AppendText("Error de conexión. Posibles causas:\n");
            m_textChat->AppendText("- El servidor no está en ejecución\n");
            m_textChat->AppendText("- La dirección o puerto son incorrectos\n");
            m_textChat->AppendText("- Hay un firewall bloqueando la conexión\n");
            m_textChat->AppendText("- El servidor rechazó la conexión\n\n");
            m_textChat->AppendText("Verifica que el servidor (chat_server) esté ejecutándose.\n");
            
            return;
        }
        
        // Crear objeto de mensajes
        m_mensajes = new Mensajes(m_conexion);
        
        // Solicitar lista de usuarios
        m_mensajes->solicitarListaUsuarios();
        
        // Cambiar estado de botón y habilitar controles
        m_btnConectar->SetLabel("Desconectar");
        m_comboEstado->Enable();
        m_btnEnviar->Enable();
        m_textMensaje->Enable();
        
        // Deshabilitar campos de conexión
        m_textUsuario->Disable();
        m_textServidor->Disable();
        m_textPuerto->Disable();
        
        // Iniciar temporizador
        m_timer->Start(500);  // Reducir la frecuencia a 500ms (era 100ms)
        
        // Mostrar mensaje de conexión exitosa
        m_textChat->AppendText("Conectado al servidor como " + usuario + "\n");
        SetStatusText("Conectado como " + usuario);
    } else {
        // Estamos conectados, desconectar
        SetStatusText("Desconectando...");
        
        // Detener temporizador
        m_timer->Stop();
        
        m_conexion->cerrar();
        delete m_conexion;
        m_conexion = nullptr;
        
        delete m_mensajes;
        m_mensajes = nullptr;
        
        // Cambiar estado de botón y deshabilitar controles
        m_btnConectar->SetLabel("Conectar");
        m_comboEstado->Disable();
        m_btnEnviar->Disable();
        m_textMensaje->Disable();
        
        // Habilitar campos de conexión
        m_textUsuario->Enable();
        m_textServidor->Enable();
        m_textPuerto->Enable();
        
        // Limpiar lista de usuarios
        m_listaUsuarios->DeleteAllItems();
        
        // Cerrar todas las pestañas de chat excepto la general
        while (m_notebook->GetPageCount() > 1) {
            m_notebook->DeletePage(1);
        }
        
        // Limpiar el mapa de pestañas
        m_pestanasChat.clear();
        m_chatsPrivados.clear();
        
        // Mostrar mensaje de desconexión
        m_textChat->AppendText("Desconectado del servidor\n");
        SetStatusText("Desconectado");
    }
}

void VentanaPrincipal::OnKeyDown(wxKeyEvent& event) {
	RegistrarActividad();
    if (event.GetKeyCode() == WXK_RETURN && !event.ShiftDown()) {
        // Si se presiona Enter sin Shift, enviar mensaje
        wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, ID_ENVIAR);
        wxPostEvent(this, evt);
    } else {
        // Propagar el evento
        event.Skip();
    }
}

void VentanaPrincipal::OnEnviarMensaje(wxCommandEvent& event) {

    if (!m_conexion || !m_mensajes) return;
    
    // Obtener la pestaña actual
    int pestanaActual = m_notebook->GetSelection();
    if (pestanaActual < 0) return;
    
    wxString mensaje;
    wxString destinatario;
    
    if (pestanaActual == 0) {
        // Estamos en la pestaña "General"
        mensaje = m_textMensaje->GetValue();
        destinatario = "~";  // Chat general
		RegistrarActividad();
        
        if (mensaje.IsEmpty()) {
            wxMessageBox("El mensaje no puede estar vacío", "Error", wxICON_ERROR);
            return;
        }

		if (m_estadoActual == ESTADO_OCUPADO && m_conexion && m_mensajes) {
        	m_estadoActual = ESTADO_ACTIVO;
        	m_mensajes->cambiarEstado(ESTADO_ACTIVO);
        	m_comboEstado->SetValue("ACTIVO");
    	}
        
        // Enviar al chat general
        bool exito = m_mensajes->enviarMensaje(mensaje.ToStdString());
        
        if (exito) {
            // Mostrar en el área de chat general
            m_textChat->AppendText(m_nombreUsuario + ": " + mensaje + "\n");
            m_textMensaje->Clear();
        } else {
            wxMessageBox("Error al enviar el mensaje al chat general", "Error", wxICON_ERROR);
        }
    } else {
        // Estamos en una pestaña de chat privado
        destinatario = m_notebook->GetPageText(pestanaActual);
        
        // Buscar el control de texto en esta pestaña
        wxPanel* panel = static_cast<wxPanel*>(m_notebook->GetPage(pestanaActual));
        if (!panel) return;
        
        // Obtener todos los controles de texto de la pestaña
        wxTextCtrl* textInput = nullptr;
        wxWindowList& children = panel->GetChildren();
        
        for (wxWindow* child : children) {
            wxTextCtrl* textCtrl = dynamic_cast<wxTextCtrl*>(child);
            if (textCtrl && !textCtrl->HasFlag(wxTE_READONLY)) {
                textInput = textCtrl;
                break;
            }
        }
        
        if (!textInput) {
            wxMessageBox("No se pudo encontrar el área de texto para enviar mensaje", "Error", wxICON_ERROR);
            return;
        }
        
        mensaje = textInput->GetValue();
        if (mensaje.IsEmpty()) {
            wxMessageBox("El mensaje no puede estar vacío", "Error", wxICON_ERROR);
            return;
        }
        
        std::cout << "Enviando mensaje privado - A: [" << destinatario.ToStdString() 
                  << "], Contenido: [" << mensaje.ToStdString() << "]" << std::endl;
        
        // Enviar mensaje privado
        bool exito = m_mensajes->enviarMensajeA(destinatario.ToStdString(), mensaje.ToStdString());
        
        if (exito) {
            // Mostrar en el área de chat privado
            auto it = m_chatsPrivados.find(destinatario);
            if (it != m_chatsPrivados.end()) {
                it->second->AppendText(m_nombreUsuario + ": " + mensaje + "\n");
                textInput->Clear();
            }
        } else {
            wxMessageBox("Error al enviar el mensaje privado", "Error", wxICON_ERROR);
        }
    }
}

wxTextCtrl* VentanaPrincipal::ObtenerChatActual() {
    // Obtener el control de texto de la pestaña actual
    int pestanaActual = m_notebook->GetSelection();
    if (pestanaActual == 0) {
        return m_textChat;
    } else {
        wxString nombrePestana = m_notebook->GetPageText(pestanaActual);
        auto it = m_chatsPrivados.find(nombrePestana);
        if (it != m_chatsPrivados.end()) {
            return it->second;
        }
    }
    return nullptr;
}

void VentanaPrincipal::AgregarMensaje(const wxString& usuario, const wxString& mensaje, const wxString& destinatario) {
    // Obtener el área de chat correspondiente
    wxTextCtrl* areaChat = nullptr;
    
    if (destinatario == "General" || destinatario == "~") {
        // Mensaje al chat general
        areaChat = m_textChat;
    } else {
        // Mensaje privado - buscar o crear pestaña
        auto it = m_chatsPrivados.find(destinatario);
        if (it != m_chatsPrivados.end()) {
            areaChat = it->second;
        } else {
            // También buscar por el nombre del remitente
            it = m_chatsPrivados.find(usuario);
            if (it != m_chatsPrivados.end()) {
                areaChat = it->second;
            } else {
                // Crear nueva pestaña si no existe
                wxPanel* panel = CrearPestanaChat(destinatario != "~" ? destinatario : usuario);
                wxTextCtrl* nuevoChat = dynamic_cast<wxTextCtrl*>(panel->FindWindow(wxID_ANY));
                if (nuevoChat) {
                    m_chatsPrivados[destinatario != "~" ? destinatario : usuario] = nuevoChat;
                    areaChat = nuevoChat;
                }
            }
        }
    }
    
    // Agregar el mensaje al área de chat
    if (areaChat) {
        wxString textoMensaje = usuario + ": " + mensaje + "\n";
        areaChat->AppendText(textoMensaje);
    }
}

wxPanel* VentanaPrincipal::CrearPestanaChat(const wxString& titulo) {
    // Verificar si ya existe la pestaña (buscar por título exacto)
    int indicePestana = -1;
    for (size_t i = 0; i < m_notebook->GetPageCount(); i++) {
        if (m_notebook->GetPageText(i) == titulo) {
            indicePestana = i;
            break;
        }
    }
    
    if (indicePestana != -1) {
        // Pestaña encontrada, activarla
        m_notebook->SetSelection(indicePestana);
        return static_cast<wxPanel*>(m_notebook->GetPage(indicePestana));
    }
    
    // No existe, crear nueva pestaña
    wxPanel* nuevaPestana = new wxPanel(m_notebook);
    wxBoxSizer* sizerPrincipal = new wxBoxSizer(wxVERTICAL);
    nuevaPestana->SetSizer(sizerPrincipal);
    
    // Área de chat (texto de solo lectura)
    wxTextCtrl* textChat = new wxTextCtrl(nuevaPestana, wxID_ANY, wxEmptyString,
                                        wxDefaultPosition, wxDefaultSize,
                                        wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2);
    sizerPrincipal->Add(textChat, 1, wxEXPAND | wxALL, 5);
    
    // Sizer horizontal para la entrada de texto y botón
    wxBoxSizer* sizerEntrada = new wxBoxSizer(wxHORIZONTAL);
    
    // Control de texto para escribir mensajes
    wxTextCtrl* textMensaje = new wxTextCtrl(nuevaPestana, wxID_ANY, wxEmptyString,
                                           wxDefaultPosition, wxDefaultSize, 
                                           wxTE_PROCESS_ENTER);
    sizerEntrada->Add(textMensaje, 1, wxEXPAND | wxALL, 5);
    
    // Manejar la tecla Enter
    textMensaje->Bind(wxEVT_KEY_DOWN, &VentanaPrincipal::OnKeyDown, this);
    
    // Botón de enviar
    wxButton* btnEnviar = new wxButton(nuevaPestana, ID_ENVIAR, "Enviar");
    sizerEntrada->Add(btnEnviar, 0, wxALL, 5);
    
    sizerPrincipal->Add(sizerEntrada, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
    
    // Botón para cerrar chat
    wxButton* btnCerrar = new wxButton(nuevaPestana, ID_CERRAR_CHAT, "Cerrar");
    wxBoxSizer* sizerBoton = new wxBoxSizer(wxHORIZONTAL);
    sizerBoton->Add(btnCerrar, 0, wxALL, 5);
    sizerPrincipal->Add(sizerBoton, 0, wxALIGN_RIGHT);
    
    // Añadir la nueva pestaña al notebook
    m_notebook->AddPage(nuevaPestana, titulo, true);
    
    // Guardar referencias usando el título exacto como clave
    m_pestanasChat[titulo] = nuevaPestana;
    m_chatsPrivados[titulo] = textChat;
    
    // Mensaje inicial
    textChat->AppendText("Chat privado con " + titulo + "\n");
    textChat->AppendText("--------------------------------------------------\n");
    
    // Solicitar historial
    if (m_mensajes) {
        m_mensajes->solicitarHistorial(titulo.ToStdString());
    }
    
    return nuevaPestana;
}

void VentanaPrincipal::ActualizarListaUsuarios() {
    // Actualizar la lista de usuarios (normalmente llamada después de recibir un mensaje tipo 51)
    // Como no tenemos acceso directo a los datos, solo refrescamos la UI
    if (m_mensajes) {
        m_mensajes->solicitarListaUsuarios();
    }
}

void VentanaPrincipal::OnCambiarEstado(wxCommandEvent& event) {
    if (!m_conexion || !m_mensajes) return;
    
    wxString estadoStr = m_comboEstado->GetValue();
    uint8_t estado = 1; // ACTIVO por defecto
    
	 if (estadoStr == "OCUPADO") {
        estado = ESTADO_OCUPADO;
    } else if (estadoStr == "INACTIVO") {
        estado = ESTADO_INACTIVO;
    }
    
    // Actualizar estado actual
    m_estadoActual = estado;
    
    // Cambiar estado en el servidor
    if (!m_mensajes->cambiarEstado(estado)) {
        wxMessageBox("Error al cambiar el estado", "Error", wxICON_ERROR);
    }
}

void VentanaPrincipal::OnSeleccionarUsuario(wxListEvent& event) {
    if (!m_conexion || !m_mensajes) return;
    
    // Obtener el usuario seleccionado
    long itemIndex = event.GetIndex();
    wxString usuario = m_listaUsuarios->GetItemText(itemIndex);
    
    // Solicitar información del usuario
    m_mensajes->obtenerUsuario(usuario.ToStdString());
}

void VentanaPrincipal::OnNuevoChat(wxCommandEvent& event) {
    if (!m_conexion || !m_mensajes) return;
    
    // Obtener el usuario seleccionado
    long itemSeleccionado = m_listaUsuarios->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (itemSeleccionado == -1) {
        wxMessageBox("Selecciona un usuario para iniciar un chat privado", "Aviso", wxICON_INFORMATION);
        return;
    }
    
    wxString usuario = m_listaUsuarios->GetItemText(itemSeleccionado);
    wxPanel* panel = CrearPestanaChat(usuario);
    
    // Solicitar historial para este usuario específico
    m_mensajes->solicitarHistorial(usuario.ToStdString());
}

void VentanaPrincipal::OnCerrarChat(wxCommandEvent& event) {
    // Obtener la pestaña actual
    int pestanaActual = m_notebook->GetSelection();
    if (pestanaActual > 0) { // No cerramos la pestaña general
        wxString nombrePestana = m_notebook->GetPageText(pestanaActual);
        
        // Eliminar del mapa
        m_pestanasChat.erase(nombrePestana);
        m_chatsPrivados.erase(nombrePestana);
        
        // Cerrar pestaña
        m_notebook->DeletePage(pestanaActual);
    }
}

void VentanaPrincipal::OnSalir(wxCommandEvent& event) {
    Close(true);
}
