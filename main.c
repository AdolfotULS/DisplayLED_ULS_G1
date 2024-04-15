// --------------- LIBRERIAS ---------------

#include <signal.h>  // Biblioteca para manejar señales
#include <stdio.h>   // Biblioteca para entrada/salida
#include <pigpio.h>  // Biblioteca para controlar GPIO de Raspberry Pi
//#include <unistd.h> //Testing
//#include <time.h> //Testing

// --------------- GLOBALES ---------------

#define TAMANO 8 // Tamaño matriz de LEDs
#define FOTOGRAMAS_POR_SEGUNDO 30 // Velocidad de otoframas por segundo para imagenes
#define TIEMPO_POR_FOTOGRAMA (1.0 / FOTOGRAMAS_POR_SEGUNDO) // Tiempo de demora de un frame
#define FOTOGRAMAS_POR_SEGUNDO_ANIMACION 3 // Velocidad de fotogramas para animaciones
#define TIEMPO_POR_FOTOGRAMA_ANIMACION (1.0 / FOTOGRAMAS_POR_SEGUNDO_ANIMACION) // Teimpo de demora por imagen de la animacion

const int pines_positivos[TAMANO] = {26, 19, 13, 6, 5, 11, 9, 10}; // Pines Positivos GPIO | Columnas

const int pines_negativos[TAMANO] = {21, 20, 16, 12, 7, 8, 25, 24}; // Pines Negativos GPIO | Filas

volatile sig_atomic_t senal_recibida = 0; // Almacenar la senal recibida de consola

// --------------- DECLARACIONES ---------------
// Declaraciones de todas las funciones para llamarlas de cualquier parte del codigo

int inicializar_gpio();
void finalizar_gpio();
int mostrar_menu();
void copiar_imagen();
void extraer_frame();
void renderizar_animacion();
void renderizar_animacion_2();
void renderizar_imagen();
void renderizar_imagen_2();
void mostrar_fotograma();
void mostrar_fotograma_2();
void testear_y_mostrar_leds()
void senal_led_coordinado();
void control_led();
void mostrar_menu();
void copiar_imagen();

// --------------- MAIN ---------------

/*  
    Programa Principal
*/

void main() {  
    //Inicializacon de los GPIO y tablero de leds.
    if (inicializar_gpio() == 0) {
        printf("No se ha podido Inicializar GPIO.\n")
        return 1;
    }
    signal(SIGINT, sigint_handler);

    //Menu
    int opcion = 0;
    while (opcion != 3)
    {
        opcion = mostrar_menu();
    }

    finalizar_gpio();
    
    return 0;
}

// --------------- DISPLAY --------------

//TIEMPO TOTAL DE LA IMAGEN EN PANTALLA

//SUMA DE TODO EL TIEMPO QUE TOME REFRESCAR LA CANTIDAD DE FRAMES_POR_SEGUNDOS NO DEBE DE SER SUPERIOR A seg_duracion
void renderizar_animacion(int num_repeticiones,float seg_duracion_por_imagen, int animacion[][TAMANO][TAMANO]) {
    for (int repeticion = 0; repeticion < num_repeticiones; repeticion++) {
        for (int i = 0; i < num_imagenes; i++) {
            renderizar_imagen(seg_duracion_por_imagen, animacion[i]);
        }
    }
}

//TIEMPO
void renderizar_animacion_3(float seg_duracion, int animcion[][TAMANO][TAMANO], int frames_animacion) {
    int frame_actual = 0; //Frame que esta mostrando
    int frames_animacion = frames_animacion - 1; // Cuantos frames contiene la animacion
    double tiempo_inicio = time_time();

    // Bucle que renderiza la imagen por x tiempo
    // Tiene que mostrar los todos los frames de animacion, y volver a repetirse por el x tiempo
    // Finalizar luego del x tiempo
    while ((time_time() - tiempo_inicio) < seg_duracion)
    {
        if (frame_actual > frames_animacion) { // La animacion termino entonces se repite
            frame_actual = 0;
        }

        int imagen_actual[TAMANO][TAMANO]; //Imagen - Frame
        extraer_frame(frame_actual, animacion, imagen_actual); // Extraer el frame que corresponde
        renderizar_imagen(TIEMPO_POR_FOTOGRAMA_ANIMACION, frame_actual); // Renderizar imagen por el tiempo correspondiente
        frame_actual++;
    }
    
}

