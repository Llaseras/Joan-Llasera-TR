  //Part especifica que es fa depenent de si el dispositiu actúa com un Tag o com un ancla.
// Aqui es defineix que el dispositiu és un Tag. (Si volguessim definir l'ancla, llavors ficariem #define IS_ANCHOR).
#define IS_TAG



// Aquí incluim la llibreria SPI que és necessaria per fer servir la següent llibreria.
#include <SPI.h>
// Aquí incluim la llibreria DW100 modificada per poder fer-se servir en un ESP32(Es pot fer servir amb qualsevol model d'aquest). Decarrega a: https://github.com/playfultechnology/arduino-dw1000
#include <DW1000Ranging.h>
// Aquí incluim la llibreria LiquidCrystal, que serveix per poder fer servir pantalles LCD.
#include <LiquidCrystal.h>


// Aquí el que passa és que només en cas de que el dispositiu sigui un Tag hem de incluir algunes llibreries.
#ifdef IS_TAG
  #include <WiFi.h> //Aquesta és per conectar-se a una xarxa WIFI. Encara que en aquest cas no ho fem, algunes funcions de dins són utils per fer el codi.
  #include <esp_now.h> // Aquesta llibreria és conectar el Tag amb el receptor. (Per transmitir les dades).
  #include "link.h" //Aquesta també. No és una llibreria, és un altre document, però també fa falta incluir-lo.
  #include <ArduinoJson.h> // Aquesta llibreria és per transformar el document anterior en un document Json.
  #include <algorithm> // Aquesta llibreria l'unica funció que té és poder fer servir el min() de la linia 279
#endif

// Per cada dispositiu/ancla s'ha de definir una adreça. 
// El 02 del principi és un estandar que se sol fer servir en aquest tipus de dispositius. Però, realment es pot fer servir com un vulgui.
// Aquí es fa servir el MAC, un tipus d'adreça eletronica comunment utilitzat.
#define DEVICE_ADDRESS "02:00:00:00:00:00:77:76"

// Aquí es defineix una variable de tipus char(cadena de text) que té 6 caracters.
// A aquesta variable se li asigna el nom shortAddress que després es fara servir com una Adreça abreviada. (Per generar menys carrega al processador).
char shortAddress[6];

struct MyLink *uwb_data;

// setup és la funció que inicia el codi i que serveix per a posar en marxa el dispositiu.
//En el setup es fiquen totes les funcions ue en algun moemtn es cridaran desde una llibreria.
void setup() {

  // Això el que fa és determinar la freqüencia a la que la placa i l'ordinador es comunicaran
  Serial.begin(115200);

  // En aquesta linia es crida la funció begin desde la llibreria SPI.
  // I se li asignan els pins: (18 per al rellotge, 19 per sortida de informació, 5 per entrada de informació).
  // En aquest cas l'argument són els pins que es faran servir.
  SPI.begin(18, 19, 23);
  
  // En aquesta linia següent sinicialitza la comunicació, desde la llibreria DWRanging es crida la funció initcomunication().
  //En aquesta funció, el 27 és el pin Reset
  // Un altre cop l'argument d'aquesta funció són els pins que aquesta fara servir.
  DW1000Ranging.initCommunication(33, 32, 35);

  // A la següent linia, de la llibreria DW100Ranging fem servir la funció attachNewRange.
  //Es fa servir la funció NewRange com a argument de la funció. (Com la funcio es fa servir com a argument no fa falta fica "()").
  // La funcio NewRange esta mes avaix en el codi.
  // Aquesta funcio s'activara quan la distancia entre els dispositius canvií.
  DW1000Ranging.attachNewRange(newRange);

  // A la següent linia, de la llibreria DW100Ranging es fa servir la funció atachNewDevice.
  // Al igual que en l'anterior linia es fa servir una funcio (NewDevice) com a argument.
  // Aquesta funció s'activara quan es detecti un nou dispositiu a la xarxa.
  DW1000Ranging.attachNewDevice(newDevice);

  // A la següent linia, de la mateixa llibreria que les anteriors dos es fa servir la funció attachInactiveDevice
  // I igualment es fa servir la funció InactiveDevice (més endevant en el codi) com a argument.
  // Aquesta funció s'activara quan un dispositiu deixi de ser detectat.
  DW1000Ranging.attachInactiveDevice(inactiveDevice);

  // Aquesta funcio el que fa iniciar la comunicació DW1000 amb el mode de TAG.
  // La comunicació DW-1000 es una especifica per poder localitzar mitjançant la trilateració.
  // La funció té tres arguments. Device_Adress que hem definit abans, el mode de transmissió de dades i el filtre de rang.
  // El filtre de rang no és necessari així que l'hem selecionat com a 0.
  DW1000Ranging.startAsTag(DEVICE_ADDRESS, DW1000.MODE_LONGDATA_RANGE_ACCURACY, false);

  // Aquí se li asigna a la variable uwb_data el valor que retorna la funció init_link que esta dins del document link.h.
  uwb_data = init_link();

  // I ara el que fenm és calcular una adreça més curta per a poder fer servir una adreça més accessible.

  // byte* serveix per a definir quin tipus de dades será la varible. (1 byte= 8 bits)
  // I després es fa servir la funció getCurrentShortAddres() de la biblioteca DW1000Ranging per a definir el valor de la variable que és current short addres.
  byte* currentShortAddress = DW1000Ranging.getCurrentShortAddress();

  // sprintf() és una funció que permet canviar els valors d'una funció ja existent amb un format de dades string.
  // shortAddress és la variable on emmagatzenarem els valors
  // el següent argument és "%02X%02X" les dues comiilles indiquen que és una cadena de text. Mentress que l'interior es pot dividir en dues parts iguals.
  // Cada una de les parts és %02X que vol dir que ha de ser un string amb dos digits. (En cas de que només sigui un digit es fica un 0 a la esquerra)
  sprintf(shortAddress, "%02X%02X", currentShortAddress[1], currentShortAddress[0]);
}


