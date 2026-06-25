# Librerías

## Librerías Estándar (Arduino IDE)

Este proyecto utiliza exclusivamente las librerías nativas incluidas en el sistema oficial de desarrollo para ESP32 de Espressif. Por lo tanto, no se requiere la instalación de librerías de terceros desde el Library Manager para el funcionamiento del código base.
1. WiFi.h - vBuilt-in
   - Propósito: Gestiona el hardware de red de la ESP32, permitiendo el escaneo, conexión y mantenimiento del enlace inalámbrico a puntos de acceso (como redes Wi-Fi domésticas o anclajes de red móviles).
   - Instalación: Incluida por defecto al instalar las tarjetas ESP32 en el entorno.

2. HTTPClient.h - vBuilt-in
   - Propósito: Proporciona las herramientas necesarias para estructurar, enviar y procesar peticiones de red HTTP (como peticiones GET para la API de Telegram y PUT para el Realtime Database de Firebase).
   - Instalación: Incluida por defecto al instalar las tarjetas ESP32 en el entorno.

---

## Librerías Personalizadas/Modificadas

Este proyecto no utiliza librerías personalizadas, modificadas de forma externa o alojadas localmente dentro del repositorio. Toda la lógica de control del sensor, el buzzer, los temporizadores de bloqueo y la codificación de caracteres web ("urlEncode") se encuentra integrada nativamente en el script principal de desarrollo.
Si hay librerías modificadas o no disponibles en Arduino IDE, incluir en esta carpeta:

### Estructura
```
librerias/
├── MiLibreria/
│   ├── MiLibreria.h
│   ├── MiLibreria.cpp
│   ├── keywords.txt
│   └── README.md
```

### Instalación Manual
```bash
# Copiar carpeta a Arduino libraries
cp -r MiLibreria ~/Documents/Arduino/libraries/
```

---

## Dependencias Completas

Listar TODAS las librerías necesarias con versiones exactas:

| Librería | Versión | Fuente | Obligatoria |
|----------|---------|--------|-------------|
| WiFi | Built-in | ESP32 Core | ✅ |
| HTTPClient| Built-in| ESP32 Core | ✅ |