void renderizar_animacion_2(float seg_duracion, int animcion[][TAMANO][TAMANO], int frames_animacion) {
    int frames_totales = seg_duracion * FOTOGRAMAS_POR_SEGUNDO_ANIMACION; //Cuantos fotogramas se pueden en ese tiempoo
    int frame_contador = 0; //Frame actual hasta los totales
    int frame_actual = 0; //Frame que esta mostrando
    int frames_animacion = frames_animacion - 1; // Cuantos frames contiene la animacion

    // Bucle que renderiza la imagen por x tiempo
    // Tiene que mostrar los todos los frames de animacion, y volver a repetirse por el x tiempo
    // Finalizar luego del x tiempo
    while (frame_contador < frames_totales)
    {
        if (frame_actual > frames_animacion) { // La animacion termino entonces se repite
            frame_actual = 0;
        }

        int imagen_actual[TAMANO][TAMANO]; //Imagen - Frame
        extraer_frame(frame_actual, animacion, imagen_actual); // Extraer el frame que corresponde
        renderizar_imagen(TIEMPO_POR_FOTOGRAMA_ANIMACION, frame_actual); // Renderizar imagen por el tiempo correspondiente
        frame_actual++;
        frame_contador++;
    }
    
}

void renderizar_imagen(float seg_duracion, int imagen[TAMANO][TAMANO]) {
    int frames_totales = seg_duracion * FOTOGRAMAS_POR_SEGUNDO ; //CUanto frames en el x tiempo

    int imagen_actual[TAMANO][TAMANO];
    copiar_imagen(imagen, imagen_actual);
    
    int frames = 0;
    while (frames < frames_totales)
    {
        mostrar_fotograma(imagen_actual);
        frames++; 
    }
}

//TIEMPO
void renderizar_imagen_2(float seg_duracion, int imagen[TAMANO][TAMANO]) {
    double tiempo_inicio = time_time();

    int imagen_actual[TAMANO][TAMANO];
    copiar_imagen(imagen, imagen_actual);
    
    int frames = 0;
    while ((time_time() - tiempo_inicio) < seg_duracion) {
        mostrar_fotograma(imagen_actual);
        frames++;
    }
}


void mostrar_fotograma(int imagen[TAMANO][TAMANO]) {
    // Tiempo de inicio del fotograma
    double tiempo_inicio = time_time();
    // Calcular el tiempo de espera para cada LED
    double tiempo_espera = TIEMPO_POR_FOTOGRAMA / (TAMANO * TAMANO);

    // Mostrar el fotograma en la matriz de LEDs
    for (int columna = 0; columna < TAMANO; columna++) {
        for (int fila = 0; fila < TAMANO; fila++) {
            int valor_pixel = imagen[TAMANO][TAMANO];
            if (valor_pixel == 1) {
                senal_led_coordinado(fila, columna, 1);
                time_sleep(tiempo_espera);
                senal_led_coordinado(fila, columna, 0);
            } else {
                time_sleep(tiempo_espera);
            }
        }
    }

    // Calcular el tiempo transcurrido
    double tiempo_transcurrido = time_time() - tiempo_inicio;
    
    // Verificar si el tiempo transcurrido es menor que TIEMPO_POR_FOTOGRAMA
    if (tiempo_transcurrido < TIEMPO_POR_FOTOGRAMA) {
        // Esperar el tiempo restante del fotograma
        time_sleep(TIEMPO_POR_FOTOGRAMA - tiempo_transcurrido);
    }
}

// EN CASO DE NO FUNCIONAR EL PRIMERO
void mostrar_fotograma_2(int imagen[TAMANO][TAMANO]) {
    // Calcular el tiempo de espera para cada LED
    double tiempo_espera = TIEMPO_POR_FOTOGRAMA / (TAMANO * TAMANO);

    // Mostrar el fotograma en la matriz de LEDs
    for (int columna = 0; columna < TAMANO; columna++) {
        for (int fila = 0; fila < TAMANO; fila++) {
            int valor_pixel = imagen[TAMANO][TAMANO];
            if (valor_pixel == 1) {
                senal_led_coordinado(fila, columna, 1);
                time_sleep(tiempo_espera);
                senal_led_coordinado(fila, columna, 0);
            } else {
                time_sleep(tiempo_espera);
            }
        }
    }
}

