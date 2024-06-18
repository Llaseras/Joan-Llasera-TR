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


// Aquesta part només afecta si esta fent servir una pantlla LCD en aquell dispositiu en especific.
//En la següent linia LiquidCrystal és el nom que li fiquem a la pantalla LCD.
//I els numeros dins dels parentesis fan referencia als pins assignats al dispositiu.
//Els pins també fan referencia al següent ordre de pins del LCD: RS, E, D4, D5, D6, D7.
LiquidCrystal lcd(13, 12, 5, 18, 19, 21);

#ifdef IS_TAG

  // Ara el que anem a fer és crear una variable que tingui la adreça MAC del nostre ESP32.
  // S'ha de ficar la adreça MAC del receptor. Cada placa ESP32 té una direcció MAC dferent. 64:B7:08:CA:C0:7C
  uint8_t receiverAddress[] = {0x64, 0xB7, 0x08, 0xCA, 0xC0, 0x7C};

  // Aquí creem un objecte anomenat uwb_data que emmagatzemarà informació amb la estrucutra de MyLink, que ve del document "link.h".
  struct MyLink *uwb_data;

  // Aquí es crea una variable del tipus long (32 bits) enlloc del tipus int(16 bits). El unsigned fa que només sigui positiu.
  // Després a la variable se li asigan el nom lastUpdateTime i per ultim se loi asigna el valor 0.
  unsigned long lastUpdateTime = 0;

  // Aquí es crea una variable de tipus int amb nomes valors positius.
  // A la variable se li asigna el nom udateInterval i se li asigna el valor 200.
  unsigned int updateInterval = 200;

#endif

// Aquí es defineix una variable de tipus char(cadena de text) que té 6 caracters.
// A aquesta variable se li asigna el nom shortAddress que després es fara servir com una Adreça abreviada. (Per generar menys carrega al processador).
char shortAddress[6];

// setup és la funció que inicia el codi i que serveix per a posar en marxa el dispositiu.
//En el setup es fiquen totes les funcions ue en algun moemtn es cridaran desde una llibreria.
void setup() {

  // En aquesta linia es crida la funció begin desde la llibreria SPI.
  // I se li asignan els pins: (18 per al rellotge, 19 per sortida de informació, 5 per entrada de informació).
  // En aquest cas l'argument són els pins que es faran servir.
  SPI.begin(18, 19, 5);
  
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

  // En la següent linia el que es fa és de la llibreria lcd(LiquidCrystal) es crida la funció begin.
  // Es donen els arguments 16 i 2 que són el numero de columans i files que tindra la pantalla.
  lcd.begin(16,2);

  lcd.print("Estoy encendido");
  delay(3000);

  // En la següent linia de codi el que es fa és cridar la funció clear desde la llibreria lcd.
  // En aquesta funció no fa falta introduir cap argument. Ja que no hi ha cap valor etern necessari en la funció.
  lcd.clear();


  // Aqui el que es fa és executar el codi només en cas de que l'objecte estigui definit com a Tag.
  #ifdef IS_TAG

    // Aquesta funcio el que fa iniciar la comunicació DW1000 amb el mode de TAG.
    // La comunicació DW-1000 es una especifica per poder localitzar mitjançant la trilateració.
    // La funció té tres arguments. Device_Adress que hem definit abans, el mode de transmissió de dades i el filtre de rang.
    // El filtre de rang no és necessari així que l'hem selecionat com a 0.
    DW1000Ranging.startAsTag(DEVICE_ADDRESS, DW1000.MODE_LONGDATA_RANGE_ACCURACY, false);

    // En la següent linia el que fem és generar text en la pantalla. El que estigui entre les dues "" serà el que es mostrara a la pantalla LCD.
    lcd.print("Modo: TAG");


    //Deixem temps per a llegir el missatge abans d'eliminar-lo de la pantalla.
    delay(3000);
    lcd.clear();

    // Aquí se li asigna a la variable uwb_data el valor que retorna la funció init_link que esta dins del document link.h.
    uwb_data = init_link();


    // Aquí el que es fa és iniciar una conexió Wifi amb el dispositiu amb el que vols compartir la teva ubicació.
    // A la següent linia de codi el que passa és que es defineix la comunicació Wifi com si el dispositiu fos un un client regular de la xarxa.
    // El que significa és que es connecta com si fos un movil o un ordinador. L'unic que fa falta saber i especificar és el nom de la xarxa i la contrasenya.
    WiFi.mode(WIFI_STA);
    WiFi.begin("La red de Peta", "contraseña");

    // La següent linia el que fa és imprimir conectant mentres que el dispositiu es conecta a la red WIFI.
    // Aquest bucle funciona creant el tipic patro de tres punts que van apareixent mentres diu conectando.
    while (WiFi.status() != WL_CONNECTED) {
      lcd.print("Conectando");
      delay(250);
      lcd.print(".");
      delay(250);
      lcd.print(".");
      delay(250);
      lcd.print(".");
      delay(250);
      lcd.clear();
    }


    // Aquí s'inicialitza ESP_NOW.
    if (esp_now_init() != ESP_OK) {
      lcd.clear();
      lcd.print("Ha Fallao");
      delay(3000);
      lcd.clear();
      return;
    }


    // En la següent linia el que es fa és crear una nova variable amb nome: peerInfo, que té la estructura esp_npm_peer_info_t.
    esp_now_peer_info_t peerInfo;
 
    // La següent linia el que fa és copiar desde la adreça MAC del receptor els numeros darrere d'un 0x i desp´res posar-los a la variable peerInfo.
    // El tercer argument que és 6, vol dir que hi ha sis numeros darrere d'un 0x.
    memcpy(peerInfo.peer_addr, receiverAddress, 6);

    // Aquestes dues linies de codi següents el que fan és determinal el canal en el que el es comunicaran els dispositius i que la conexio no estarà encriptada.
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    // Abans de conectar els dos disposiitius s'ha d'agregar el receptor com a un dispositiu conegut.
    esp_now_add_peer(&peerInfo);


  #endif

  // I ara el que fenm és calcular una adreça més curta per a poder fer servir una adreça més accessible.

  // byte* serveix per a definir quin tipus de dades será la varible. (1 byte= 8 bits)
  // I després es fa servir la funció getCurrentShortAddres() de la biblioteca DW1000Ranging per a definir el valor de la variable que és current short addres.
  byte* currentShortAddress = DW1000Ranging.getCurrentShortAddress();

  // sprintf() és una funció que permet canviar els valors d'una funció ja existent amb un format de dades string.
  // shortAddress és la variable on emmagatzenarem els valors
  // el següent argument és "%02X%02X" les dues comiilles indiquen que és una cadena de text. Mentress que l'interior es pot dividir en dues parts iguals.
  // Cada una de les parts és %02X que vol dir que ha de ser un string amb dos digits. (En cas de que només sigui un digit es fica un 0 a la esquerra)
  sprintf(shortAddress, "%02X%02X", currentShortAddress[1], currentShortAddress[0]);

  lcd.clear();

  // Ara a la pantalla LCD imprimim la adreça curta del dipositiu.
  lcd.print("Short Address:");
  lcd.setCursor(0, 1);
  lcd.print(shortAddress);

  // Deixem un temps per a que l'ususari pugui llegir el missatge
  delay(3000);

  // Ara tornarem a deixar la pantalla lcd com estava.
  lcd.clear();

  // Per ultim, el que fem és avisar de que ja s'ha acabat la posada en marcha i ara comença el codi principal.
  lcd.print("Finiquitao");
  delay(3000);
  lcd.clear();

  pinMode(2, OUTPUT);
}


