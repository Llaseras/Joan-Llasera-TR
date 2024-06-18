#include <WiFi.h>

void setup() {
  Serial.begin(115200);

  // Configura el ESP32 en modo Station
  WiFi.mode(WIFI_STA);

  // Espera a que el WiFi esté listo
  delay(1000);

  // Obtiene y muestra la dirección MAC
  Serial.print("Dirección MAC del ESP32: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
  // No se necesita nada en el loop
}