// --------------- CONTROLADOR ---------------

/*
    Funcion: Probar los leds.
    Ingreso: NADA.
    Salida: NADA.
    Detalles: Prende todos los leds en rapida sucesion con respecto
    a una imagen predeterminada.
    Autores: Benjamin M.
*/
void testear_y_mostrar_leds() {
    for (int columna = 0; columna < TAMANO; columna++) { // Recorre los pines positivos
        for (int fila = 0; fila < TAMANO; fila++) { //Recorres pines negativos
            senal_led_coordinado(fila, columna, imagen[fila][columna]); // Mostrar la imagen
            time_sleep(0.0001); // Tiempo que estara prendido el led
            senal_led_coordinado(fila, columna, 0); // Apagar el LED
            
        } 
    }
}

/*
    Funcion: Inicializar LEDS y Pines.
    Ingreso: Arreglo de los pines postivos y negativos.
    Salida: NADA.
    Detalles: Tiene como objetivo iniciar los GPIO para
    hacer uso de las leds y iniciarlas apagarlas.
    Autores: Benjamin M.
*/
void inicializar_leds(int pines_positivos[TAMANO], int pines_negativos[TAMANO]) {
    for (int pin = 0; pin < TAMANO; pin++) { // Indice del pin
        gpioSetMode(pines_positivos[pin], PI_OUTPUT); // Poner pines GPIO positivos en OUTPUT
        gpioSetMode(pines_negativos[pin], PI_OUTPUT); // Poner pines GPIO positivos en OUTPUT
        control_led(pines_positivos[pin], pines_negativos[pin], 0); // Apagar LED
    }
}


/*
    Funcion: Finalizar LEDS y Pines.
    Ingreso: Arreglo de los pines postivos y negativos.
    Salida: NADA.
    Detalles: Tiene como objetivo apagar las leds y luego
    finalizar la salida de datos de los GPIO.
    Autores: Benjamin M.
*/
void finalizar_leds(int pines_positivos[TAMANO], int pines_negativos[TAMANO]) {
    for (int pin = 0; pin < TAMANO; pin++) { // Indice del pin
        control_led(pines_positivos[pin], pines_negativos[pin], 0); // Apagar LED
        gpioSetMode(pines_positivos[pin], PI_INPUT); // Poner pines GPIO positivos en INPUT
        gpioSetMode(pines_negativos[pin], PI_INPUT); // Poner pines GPIO negativos en INPUT
    }
}

/*
    Funcion: Prender y apagar LEDs con respecto a su posicion en dos dimensiones.
    Ingreso: Fila y columna de la posicion, estado de la led.
    Salida: NADA.
    Detalles: Facilita el prendido de apagado de las leds segun una posicion 
    virtual de las leds en un arreglo de dos dimensiones.
    Autores: Benjamin M. | Mejoras: Adolfo T.
*/
void senal_led_coordinado(int fila, int columna, int estado) {
    int pin_positivo = pines_positivos[columna]; // Obtener el valor del pin positivo en la posicion
    int pin_negativo = pines_negativos[fila]; // Obtener el valor del pin negativo en la posicion
    control_led(pin_positivo, pin_negativo, estado); // Cambiar el estado del LED
}

/*
    Funcion: Prender y apagar led segun su posicion de pines.
    Ingreso: Pin positivo y negativo de una led correspondiente, su estado.
    Salida: NADA.
    Detalles: Facilita el cambio de estado de una leds unicamente con sus
    pines correspondientes.
    Autores: Adolfo T.
*/
void control_led(int pin_positivo, int pin_negativo, int estado) {
    if (estado) { // Si el estado es 1 entonces se quiere pender el led.
        gpioWrite(pin_positivo, PI_HIGH); // Enviar Señal Positiva al pin positivo
        gpioWrite(pin_negativo, PI_LOW); // Enviar señal negativa al pin negativo
    } else { // El estado no es 1 entonces apagara el led
        gpioWrite(pin_positivo, PI_LOW); // Enviar Señal negativa al pin positivo
        gpioWrite(pin_negativo, PI_HIGH); // Enviar señal positiva al pin negativo
    }
}

