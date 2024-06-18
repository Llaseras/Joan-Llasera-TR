#include <esp_now.h> // Incluim la llibreria esp_now que serveix per rebre la informació desde el TAG
#include <WiFi.h> // Incluim la llibreria WiFi que ens permet especificar el tipus de conexió que tindra el dispositiu.
#include <ArduinoJson.h> // Incluim la llibreria ArduinoJaon que serveix per fer operacions amb documents JSON.

// En les següents linies creem dues variables. Una és una cadena de caracters que és on s'emmagatzemara el Json sense serialitzar.
// i punt_de_llibre que és on indicarem quants bytes hem guardat dins del JSON actualement.
char jsonBuffer[500];
int dadesrebudes = 0;

bool LED = false;


// Ara es crea una funció que s'xecutará a l'hora de rebre dades. Aquesta funció té cuatre arguments que s'especifiquen a l'hora de trucar la funció,
void Recibiendo(const uint8_t *mac, const uint8_t *incomingData, int len) {
  // Aquesta linia el que fa és que si la suma dels bytes ya rebuts més els bytes que s'sestan rebent ara per a que així quan es guardin les dades la cadena de caracters no es quedi curta d'espai.
  // Aquesta linia de codi només executara el codi que esta dins d'ell si la suma de les dues coses és igual o inferior a 500.
  if (dadesrebudes + len <= 500) {

    // Això el que fa és agregar a la cadena JsonBuffer la cadena amb les dades rebudes. I el len és la quantitat de daes que s'han de copiar.
    // I el que fa és copiar-ho desde la part on no hi havia un valor ja escrit.
    memcpy(jsonBuffer + dadesrebudes, incomingData, len);

    // La següent linia el que fa és sumar a la quantitat de bytes que ja s'havien rebut més la quantitat de dades que s'han agregat al buffer.
    dadesrebudes += len;
  }

  // La següent linia el que fa és comprovar si l'ultim element de la informació que ve és un }. Que és el final del JSON que nosaltres hem enviat desde l'altre dispositiu.
  // A l'hora de fer compararcions amb caracters enlloc de "text" es fa servir 'text'.
  if (incomingData[len - 1] == '}') {

    // El que es fa en la següent linia és crear un docuemtn JSON amb 500 bytes d'emmagatzenament. El document JSON es dirá doc.
    StaticJsonDocument<500> doc;

    // En la següent linia el que es fa és crear una variable de tipus DeserializationError. El que vol dir és que si es produeix un error al serialitzar les dades la variable error ho emmagatzemará.
    // I en cas de que no doni error, el que pasara és que dins del document JSON (doc) es guardaran les dades que vindran de la variable JsonBuffer i que tindrán la llargada de dades rebudes.
    DeserializationError error = deserializeJson(doc, jsonBuffer, dadesrebudes);


    // Aquesta lnia el que fa és que si en cas de que la deserialització no doni error es fagi lo que esta entre claudators
    if (!error) {

      // A la següent linia el que es fa és imprimir a la pantlla de l'ordinador el missatge. que en aquest cas és ""JSON rebut:""
      Serial.println("JSON rebut:");
      serializeJsonPretty(doc, Serial);
    } else { // I aquí el que diu és que en cas de que falli es fara el que esta etre claudators d'aquesta part.

      // Aquí el que fara és que per el port USB-microB el dispositiu es conectara a l'ordinador i imprimirá l'error a la pantalla.
      Serial.print("Failed to parse JSON: "); 

      // Aixó el que fará será imprimir a la pantalla de l'ordinador la decripció de l'error que s'ha tingut.
      Serial.println(error.c_str());
    }

    // Per últim el que es fa és que es reinicia la variable dadesrebudes per poder formar un nou document JSON
    dadesrebudes = 0;

    // I aquí el que es fará és omplir JSON buffer amb valors 0. El que vol dir és que els 500 caracters de JSONBuffer pasaran a ser un 0.
    memset(jsonBuffer, 0, 500);
  }
}


// Ara ja si que comença la posada a la marxa del dispositiu.
void setup() {

  // Aquí el que es fa és especificar la freqüencia de la conexió, de manera que tant el ordinador com el esp32 funcionen a la mateixa freqüencia.
  Serial.begin(115200);

  // Aquí el que es fa és especificar quin tipus de conexió fará el dispositiu. En aquest cas el dispositiu creará una red WIFI on l'altre dispositiu actuará com a client.
  WiFi.mode(WIFI_AP);

  // Aquí el que es fa és inciar la xarxa WIFI i s'specifica la contrasenya i el ssid(nom) d'questa.
  WiFi.softAP("La red de Peta", "contraseña");

  // La següent linia el que fa és executar el codi que està entre claudators només en cas de que la inicialització de la conexió esp_now falli.
  if (esp_now_init() != ESP_OK) {

    // Al ordinador s'escriu: "Error inicialitzant l'ESP-NOW"
    Serial.println("Error inicialitzant l'ESP-NOW");
    return; // Aquí s'acavaria la funció i per tant faria falta reexecutar el codi.
  }

  // Aquí el que pasa és que la fucnió esp_now_register_recv_cb obté dades de la informació rebuda i aquestes dades les pasa com a arguments per a la funció Recibiendo.
  esp_now_register_recv_cb(Recibiendo);
}

void loop() {
  // El loop estará buit. Això és perque la funció esp_now_register_recv_cb al final de cada itineració es crida a si mateixa de manera que no para de repetir-se.
}