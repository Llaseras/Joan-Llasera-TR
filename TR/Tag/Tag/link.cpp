// En aquest docuemnt s'inclueix el document link.h. No es una llibreria però és per poder fer servir les esttructures i les funcions que té aquest document.
#include "link.h"


// Aquí es defineix una funció anomenada init_link()
struct MyLink *init_link() {

  // Aquí se li asigna a la variable p la llargada/ tamany de la setructura MyLink. 
  // De manera que pugui tindre suficient tamany com per a poder tindre una estructura MyLink dins.
  struct MyLink *p = (struct MyLink *)malloc(sizeof(struct MyLink));

  // Aquí el que es fa és asignar a l'ultim valor de tots el valor NULL.
  // El valor NULL significa que no hi ha res dins. A diferencia del false que significa que no.
  // Quan asignas la structura MyLink a p aquesta variable rep el nom que té cada valor.
  // I quan es defineix next, es defineix com l'ultim valor. A cada nom se li diu un membre.
  // Així que es podria dir que se li asigna al membre next el valor NULL.
  p->next = NULL;

  // Aquí al menbre anchor_addr se li asigna el valor 0.
  p->anchor_addr = 0;

  // El membre range té 3 valors. Així que cada un dels valors que pot emmgatzemar range han de ser definits.
  // El membre range emmagatzema els valors de la distancia entre el el Tag i les tres ancles.
  // Aquí s'asigna el primer valor de range
  p->range[0] = 0.0;

  // Aquí s'asigna el segon valor de range
  p->range[1] = 0.0;

  // Aquí s'asigna el trecer valor de range.
  p->range[2] = 0.0;

  // Aquí el que es fa és tornar el valor de tota la estrucutra p com a resulatt de la funció.
  return p;
} // Aquí és on s'acava la funció.



// A la següent linia de codi el que es fa és definir la funció add_link que es fa servir al codi principal. (linia 381).
// La funció té dos arguemts que són p i addr. Que en el codi principal són uwb_data i device->getShortAddress() respectivament.
void add_link(struct MyLink *p, uint16_t addr) {

  // Aquí el que es fa és crear la variable temp la qual és igual a p.
  struct MyLink *temp = p;

  // Aquí es defineix un loop que segueix funcionant mentres que el valor del membre next de la estructura temp sigui diferent a NULL.
  // En Cristia vol dir que mentres el valor de next sigui diferent de nul (no hi ha res) el bucle seguirá funcionant.
  while (temp->next != NULL) {
    temp = temp->next;
  } // Aquí s'acava el bucle

  // Aqui el que es fa és crear la estructura on s'emmagatzemara la informació de la nova ancla.
  // Es crea la variable "a" amb la estrcutura de MyLink, i a aquest se li dona el tamany i el la forma de aquesta estructura.
  struct MyLink *a = (struct MyLink *)malloc(sizeof(struct MyLink));

  // Aquí el que es fa és donar-li a cadscún dels membres de a el seu valor corresponent.
  a->anchor_addr = addr;
  a->range[0] = 0.0;
  a->range[1] = 0.0;
  a->range[2] = 0.0;
  a->dbm = 0.0;
  a->next = NULL;

  // I per últim el que es fa és agregar el node "a" a la variable temp.
  temp->next = a;

} // Aquí s'acava la funció.


// La següent funció és find_link. Aquesta funció no apareix al programa principal, però si que es troba a una funció que esta més avaix en aquest document.
// Aquesta variable retorna un valor amb la estructura de MyLink i té dos arguments.
// El primer arguemnt és p i el segon és addr que quan es crida la funció és exactament el mateix. Sol que d'aquella funció, aquests són els valors:
// uwb_data i DW1000Ranging.getDistantDevice()->getShortAddress() que són la infromació de la conexió i la adreça curta.
struct MyLink *find_link(struct MyLink *p, uint16_t addr) {

  // La següent linia de codi significa que si la adreça és igual a 0, la funció no ha de retornar cap valor.
  if (addr == 0) {  return NULL; }

  // Aquesta linia significa que si el membre ext de p és igual a NULL (esta vuit), la funció no ha de tornar cap valor.
  if (p->next == NULL) { return NULL; }