/*
    Funcion: Inicializar GPIO y LEDs
    Ingreso: NADA
    Salida: Devuelve si se pudo inicializar correctamente.
    Detalles: Tiene como objetivo iniciar a gpio y las
    leds.
    Autores: Adolfo T. & Benjamin M.
*/

int inicializar_gpio() {
    // Inicializar GPIO
    if (gpioInitialise() == PI_INIT_FAILED) {
        return 0; // Devolver 0 si no se pudo inicializar.
    }
    // Inicializar LEDs
    inicializar_leds(pines_positivos, pines_negativos);
    return 1; // Devolver 1 si todo se realizo correctamente.
}

void finalizar_gpio() {
    // Finaliza las LEDs
    finalizar_leds(pines_positivos, pines_negativos);
    // Terminar los pines GPIO
    gpioTerminate();
}


// --------------- MENU(provicional) ---------------

void mostrar_menu(){
    int opcion;
    printf("INGRESE UNA OPCION \n1-Imagen 2-Video 3-Salir");
    scanf("%d",&opcion);
    switch (opcion)
    {
    case 1:
            printf("\nSeleccione la imagen \n1- Carita feliz 2- Opcion2");
            return 1;

            break;
    case 2:
            printf("\nSeleccione la animacion");
            return 2

            break;
    default:
            printf("");
            return 3

            break;
    }
    
    return opcion;
}

// --------------- UTILS ---------------

void sigint_handler(int signal) {
    senal_recibida = signal;
}

void copiar_imagen(int imagen_original[TAMANO][TAMANO], int imagen_copia[TAMANO][TAMANO]) {
    int imagen_copia[TAMANO][TAMANO];
    for (int fila = 0; fila < TAMANO; fila++) {
        for (int columna = 0; columna < TAMANO; columna++) {
            imagen_copia[fila][columna] = imagen_original[fila][columna];
        }
    }
}

void extraer_frame(int frame, int animcion[][TAMANO][TAMANO], int frame_extraido[TAMANO][TAMANO]) {
    int frame_actual[TAMANO][TAMANO];
    for (int i = 0; i < TAMANO; i++) {
        for (int j = 0; j < TAMANO; j++) {
            frame_actual[i][j] = animacion[frame][i][j];
        }
    }
}

// --------------- IMAGENES ---------------

int imagen[TAMANO][TAMANO] = {
        {1, 0, 0, 0, 0, 0, 0, 1},
        {0, 1, 0, 0, 0, 0, 1, 0},
        {0, 0, 1, 0, 0, 1, 0, 0},
        {0, 0, 0, 1, 1, 0, 0, 0},
        {0, 0, 0, 1, 1, 0, 0, 0},
        {0, 0, 1, 0, 0, 1, 0, 0},
        {0, 1, 0, 0, 0, 0, 1, 0},
        {1, 0, 0, 0, 0, 0, 0, 1}
};
int imagen2[TAMANO][TAMANO] = {
        {1, 0, 0, 0, 0, 0, 0, 1},
        {0, 1, 0, 0, 0, 0, 1, 0},
        {0, 0, 1, 0, 0, 1, 0, 0},
        {0, 0, 0, 1, 1, 0, 0, 0},
        {0, 0, 0, 1, 1, 0, 0, 0},
        {0, 0, 1, 0, 0, 1, 0, 0},
        {0, 1, 0, 0, 0, 0, 1, 0},
        {1, 0, 0, 0, 0, 0, 0, 1}
};
int imagen3[TAMANO][TAMANO] = {
        {1, 0, 0, 0, 0, 0, 0, 1},
        {0, 1, 0, 0, 0, 0, 1, 0},
        {0, 0, 1, 0, 0, 1, 0, 0},
        {0, 0, 0, 1, 1, 0, 0, 0},
        {0, 0, 0, 1, 1, 0, 0, 0},
        {0, 0, 1, 0, 0, 1, 0, 0},
        {0, 1, 0, 0, 0, 0, 1, 0},
        {1, 0, 0, 0, 0, 0, 0, 1}
};
//--------------- ANIMACION ---------------
int animacion[][TAMANO][TAMANO] = {
        imagen1,
        imagen2,
        imagen3
    };