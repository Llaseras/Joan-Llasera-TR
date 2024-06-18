// Aquí s'inclueix la llibreria Arduino.h. Incluir una llibreria serveix per poder fer servir les funcions d'aquesta en una altre docuement de text.
#include <Arduino.h>

//Tot el codi de les funcions que estan escrites en aquest document està al document link.cpp.


// Aquí el que es fa és definir un tipus de structura anomenada MyLink
// Aquesta estructura té 4 membres: anchor_addr, rangec(que té tres valors), dbm i next.
struct MyLink {
  uint16_t anchor_addr;
  float range[3];
  float dbm;
  struct MyLink *next;
};

// Aquestes són totes les funcions que té el document link.h i el seu codi està al document link.cpp.
// Totes aquestes funcions no són d'una llibreria, sinó que són uniques per a aquest programa.

struct MyLink *init_link();
void add_link(struct MyLink *p, uint16_t addr);
struct MyLink *find_link(struct MyLink *p, uint16_t addr);
void update_link(struct MyLink *p, uint16_t addr, float range, float dbm);
void delete_link(struct MyLink *p, uint16_t addr);