void loop() {
  // La següent funció és de la biblioteca DW1000Ranging. Que es podría dir que és la que fa tota la feina del loop. 
  // Aquesta funció fa moltes coses. Com per exemple: verificar si es necessita reiniciar le dispositiu, és qui mesura el temps del dispositiu
  // processa els missatges que han de ser enviats i rebuts, entre moltes altres coses. 
  // És la funció que obté la distancia que hi ha desde les ancles i el dispositiu a localitzar.
  // La funció és capaç de saber si el dispositiu és una ancla o un Tag i depenent del que sigui fa una cosa o una altra.
  DW1000Ranging.loop();


  send_json(uwb_data);
}


// En la següent funció el que es fa és enviar l'arxiu Json.
#ifdef IS_TAG
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

  // Esp_now té la caracteristica de que només pot enviar paquest de fins 250 bytes. Per això mateix, em de dividir el nostre Json en diferents paquets
  // En la següent linia es pot veure com calculem el numero de paquets que ha de tindre el nostre Json.
  int numPackets = len / 250;
  if (len % 250 != 0) {
    numPackets++;
  }

  // Ara el que hem de fer és enviar els paquets per separat. El for és una altra manera de fer loops.
  // En la següent linia el que es pot llegir és que es crea la variable i. i Per cada vegada que es fa una volta a i se li suma 1.
  // Per últim es veu com el loop es repeteix mentres que i segui més petita al numero de paquets.
  for (int i = 0; i < numPackets; i++) {

    // Ara hem de calcular en qquin byte ha de començar el paquet i en quin ha d'acabar.
    int Start = i * 250;
    // En aquesta linia següent el que es fa és mirar quin és minim de dos valors, si la llargada del nsotre Json o la suma del començament i el final.
    // D'aquesta manera no es llegeix més enllà del final del Json Buffer.
    int Finish = min(Start + 250, (int)len);

    // I per ultim s'envia el paquet que pertoca. Aquesta part d'aqui (uint8_t *)&, serveix per a que la funció revi les dades amb el format que pertoca.
    // Auqesta part d'aqui Final - Start s'asegura de que només senvii les dades que nosaltres volem enviar. També señala el tamany en bytes del se senviará.
    esp_now_send(receiverAddress, (uint8_t *)&jsonBuffer[Start], Finish - Start);
  }
  digitalWrite(2, HIGH);
  delay(100);
  digitalWrite(2, LOW);


} // Aquí s'acava la funció
#endif


// Aquí es definirá la funció newRange() que es fa servir a la part d'adalt del codi.
void newRange() {
  // Aquí el que es fa és crear una cadena de caracters anomenada buffer. A aquesta cadena se li fiquen com a numero maxim de caracters 21.
  char buffer[21];

  // Aquí per si de cas vuidem la pantalla lcd.
  lcd.clear();

  // La següent linia de codi serveix per "omplir" la cadena de caracters buffer amb la adreça curta del dispositiu.
  // El problema és que la funció DW1000Ranging.getDistantDevice()->getShortAddress() retorna un valor en format int.
  // Així que hem de transformar aquest valor int en una cadena de caracters.
  snprintf(buffer, sizeof buffer, "%04x", DW1000Ranging.getDistantDevice()->getShortAddress());

  // La següent linia de codi el que fa és imprimir en la pantalla el numero del dispositiu al que s'esta connectant.
  lcd.print(buffer);

  // En aquesta linia el que esta fent el nostre dispositiu és grabar dins de la cadena buffer la distancia que hi ha entre el dispositiu i la ancla en qüestió.
  int ret = snprintf(buffer, sizeof buffer, "%.2f", DW1000Ranging.getDistantDevice()->getRange());

  // I per "últim el que fa aquesta linia és "
  lcd.print(buffer);

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