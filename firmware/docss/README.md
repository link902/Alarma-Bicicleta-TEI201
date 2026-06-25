# Documentación de Software

## Contenido

### Diagramas de Flujo
Incluir diagramas de las funciones principales del sistema.

**Formato:** PNG, PDF o draw.io

### Documentación de Funciones

#### Funciones Principales
UrlEncode()
Propósito: Codifica una cadena de texto en formato URL ("URL encoding") para que caracteres especiales o espacios se transmitan correctamente en peticiones HTTP.
Parámetros: "String Value" - El texto original que se desea codificar.
Retorna: "string" - La cadena de texto codificada.

EnviarAlertaTelegram()
Propósito: Realiza una petición HTTP GET a la API de Telegram para enviar un mensaje instantáneo informando la activación de la alerta.
Parámetros: Ninguno.
Retorna:** Sin retorno ("void").

ActualizarFirebase()
Propósito: Envía el estado actual del sensor magnético/movimiento a una base de datos en tiempo real de Firebase utilizando una petición HTTP PUT. Permite la integración con tableros externos como Node-RED.
Parámetros: int valor - El estado binario del sensor (HIGH o LOW).
Retorna: Sin retorno (void).

Ejemplo de uso:
```cpp
if (estadoActual == HIGH) {
    if (tiempoActual - ultimaAlertaTime > tiempoBloqueo) {
        enviarAlertaTelegram();
        ultimaAlertaTime = tiempoActual;
    }
}

---

## Manual de Instalación
Instalación del entorno: Descargar e instalar la última versión de Arduino IDE.
Configurar Placa: Instalar el soporte de tarjetas ESP32 de Espressif.
Instalar Librerias: Se utilizan librerías nativas del core ESP32 (Wifi.h y HTTPClient.h) por lo que no son necesarias descargas externas.
Configuración de Parámetros: Modificar las siguientes constantes en el archivo fuente:
ssid y password con los datos de tu red
token y chat_id correspondientes al bot de telegram
firebaseURL con la dirección del nodo JSON en firebase 
Carga: Conectar la ESP32 por USB, seleccionar la placa y el puerto correspondiente. Subir el código.

Encendido del Sistema
Alimentar la placa ESP32 mediante su puerto USB o una fuente de poder externa de 5V.

Abrir el Monitor Serie para observar el mensaje "Conectando a WiFi...". Una vez establecida la conexión, aparecerá "¡WiFi Conectado!".

Operación Normal
Monitoreo: El pin GPIO 4 lee constantemente el estado del sensor utilizando una resistencia interna de INPUT_PULLUP.

Detección: Si el sensor detecta un cambio de estado a HIGH (movimiento), se activa un filtro de estabilidad (anti-rebote) de 100ms.

Ejecución de Alertas:

Se envía inmediatamente el nuevo estado de la alarma a Firebase para actualizar de forma síncrona el gráfico de Node-RED.

El buzzer conectado al GPIO 12 emite un pulso sonoro rápido a 2500Hz durante 1 segundo.

Si ha transcurrido un tiempo mayor al de bloqueo (5 segundos), se despacha de inmediato una notificación push al canal de Telegram configurado.

Apagado del Sistema
Desconectar físicamente el cable de alimentación o la batería de la placa ESP32. Al ser una memoria no volátil, el firmware permanecerá guardado listo para el próximo inicio.

## Troubleshooting

### Problemas Comunes

Causa probable: El punto de acceso (por ejemplo, el anclaje de red móvil "iPhone") está configurado en una banda exclusiva de 5 GHz o las credenciales no coinciden.

Solución: Asegurarse de activar la opción de "Maximizar compatibilidad" en el teléfono para habilitar la banda de 2.4 GHz requerida por el hardware de la ESP32, y verificar que la contraseña sea exacta.

Problema 2: El sistema detecta falsos positivos / El buzzer suena intermitentemente

Causa probable: Ruido eléctrico en la línea del sensor o rebotes mecánicos durante la transición del pin.

Solución: El código base incluye un delay(100) de estabilización. Si el ruido persiste, verificar la correcta fijación del sensor magnético y la cercanía de sus piezas, o incrementar ligeramente este retardo de validación en el loop().