void loop() {
  // La següent funció és de la biblioteca DW1000Ranging. Que es podría dir que és la que fa tota la feina del loop. 
  // Aquesta funció fa moltes coses. Com per exemple: verificar si es necessita reiniciar le dispositiu, és qui mesura el temps del dispositiu
  // processa els missatges que han de ser enviats i rebuts, entre moltes altres coses. 
  // És la funció que obté la distancia que hi ha desde les ancles i el dispositiu a localitzar.
  // La funció és capaç de saber si el dispositiu és una ancla o un Tag i depenent del que sigui fa una cosa o una altra.
  DW1000Ranging.loop();

  // La funció send_json el que fa és enviar el docment json a través del port serial.
  send_json(uwb_data);

  // Ara per últim anem a afegir un petit punt que el que fará és millorar l'estalbi energetic i alliberará carga del TAG.
  // Això ho podem fer augmentant el temps entre itineració i itineració del codi.
  delay(100)
}


// En la següent funció el que es fa és enviar l'arxiu Json.
void send_json(struct MyLink *p) {

  // En la següent linia el que es fa és crear un document Json amb una capacitat de 500 bytes.
  StaticJsonDocument<500> doc;

  // Aquí dins del document Json el que es fa és crear un nou membre que es diu id. I el contingut que té aquest membre és igual a la adreça curta del dispositiu.
  doc["id"] = shortAddress;
  
  // Aquí el que es fa és crear un nou membre amb el nom links. Aquest membre serà un array, el que vol dir que tindrá més d'un valor dins d'aquest.
  JsonArray links = doc.createNestedArray("links");

  // En aquesta linia el ques fa és crear una variable amb el nom p i amb la estructura MyLink que tingui el valor p.
  // p és l'argument que es introduït a l'hora de fer servir aquesta funció.
  struct MyLink *temp = p;

  //La següent linia és un loop. Aquest loop es reproduira mentres que el següent membre de la variable temp sigui diferent a NULL. (Estigui buit).
  while (temp->next != NULL) {

    // Aquesta linia serveix per a que la següent vegada que el loop s'activi el que fagi sigui mirar el següent membre de la variable temp.
    temp = temp->next;

    // En la següent linia de codi el que es fa és crear un nou objecte anomenat obj1 dins del array anteriorment fet "links".
    JsonObject obj1 = links.createNestedObject();

    // En aquesta línia de codi el que es fa és donar-li el nom "a" al objecte 1 i després donar-li el valor del membre anchor_addr que esta dins de temp.
    obj1["a"] = temp->anchor_addr;

    // Eb la següent linia el que es fa és crear una cadena de caracters anomenada range. Aquesta cadena de caracters ha de tindre un total de 5 caracters.
    char range[5];

    // Aquí el que es fa és determinar el valor de la variable range.
    // Aquí s'agafa el valor del membre range de temp i transforma-lo en una cadena de caracters. En aquest cas que sería fet servir per donar-li valor a range.
    sprintf(range, "%.2f", temp->range[0]);

    // Aquí el que es fa és que en el objecte obj1 es fica un nou valor amb el nom de "r" i el valor que tindria dins sería el mateix que la cadena range.
    obj1["r"] = range;
  } // Aquí s'acava el loop.

  // Ara es crea una cadena de caracters anomenada Jsonbuffer que será qui guardara el document Json serialitzat.
  char jsonBuffer[500];


  // La següent linia de codi el que fa és transformar el document Json en una cadena de text Json, el qual és més facil de trasmetre.
  // En aquest cas els arguments són doc i udp, que són el arxiu a serialitzar i el destinatari on s'enviara aquest "arxiu".
  // També es crea una variable amb que té com a valor el tamany de JsonBuffer.
  size_t len = serializeJson(doc, jsonBuffer);


  Serial.println(jsonBuffer);
  Serial.print(DW1000Ranging.getDistantDevice()->getRange());

} // Aquí s'acava la funció


