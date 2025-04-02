# Proyecto: SistOS_Proyecto

## Descripción
Este proyecto implementa un sistema de chat en C++ basado en WebSockets. Incluye un servidor y un cliente que permiten la comunicación en tiempo real entre usuarios conectados.

## Autores
- **Javier Chen** - 22153
- **Gustavo Cruz** - 22779

## Requisitos
Antes de compilar y ejecutar el proyecto, asegúrese de tener instaladas las siguientes dependencias:

- `libwebsockets-dev`
- `wxwidgets-gtk3`
- `g++` (para compilar código en C++)

## Instalación
Clonar el repositorio desde GitHub:
```sh
git clone -b main https://github.com/G2309/SistOS_Proyecto.git
cd SistOS_Proyecto
```

Compilar el proyecto con `make`:
```sh
make
```

Iniciar el servidor:
```sh
./chat_server &
```

Iniciar el cliente:
```sh
./chat_client &
```

## Explicación del Código

### Cliente (`src/cliente/`)
- **Conexión.cpp**
  - Conectarse a un servidor WebSocket.
  - Enviar y recibir mensajes binarios.
  - Manejar errores y eventos como la lista de usuarios, mensajes y historial.
  - Ejecutar la escucha de mensajes en un hilo separado.

- **Mensajes.cpp**
  - Solicitar lista de usuarios.
  - Obtener información de un usuario.
  - Cambiar estado del usuario.
  - Enviar mensajes públicos y privados.
  - Obtener historial de mensajes.
  - Registrar un nuevo usuario.

- **Main_cli.cpp**
  - Inicio de sesión y conexión con el servidor en `127.0.0.1:9000`.
  - Menú interactivo con opciones para ver usuarios, cambiar estado, enviar mensajes y ver historial.
  - Manejo de desconexiones seguras.

### Servidor (`src`)
- **Handlers.cpp**
  - Procesa mensajes entrantes y eventos como conexiones y cambios de estado.
  - Interactúa con la lógica del servidor para responder a los clientes.
  
- **Logging.cpp**
  - Registra eventos importantes como conexiones, errores y mensajes.
  - Implementa bloqueo seguro con `pthread_mutex_t` para concurrencia.

- **Server.cpp**
  - Implementa el servidor WebSocket en el puerto `9000`.
  - Maneja conexiones de clientes y mensajes.
  - Gestiona la desconexión segura para evitar fugas de memoria.

- **user_management.cpp**
  - Maneja registro e inicio de sesión de usuarios.
  - Gestiona estados de usuario (Activo, Ocupado, Inactivo, Desactivado).
  - Implementa monitoreo de inactividad y eliminación de usuarios.

- **Main.cpp**
  - Inicializa logs y el estado del servidor.
  - Lanza un hilo para monitoreo de inactividad.
  - Arranca el servidor WebSocket en `9000`.

## Paralelización y Concurrencia
El proyecto usa **pthread** para ejecutar operaciones en paralelo, asegurando:
- Múltiples hilos para la escucha de mensajes.
- Mutexes (`pthread_mutex_t`) para evitar condiciones de carrera en el acceso a estructuras compartidas.
- Un sistema seguro de registro de eventos.


## Licencia
Este proyecto está bajo la licencia MIT. Puedes ver más detalles en el archivo `LICENSE`.

