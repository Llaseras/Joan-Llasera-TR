// S'afegeixen les llibreries necessaries.
#include <stdio.h> // Per modificar strings (cadenes de caracters)
#include <stdlib.h> // Més del mateix
#include <string.h> // Adivina per el nom per a que serveix
#include <json.h> // Per modificar, serialitzar/deserialitzar documents JSON.
#include <libserialport.h> // Per fer conexionns per el port serial
#include <Windows.h> // per poder generar finestres també.
#include <SDL.h> // Per poder generar finestres
#include <SDL_ttf.h> // Per poder escriure text en les finestres creades
#include <SDL_image.h> // Per poder ficar images en la finestra
#include <SDL2_gfxPrimitives.h> // Per poder representar figures en la pantalla
#include <iostream> // Per poder retornar una cadena de floats.
#include <math.h> // Aquesta llibreria serveix per fer calculs matematics
#include <SDL_syswm.h> // Per fer algunes operacions amb les finestres de Windows.
#include <fstream>

// En el meu cas el port es diu COM7
#define Port_del_portatil "COM7"
#define Frequencia 115200
#define Tamany_del_buffer 501

// Això és per declarar la funció de la finestra inicial. El CALLBACK el que fa és que no està escrit explicitament al
// codi quan s'executa la funció, sinó que cada vegada que alguna cosa succeeix a la pantalla aquesta finció es crida.
LRESULT CALLBACK Posición_Anclas(HWND hDlg, UINT misatge, WPARAM wParam, LPARAM lParam);

// Anem a declarar totes les globals (Valors que es poden fer servir en tot el text.)
SDL_Window* ventana;
SDL_Renderer* renderitzador;
TTF_Font* fuente;
SDL_Texture* fondo; 
SDL_Texture* TAG;
SDL_Texture* Ancla_1;
SDL_Texture* Ancla_2;
SDL_Texture* Ancla_3;

// Ara creem una variable on es guardara la posicio on sortira el text.
SDL_Rect sitio_x, sitio_y, sitio_ancla1, sitio_ancla2, sitio_ancla3, sitio_TAG;


// També hem de definir les globals que recolliran les coordenades de les ancles.
// Si fiquem les ancles en una altre banda, hem de canviar aquests numeros.
// Els valors de dins representen (x àncora 1, y àncora 1, x àncora 2...)
float Anchor_cords[6] = {0.0, 0.0, 100.0, 0.0, 100.0, 100.0};

// Definim una variable que tindra un array amb la adreça de les tres ancles.
uint64_t adreces[3];

