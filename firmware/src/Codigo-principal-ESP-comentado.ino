#include <WiFi.h>       // Librería nativa para gestionar la conectividad inalámbrica de la ESP32
#include <HTTPClient.h> // Librería nativa para estructurar y enviar peticiones web (GET/PUT)

// 📶 CONFIGURACIÓN DE RED (Parámetros de acceso al punto de conexión)
const char* ssid = "iPhone"; 
const char* password = "123456789";

// 🤖 CONFIGURACIÓN TELEGRAM (Credenciales del Bot y chat de destino)
const String token = "8657909275:AAENIbB-E8Zo7_slHKeRZdorK5tXStDckus";
const String chat_id = "6338886384";

// 🔥 CONFIGURACIÓN FIREBASE (Dirección URL del nodo JSON para sincronizar con Node-RED)
const String firebaseURL = "https://dashboard-bb719-default-rtdb.firebaseio.com/estado_alarma.json?auth=kbL5pxjaeQLLbzDe13MrXaaNVcnwgwtPVTO8jvuq";

// 🔌 PINES (Asignación física de periféricos en la placa ESP32 DevKit)
#define PIN_SENSOR 4   // Pin GPIO 4 conectado a la señal del sensor magnético/movimiento
#define PIN_BUZZER 12  // Pin GPIO 12 conectado a la señal del actuador sonoro (Buzzer)

// Variables de control de estado y temporización (Siguen formato camelCase)
int ultimoEstadoSensor = -1;         // Almacena el registro previo para detectar variaciones de flanco
unsigned long ultimaAlertaTime = 0;   // Registra el tiempo de ejecución (en ms) del último mensaje enviado
const unsigned long tiempoBloqueo = 5000; // Candado de seguridad (5s) para evitar ráfagas de Spam en Telegram

/**
 * Función: urlEncode
 * Descripción: Convierte caracteres especiales, símbolos y espacios de una cadena de texto 
 * a su equivalente hexadecimal seguro para la web (%XX). Esto asegura que el mensaje 
 * sea interpretado correctamente por los servidores de la API de Telegram.
 * Parámetros: String value - El texto plano que se va a procesar.
 * Retorna: String - El texto codificado listo para ser insertado en una URL HTTP.
 */
String urlEncode(String value) {
  String encodedString = "";
  char c;
  for (int i = 0; i < value.length(); i++) {
    c = value.charAt(i);
    if (isalnum(c)) { 
      encodedString += c; // Mantiene intactos letras y números
    } else {
      // Proceso de conversión binaria para codificación en formato web de caracteres especiales
      char code0 = (c >> 4) & 0xf;
      char code1 = c & 0xf;
      encodedString += '%';
      encodedString += (char)(code0 > 9 ? code0 + 55 : code0 + 48);
      encodedString += (char)(code1 > 9 ? code1 + 55 : code1 + 48);
    }
  }
  return encodedString;
}

/**
 * Función: enviarAlertaTelegram
 * Descripción: Comprueba el estado de la red e inicia una petición síncrona HTTP GET 
 * hacia la API oficial de Telegram para despachar de inmediato una notificación 
 * push con el aviso de seguridad de la bicicleta.
 * Parámetros: Ninguno.
 * Retorna: void
 */
void enviarAlertaTelegram() {
  if (WiFi.status() == WL_CONNECTED) { // Evita bloqueos en el hilo si se pierde la señal de red
    HTTPClient http;
    String mensaje = "🚨 ⚠️ ¡ALERTA! Se ha detectado movimiento en tu bicicleta. ¡Revisa de inmediato! 🚲";
    // Construcción dinámica del endpoint de la API con los parámetros codificados
    String url = "https://api.telegram.org/bot" + token + "/sendMessage?chat_id=" + chat_id + "&text=" + urlEncode(mensaje);
    
    http.begin(url);            // Inicializa la conexión con el servidor web
    int httpCode = http.GET();  // Despacha la petición de datos
    http.end();                 // Libera los recursos de red de la ESP32
  }
}

