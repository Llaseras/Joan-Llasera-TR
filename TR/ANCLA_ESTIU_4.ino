#include <WiFi.h>
#include <esp_now.h>

#define CHANNEL 1

// Estructura para los mensajes
typedef struct struct_message {
  unsigned long timestamp; // Marca de tiempo enviada por el TAG
} struct_message;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW.");
    return;
  }

  // Configura el callback de recepción
  esp_now_register_recv_cb(onReceiveData);

  // Configura el TAG como peer
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  peerInfo.channel = CHANNEL;
  peerInfo.encrypt = false;

  // Aquí deberías agregar la dirección MAC del TAG
  uint8_t tag_addr[] = {0x64, 0xB7, 0x08, 0xCA, 0xC0, 0x7C}; // Dirección MAC del TAG (ajusta según corresponda)
  memcpy(peerInfo.peer_addr, tag_addr, 6);
  esp_now_add_peer(&peerInfo);
}

void loop() {
  // No hay nada que hacer en el loop
}

// Callback de recepción con la firma correcta
void onReceiveData(const esp_now_recv_info* recv_info, const uint8_t* data, int len) {
  struct_message receivedData;
  memcpy(&receivedData, data, sizeof(receivedData));
  
  // Envía una respuesta con el tiempo de recepción
  struct_message responseData;
  responseData.timestamp = millis();

  esp_now_send(recv_info->src_addr, (uint8_t *)&responseData, sizeof(responseData));
}