// Ara definim el nom d'una altra variable amb un array que tindra dins el valor de les distancies.
// Values[0,1,2,] son per la d1, [3,4,5] son per la d2 i [6,7,8] per la d3.
int values[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

// Ara crarem tres variables globals que emmagatzemaran el valor de x, y i z.
double x; //La linia més facil del codi. XD
double y;
double z;

// Ara hem de definir el colo que tindra la nostra lletra. També és una global, pero es pot definir el seu valor directament.
SDL_Color color = {0, 0, 0, 255}; // Vermell, Verd, Blau i opacitat.

// Aqui el que es fa és crear la funció error_exit()
// Aquesta funció té un argument que és el missatge d'error que s'acavará imprimint a la pantalla.
void error_exit(const char *message) {

    // Imprimeix el codi d'error que sortirá a la pantalla.
    fprintf(stderr, "%s\n", message);

    // Finalitza el codi senser.
    exit(EXIT_FAILURE);
}

void start_window(){

    // Iniciem la pantalla. Per`´o no la obrim ni la representem. Això és simplement per a que hi hagi una finestra incialitzada.
    SDL_Init(SDL_INIT_VIDEO);

    // Iniciem ttf per poder escriure text en la finestra que mostrarem. No nesessita cap argument.
    TTF_Init();

    // Ara iniciem image per poder ficar imatges en la finestra emergent.
    IMG_Init(IMG_INIT_JPG);
    
    //Ara creem una varible que tingui les dades de la finestra que es creará.
    SDL_DisplayMode Modo;

    // Omplim la variable amb la actual configuració que té la pantalla. Està la amplada de la pantalla, la altura i la tasa de refresc de la pantalla.
    SDL_GetCurrentDisplayMode(0, &Modo);

    // Obrim la pantalla i guardem la pantalla com una variable. Per facilitar el seu tancament i processament.
    ventana = SDL_CreateWindow("Joan Llasera TR - Localitzador", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1366, 768, SDL_WINDOW_FULLSCREEN);

    // Ara creem un creem un renderitzador per poder generar imatges i elements en la finestra. Creem una variable que guardi aquest renderitzador.
    renderitzador = SDL_CreateRenderer(ventana, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // Ara creem una font amb la que s'escriura el text. Més que crear escollir. També seleccionem el tamany de la lletra
    fuente = TTF_OpenFont("Fonts/RobotoRegular.ttf", 18);

    // Ara seleccionem una imatge que sera el nostre fondo.
    fondo = IMG_LoadTexture(renderitzador, "Textures/FondodelTR.jpg");

    // A més a més del fondo tabé hem de carregar el text que sortirá incialment en pantalla
    SDL_Surface* superficie_x = TTF_RenderText_Solid(fuente, "Posicio x:", color);
    SDL_Texture* textura_x = SDL_CreateTextureFromSurface(renderitzador, superficie_x);
    SDL_Surface* superficie_y = TTF_RenderText_Solid(fuente, "Posicio y:", color);
    SDL_Texture* textura_y = SDL_CreateTextureFromSurface(renderitzador, superficie_y);

    // Ara determinem on s'imprimira el text
    sitio_x.x = 15;
    sitio_x.y = 15;
    sitio_x.w = superficie_x->w;
    sitio_x.h = superficie_x->h;

    sitio_y.x = 15;
    sitio_y.y = 40;
    sitio_y.w = superficie_y->w;
    sitio_y.h = superficie_y->h;

    // Alliverem superficie perque no el farem servir més.
    SDL_FreeSurface(superficie_x);
    SDL_FreeSurface(superficie_y);

    // Reiniciem la variable renderitzador perque no hi hagi res dins de la variable.
    SDL_RenderClear(renderitzador);

    // Ara afegim dins del renderitzador el fons. (Si ho afegim primer, les demes coses es veuran per sobre.)
    // Els dos NULL, són el tamany x, y de la fotografia que volem agafar. Amb els dos nulls diem;
    // que la fotografia s'escali sola i que agafara la imatge sencera.
    SDL_RenderCopy(renderitzador, fondo, NULL, NULL);

    // Fem el mateix per ficar el text.
    SDL_RenderCopy(renderitzador, textura_x, NULL, &sitio_x);
    SDL_RenderCopy(renderitzador, textura_y, NULL, &sitio_y);

    // Per ultim hem de mostrar en pantlla tot el que esta en la variable renderitzador.
    SDL_RenderPresent(renderitzador);

    // Per ultim lliberem tot el espai que estaven utilitzant les dues textures.
    SDL_DestroyTexture(textura_x);
    SDL_DestroyTexture(textura_y);
}

void close_program(sp_port *port) {
    // Tanca el port per a poder-lo fer servir futurament.
    sp_close(port);

    // Deixa lliure el port per poder-lo fer servir en el futur per un altre programa
    sp_free_port(port);


    // Tanquem la finestra i SDL i totes les llibreries.
    SDL_DestroyTexture(fondo);
    TTF_CloseFont(fuente);
    SDL_DestroyRenderer(renderitzador);
    SDL_DestroyWindow(ventana);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

// Aquesta funció fara la trilateració per poder obtindre la localització del TAG
void Trilateracion(char *buffer) {
    // El primer de toto que hem de fer és trobar quins son els valors
    // En aquest cas, la d sempre será igual al valor numero 3 de l'array Anchor_cords.
    // la j sempre sera igual al valor numero 5 de l'array Anchor_cords.
    // Per ultim la i és igual al valor numero 6 de l'array Anchor_cords.
    float d = Anchor_cords[2];
    float j = Anchor_cords[4];
    float i = Anchor_cords[5];

    // També hem de definir els valors que són més rellevants a l'hora de fer la trilateració.
    // Aquests valors es defineixen com la mitjana dels tres ultims valors del radi de cada ancora.
    double r1, r2, r3;

    // Aquí el que fem és 'analitzar' el buffer en busca dels valors que volem agafar.
    sscanf(buffer, "%*[^\n]\n%f %f %f%*[\n]", &r1,  &r2, &r3);

    // Ara anem a obtenir la posició del Tag.
    // Per obtenir la posició del Tag, el que hem de fer, és fer servir, les aquacions de la trilateració. Que per cert, estan al Marc Teoric del meu TR.
    x = ((r1 * r1) - (r2 * r2) + (d * d)) / (2 * d);
    y = (((r1 * r1) - (r3 * r3) + (i * i) + (j * j)) / (2 * j)) - ((i / j) * x);
    z = sqrt(((r1 * r1) - (x * x) - (y * y)));
}

// Una funció que ens donará la posició de les anclas.
void Anchors_get_position() {

    // Ara farem servir aquesta funció per crear les textures de les ancles i definir la seva posició.
    Ancla_1 = IMG_LoadTexture(renderitzador, "Objectes/Ancla1.jpg");
    Ancla_2 = IMG_LoadTexture(renderitzador, "Objectes/Ancla2.jpg");
    Ancla_3 = IMG_LoadTexture(renderitzador, "Objectes/Ancla3.jpg");

    // Posicio Ancla 1
    sitio_ancla1.x = (int)(Anchor_cords[0] * 1) + 20;
    sitio_ancla1.y = (int)(Anchor_cords[1] * 1) + 70;
    sitio_ancla1.w = 40;
    sitio_ancla1.h = 40;

    // Posicio Ancla 2
    sitio_ancla2.x = (int)(Anchor_cords[2] * 1) + 20;
    sitio_ancla2.y = (int)(Anchor_cords[3] * 1) + 70;
    sitio_ancla2.w = 40;
    sitio_ancla2.h = 40;

    // Posicio Ancla 3
    sitio_ancla3.x = (int)(Anchor_cords[4] * 1) + 20;
    sitio_ancla3.y = (int)(Anchor_cords[5] * 1) + 70;
    sitio_ancla3.w = 40;
    sitio_ancla3.h = 40;

    // Ara ja de pasada aprofitarem per crear la textura del TAG.
    TAG = IMG_LoadTexture(renderitzador, "Objectes/TAG.jpg");

    // Nomes farem la textura ja que definir la seva posició es fara cada itineració del bucle principal.
    // Pero el que si que podem fer és definir que tan gran será la foto del TAG
    sitio_TAG.w = 30;
    sitio_TAG.h = 30;
}

void mostrar_valores(int numero) {
    // Ara hem de mostrar tots els valors i a més a més també hem de mostrar les posiscion de les ancles i el TAG
    // El primer de tot és buidar el renderitzador per a que no se superposin les textures.
    SDL_RenderClear(renderitzador);

    // Ara afegim el fons al renderitzador.
    SDL_RenderCopy(renderitzador, fondo, NULL, NULL);

    // Creem dues codenes de caracters.
    char posicio_x[25], posicio_y[25];

    // Omplim les dues cadenes de caracters amb el text que nosaltres li volem ficar
    snprintf(posicio_x, sizeof(posicio_x), "Posicio x: %.2f", x);
    snprintf(posicio_y, sizeof(posicio_y), "Posicio y: %.2f", y);

    // Ara creem superficies que amb el text.
    SDL_Surface* superficie_x = TTF_RenderText_Solid(fuente, posicio_x, color);
    SDL_Surface* superficie_y = TTF_RenderText_Solid(fuente, posicio_y, color);

    // Ara el que fem és crea una textura per cada superficie que hem creat abans.
    SDL_Texture* textura_x = SDL_CreateTextureFromSurface(renderitzador, superficie_x);
    SDL_Texture* textura_y = SDL_CreateTextureFromSurface(renderitzador, superficie_y);

    // Hem de tornar a modificar les mides de les textures
    sitio_x.h = superficie_x->h;
    sitio_x.w = superficie_x->w;
    sitio_y.h = superficie_y->h;
    sitio_y.w = superficie_y->w;

    // Ara ja podem alliberar el espai que feien servir les superficies.
    SDL_FreeSurface(superficie_x);
    SDL_FreeSurface(superficie_y);

    // Ara afegim al renderitzador les dues textures.
    SDL_RenderCopy(renderitzador, textura_x, NULL, &sitio_x);
    SDL_RenderCopy(renderitzador, textura_y, NULL, &sitio_y);

    //Per ultim el que fem és lliberar el espai que ocupen les textures.
    SDL_DestroyTexture(textura_x);
    SDL_DestroyTexture(textura_y);

    // Ara hem de agregar les ancles al renderitzador.
    SDL_RenderCopy(renderitzador, Ancla_1, NULL, &sitio_ancla1);
    SDL_RenderCopy(renderitzador, Ancla_2, NULL, &sitio_ancla2);
    SDL_RenderCopy(renderitzador, Ancla_3, NULL, &sitio_ancla3);


    // Ara farem que la posició del TAG només s'imprimeixi en cas de que tinguem informació de la seva posició.
    if (numero == 1) {
        // Ara per utlim he de determinar la posicio on estará el nostre TAG.
        sitio_TAG.x = (int)(x * 1) + 20;
        sitio_TAG.y = (int)(y * 1) + 70;

        // Ara per ultim he de afegir la textura del tag en la posicio corresponent.
        SDL_RenderCopy(renderitzador, TAG, NULL, &sitio_TAG);
    }

    // Ara crearem una textura que només es mostrara en cas de que el document json no s'estigui llegint.
    if (numero == 0) {
        SDL_Surface* superficie_error_buffer = TTF_RenderText_Solid(fuente, "No se esta recibiendo ningun elemento por el puerto Serial", color);
        SDL_Texture* ERROR_buffer = SDL_CreateTextureFromSurface(renderitzador, superficie_error_buffer);
        SDL_Rect Sitio_ERROR_buffer;
        Sitio_ERROR_buffer.x = 20;
        Sitio_ERROR_buffer.y = 735;
        Sitio_ERROR_buffer.w = superficie_error_buffer->w;
        Sitio_ERROR_buffer.h = superficie_error_buffer->h;
        SDL_FreeSurface(superficie_error_buffer);
        SDL_RenderCopy(renderitzador, ERROR_buffer, NULL, &Sitio_ERROR_buffer);
        SDL_DestroyTexture(ERROR_buffer);
    }

    // Ara per ultim, el que fem, es imprimir tot en pantalla.
    SDL_RenderPresent(renderitzador);
}

// Ara es crea la funció read_from_serial que serveix per llegir dades desde el port COM7 que és on esta conectat l'ESP32.
// En aquesta funció hi ha un argument anomenat port que te la structura sp_port.
void read_from_serial(struct sp_port *port) {

    // Es crea un Buffer on es guardaran els caracters que vinguin per el port serial.
    char buffer[100];

    // Es crea una variable amb el nom n.
    int n;

    // Creem una variable que guardará quin event s'ha fet,
    SDL_Event evento;

    // Creem una variable que es diu quit.
    bool quit = false;

    // Ara fem els calculs de on estaran les ancores dibuixades.
    Anchors_get_position();

    // Fiquem una mica de retrás
    SDL_Delay(500);

    // Ara farem que es vegi la posicio de le ancores.
    mostrar_valores(-1);

    // Ara es crea un loop fins que quit sigui positiu.
    while (!quit) {
        
        // A la variable n anteriorment guardada se li asigna el numero de bytes que la funció sp_nonblocking_read ha llegit.
        // Les dades que ha llegit la funció es llegeixen desde el port on està conectat la funció, es guarden al buffer abans creat.
        // I per últim Buffer_Size - 1 és el tamany maxim de dades que es poden emmagatzemar. El -1 per poder ficar al final del buffer un valor NULL.
        

        n = sp_nonblocking_read(port, buffer, 99);

        // Aquasta part d'aqui del codi només funciona en cas de que s'hagin registrat dades.
        if (n > 0) {

            // Ara anem a fer la trilateració de les dades obtingudes per saber on està el nostre TAG.
            Trilateracion(buffer);

            // Aquí fem servir una variable que mostara tots el valors.                
            mostrar_valores(1);


        } else if (n < 0) {
            // Això s'executa en cas de que no es llegeixi res desde el port USB on esta el ESP32 connectat.
            error_exit("Error reading from serial port");
        } else if (n == 0) {
            mostrar_valores(0);
        }

        // Si s'executa SDL_PollEvent continuará
        if (SDL_PollEvent(&evento)) {

            // Si el tipus de event que ha retornat és igual a SDL_QUIT.
            if (evento.type == SDL_QUIT) {

                // Fem que quit sigui true per parar el bucle i acabar el codi.
                quit = true;
            }
        }
    }
}

// Ara aqui es deifineix el codi principal. // Tots els valors que té com a argument els dona Windows al començar el codi.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    // Es crea una variable anomenada port que té la structura sp_port.
    struct sp_port *port;

    // Aquesta funció esta feta per poder obtenir informació sobre el port on esta connectada la ESP32.
    // Aquesta informació obtinguda es guardara a la variable port.
    // I es crea una variable anomenada error que emmagatzema si la conexió ha sigut correcte o no.
    sp_return error = sp_get_port_by_name(Port_del_portatil, &port);

    // Aquí el que fem és que si a la variable error no s'ha emmagatzemat SP_OK enviem un codi d'error.
    if (error != SP_OK) {
        // Crida a la funció error_exit que hem fet abans per imrprimir el missatge i aturar el procés.
        error_exit("No se encuentra el puerto Serial");
    }

    // Reaprofitem la variable error que ja no necessitarem l'anterior valor.
    // A la variable error se li asigno el valor depenent de si la comunicació per el port USB s'inicia correctament.
    // es fa servir la funció sp_open() per iniciar la comunicació. I es fica que vol que fagi la comunicació. Si llegir, escriure o ambdues.
    error = sp_open(port, SP_MODE_READ_WRITE);

    // En cas de que no s'hagi iniciat correctament la comunicació, s'imprimeix l'error i s'aturen tots els procesos.
    if (error != SP_OK) {
        error_exit("No se puede abrir el puerto Serial");
    }

    // Ara es torna a reaprofitar error.
    // sp_set_baudrate el que fa és indicar a quina frequencia el port Serial llegirá les dades.
    error = sp_set_baudrate(port, Frequencia);

    // Si no es pot especificar la freqüencia enviara un codi d'error i acabará tots els processos.
    if (error != SP_OK) {
        error_exit("No s'ha pogut indicar una freqüencia.");
    }

    sp_set_bits(port, 8);
    sp_set_parity(port, SP_PARITY_NONE);
    sp_set_stopbits(port, 1);
    sp_set_flowcontrol(port, SP_FLOWCONTROL_NONE);

    // Iniciem una funció que crea una finestra i ens retorna la informació de la finestra creada.
    start_window();

    // Cridem una funció que es repeteix constantment fins que nosaltres volem apagar el codi.
    read_from_serial(port);

    close_program(port);

    // Acabem el codi
    return 0;
}