/**
 * Función: actualizarFirebase
 * Descripción: Sincroniza el estado binario actual de la alarma con el Realtime Database de Firebase 
 * usando una petición síncrona HTTP PUT. Permite que Node-RED lea el cambio en vivo y grafique.
 * Parámetros: int valor - El estado lógico actual del sensor (0 o 1).
 * Retorna: void
 */
void actualizarFirebase(int valor) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(firebaseURL); // Abre el canal hacia el endpoint de Firebase
    http.addHeader("Content-Type", "application/json"); // Define que el cuerpo del mensaje es un JSON plano
    http.PUT(String(valor)); // Sobrescribe el valor del nodo raíz en la base de datos
    http.end();              // Cierra la sesión HTTP de forma segura
  }
}

/**
 * Función: setup
 * Descripción: Bloque de configuración inicial que se ejecuta una sola vez al energizar la ESP32.
 * Establece el comportamiento de los puertos físicos e inicia el enlace Wi-Fi.
 */
void setup() {
  Serial.begin(115200); // Inicializa la consola de diagnóstico a alta velocidad
  
  // Configuración de Pines de Hardware
  pinMode(PIN_SENSOR, INPUT_PULLUP); // Activa la resistencia interna pull-up para evitar lecturas erráticas flotantes
  pinMode(PIN_BUZZER, OUTPUT);       // Define el pin del buzzer como salida de potencia lógica
  noTone(PIN_BUZZER);                // Asegura un estado de silencio absoluto en el inicio del sistema

  // Proceso síncrono de conexión a la red inalámbrica
  Serial.print("Conectando a WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { // Bloquea el flujo hasta que el hardware consiga una IP válida
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n¡WiFi Conectado!"); // Diagnóstico exitoso en consola
}

/**
 * Función: loop
 * Descripción: Ciclo infinito de ejecución donde se procesa la telemetría del sensor, 
 * se aplican filtros anti-rebote y se coordinan secuencialmente las alertas físicas y web.
 */
void loop() {
  // Captura el estado lógico instantáneo presente en el pin del sensor
  int estadoActual = digitalRead(PIN_SENSOR);

  // EVALUACIÓN DE CAMBIO DE ESTADO (Monitoreo dinámico por flanco)
  if (estadoActual != ultimoEstadoSensor) {
    
    // Filtro anti-rebote electrónico reducido a 100ms para asegurar máxima velocidad sin capturar ruido mecánico
    delay(100); 
    estadoActual = digitalRead(PIN_SENSOR); // Re-verificación de estabilidad del pin físico
    
    // Si la lectura se mantiene firme después de la ventana de filtrado, procedemos
    if (estadoActual != ultimoEstadoSensor) {
      ultimoEstadoSensor = estadoActual; // Almacena el nuevo estado base
      unsigned long tiempoActual = millis(); // Captura el tiempo transcurrido desde el encendido

      // Envío inmediato a Firebase para asegurar que los gráficos de Node-RED se actualicen al instante
      actualizarFirebase(estadoActual);

      // CONDICIÓN DE DISPARO DE ALERTA: Si el estado cambia a HIGH (Movimiento/Apertura detectada)
      if (estadoActual == HIGH) {
        Serial.println("¡Movimiento detectado! Alerta activa.");
        
        // Alerta Física Local: Genera un tono de alta frecuencia (2500Hz) por 1 segundo síncrono
        tone(PIN_BUZZER, 2500); 
        delay(1000); // Mantiene el tono activo reteniendo el flujo de ejecución            
        noTone(PIN_BUZZER); // Apaga la señal hacia el transductor sonoro    

        // Alerta Remota en Red: Evalúa el candado de seguridad para impedir spam de notificaciones
        if (tiempoActual - ultimaAlertaTime > tiempoBloqueo) {
          enviarAlertaTelegram(); // Dispara de forma instantánea el mensaje al celular
          ultimaAlertaTime = tiempoActual; // Guarda la marca de tiempo de este envío
        }
      }
    }
  }

  delay(50); // Pequeña pausa estática para relajar el procesador y prevenir sobrecalentamiento
}