  // La següent linia el que fa és crear una variable amb la estructura MyLink. i li dona el valor de p.
  struct MyLink *temp = p;

  // Aquí el que es fa és activar un loop. Aquest loop el que fa és que funciona menters que el valor next de la fucnió temp sigui diferent de nul.
  while (temp->next != NULL) {

    // Aqui el que es fa és passar temp al següent node de la llista i així cada vegada que es repetixi el bucle.
    temp = temp->next;

    // Aquesta part de aquí el que es fa és comprovar si el valor de la direcció és igual al valor de la direcció ja guardada
    // i en cas de que ho sigui, retorna la informació sobre aquell link que actualemnt esta guardada a temp.
    if (temp->anchor_addr == addr) {
      return temp;
    } // Aquí s'acava la part del if que és el que dona la condició per a que la part que esta entre els "{}" funcioni.
  } // Aquí s'acava tot el codi que es repeteix amb el loop.

  // En cas de que no es trobi cap adreça el que es fa és retornar NULL. I també s'envia un missatge dient que no s'ha trobat res.
  return NULL;
} // Aquí s'acava la funció find_link()


// Aquí és on definirem la funció update_link() que apareix al codi pricipal.
// Aquesta funció es fa servir per actualitzar les dades de la conexió entre el dispositiu i l'ancla.
// Aquesta funció té 4 arguments que són p, addr, range i dbm. Que són la informació de la conexió, la adreça curta, la distancia i la potecnia de la conexió respectivament.
void update_link(struct MyLink *p, uint16_t addr, float range, float dbm) {

  // Aquí es crea la variable temp amb la estructura de MyLink. El contingut de aquesta variable és el que retorna la funció find_link.
  struct MyLink *temp = find_link(p, addr);

  // Aquí el que es fa és executar el codi només en cas de que el temp no sigui igual a null (esta vuit). El valor null també es pot representar amb "/0".
  if (temp != NULL) {

    // Aquí és on es canvien totes les distancies del dispositiu.
    temp->range[2] = temp->range[1];
    temp->range[1] = temp->range[0];
    temp->range[0] = (range + temp->range[1] + temp->range[2]) / 3;
    temp->dbm = dbm;
    return; // Finalitza la funció.
  }

  // Aquesta altra part del codi s'executa en cas de que l'altra no s'executi.
  else {
    return; // Finalitza la funció. No és necessari ficar el return ja que la funció és void per tant no retornara en cap cas un valor.
  }
}


// Aquesta és la ultima funció que es definirá en aquest document. Aquesta funció surt al final del codi principal.
// Com es pot veure aquesta funció té dos arguments, qué són p (la informació de la conexió) i addr (que és la adreça escurçada de la ancla a la que esta conectat)
void delete_link(struct MyLink *p, uint16_t addr) {

  // Aquí el que es fa és que en cas de que el valor de la funció curta sigui 0, la funció no s'arrivi a executar.
  if (addr == 0) { return; }

  // a la variable temp se li asigna el valor de p. Els dos tenen la mateixa estructura que és MyLink.
  struct MyLink *temp = p;

  // Aquí s'inicia el bucle que es fa servir per eliminar la ancla de la conexió.
  while (temp->next != NULL) {

    // Aquesta part d'aqui el que fa és comprovar que la informació de temp sigui la que tingui que eliminar comprovant la adreça curta proporcionada a la funció i la adreça curta de temp.
    if(temp->next->anchor_addr == addr) {

      // A la següent linia el que es fa és guardar la part de temp que es vol eleiminar.
      struct MyLink *del = temp->next;

      // Això el que fa és suprimir la part de la variable que ja no es voldra fer servir.
      // Això es fa canviant la part que es vol eliminar amb la part que és posterior a aquesta.
      temp->next = del->next;

      // Aquí el que es fa és alliverar el valor del per així poder ahorrar la maxima quantitat possible de recursos.
      free(del);
      return; // Aquí el que es fa és finalitzar la funció.
    }
  temp = temp->next;
  }
  return; // Aquesta part d'aqui es fa servir només en cas de que no s'hagi trobat la part de la variable temp que te la mateixa adreça curta que la proporcionada.
}
