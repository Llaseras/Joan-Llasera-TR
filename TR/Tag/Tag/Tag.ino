#include <WiFi.h>
#include <esp_now.h>
#include <esp_timer.h>
#include <stdio.h>

#define CHANNEL 1
#define INTERVALO_ENVIO 1000 // Intervalo de 1 segundo entre envíos

// Estructura para los mensajes
typedef struct struct_message {
  unsigned long timestamp; // Marca de tiempo
} struct_message;

int64_t startTime[3];
int64_t rtt[3];
bool responseReceived[3];
float distances[3]; // Distancias calculadas
int64_t rectificadores[3] = {29988, 20004, 10006};

// Direcciones MAC de las anclas
uint8_t ancla_addrs[3][6] = {
  {0xE0, 0x5A, 0x1B, 0xE2, 0xCC, 0x70}, // Dirección MAC de la ancla 1
  {0x48, 0xE7, 0x29, 0x8D, 0x9C, 0x48}, // Dirección MAC de la ancla 2
  {0x48, 0xE7, 0x29, 0x89, 0xFA, 0x80}  // Dirección MAC de la ancla 3
};

unsigned long lastSendTime = 0;

void setup() {
  Serial.begin(115200);
  esp_timer_init();
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW.");
    return;
  }

  esp_now_register_recv_cb(onReceiveData);

  // Configura las anclas como peers
  for (int i = 0; i < 3; i++) {
    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    peerInfo.channel = CHANNEL;
    peerInfo.encrypt = false;
    memcpy(peerInfo.peer_addr, ancla_addrs[i], 6);
    esp_now_add_peer(&peerInfo);
  }
}

void loop() {
  int64_t currentTime = esp_timer_get_time();

  // Enviar mensajes si ha pasado el intervalo de tiempo especificado
  if (currentTime - lastSendTime >= INTERVALO_ENVIO) {
    sendMessage();
    lastSendTime = currentTime;
  }

  // Procesar las respuestas recibidas
  for (int i = 0; i < 3; i++) {
    if (responseReceived[i]) {
      rtt[i] = esp_timer_get_time() - startTime[i];
      distances[i] = (rtt[i] - rectificadores[i]) * 0.5 * 300; // Velocidad de la señal (m/us)

      // Reiniciar el indicador para la próxima ronda de mediciones
      responseReceived[i] = false;
    }
  }

  if (distances[0] != NULL && distances[1] != NULL && distances[2] != NULL) {
    Serial.print(distances[0]);
    Serial.print(" ");
    Serial.print(distances[1]);
    Serial.print(" ");
    Serial.println(distances[2]);
  }

  delay(3000);
}

// Enviar un mensaje a todas las anclas
void sendMessage() {
  struct_message message;
  message.timestamp = millis();
  
  for (int i = 0; i < 3; i++) {
    startTime[i] = esp_timer_get_time();
    esp_now_send(ancla_addrs[i], (uint8_t *)&message, sizeof(message));
    delay(10); // Pequeño retraso para evitar colisiones de respuestas
  }
}

// Callback de recepción
void onReceiveData(const esp_now_recv_info* recv_info, const uint8_t* data, int len) {
  struct_message receivedData;
  memcpy(&receivedData, data, sizeof(receivedData));
  
  // Identifica cuál ancla ha respondido
  for (int i = 0; i < 3; i++) {
    if (memcmp(recv_info->src_addr, ancla_addrs[i], 6) == 0) {
      responseReceived[i] = true;
      break;
    }
  }
}