// Aquí es definirá la funció newRange() que es fa servir a la part d'adalt del codi.
void newRange() {
  // Aquí el que es fa és crear una cadena de caracters anomenada buffer. A aquesta cadena se li fiquen com a numero maxim de caracters 21.
  char buffer[21];

  // La següent linia de codi serveix per "omplir" la cadena de caracters buffer amb la adreça curta del dispositiu.
  // El problema és que la funció DW1000Ranging.getDistantDevice()->getShortAddress() retorna un valor en format int.
  // Així que hem de transformar aquest valor int en una cadena de caracters.
  snprintf(buffer, sizeof buffer, "%04x", DW1000Ranging.getDistantDevice()->getShortAddress());

  // En aquesta linia el que esta fent el nostre dispositiu és grabar dins de la cadena buffer la distancia que hi ha entre el dispositiu i la ancla en qüestió.
  int ret = snprintf(buffer, sizeof buffer, "%.2f", DW1000Ranging.getDistantDevice()->getRange());

  // Aquesta linia de text només s'activa en cas de que el dispositiu sigui un Tag.
  // La aegüent linia de codi el que fa és actualitzar la informació de la comunicació entre el dispositiu i l'ancla.
  // En aquesta funció hi ha un total de 4 arguments.
  // uwb_data que és informació de la conexió amb la ancla,  DW1000Ranging.getDistantDevice()->getShortAddress() que és la adreça curta de la ancla,
  // DW1000Ranging.getDistantDevice()->getRange() que és la distancia entre el dispositiu i l'ancla
  // i per ultim:  DW1000Ranging.getDistantDevice()->getRXPower() que és la potencia de la señal que rep l'ancla desde el dispositiu.
  #ifdef IS_TAG
    update_link(uwb_data, DW1000Ranging.getDistantDevice()->getShortAddress(), DW1000Ranging.getDistantDevice()->getRange(), DW1000Ranging.getDistantDevice()->getRXPower());
  #endif
}

// Aquí és on definirem la funció newDevice, aquesta funció es fa servir anterioremnt en el codi per agregar una nova conexió al codi.
// Com es pot veure, per fer servir aquesta funció hi ha un argument que és DW1000Device *device.
void newDevice(DW1000Device *device) {


  // La següent part del codi només es produeix si el dispositiu que esta executant el codi és un TAG.
  #ifdef IS_TAG 

    // Aquí es fa servir la funció add_link per afegir una ancla més.
    // Aquesta funció té dos arguements, uwb_data que és la informació de la conexió amb la ancla i device->getShortAddress() que és la adreça curta de l'ancla.
    add_link(uwb_data, device->getShortAddress());
  #endif
} // Aquí s'acava la funció.


// Per últim hem de definir la funció: inactiveDevice(). Aquesta funció té l'argument *device. I DW10000 és la estructura que ha de tindre l'argument device.
void inactiveDevice(DW1000Device *device) {


  // La següent part del codi només s'executa en cas de que el dispositiu sigui un TAG. 
  #ifdef IS_TAG 

    // La següent funció el que fa és parar la conexió amb la ancla que s'ha desconectat. 
    // La funció té dos arguments: uwb_data que és la informació sobre la conexió entre la ancla i el dispositiu.
    // i l'altre argument és device->getShortAddress() que el que fa és donarte la adreça curta del dispositiu.
    delete_link(uwb_data, device->getShortAddress());
  #endif
}
