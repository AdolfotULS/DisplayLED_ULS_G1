#include <signal.h>  // Biblioteca para manejar señales
#include <stdio.h>   // Biblioteca para entrada/salida
#include <pigpio.h>  // Biblioteca para controlar GPIO de Raspberry Pi

// ------ Datos Iniciales ------

#define TAMANO 8 // Tamaño matriz de LEDs
// Pines GPIO columnas
const int pines_positivos[TAMANO] = {26, 19, 13, 6, 5, 11, 9, 10};
// Pines GPIO filas
const int pines_negativos[TAMANO] = {21, 20, 16, 12, 7, 8, 25, 24};

// Representación de una imagen en la matriz de LEDs
int imagen[TAMANO][TAMANO] = {
        {1, 0, 0, 0, 0, 0, 0, 1},
        {0, 1, 0, 0, 0, 0, 1, 0},
        {0, 0, 1, 0, 0, 1, 0, 0},
        {0, 0, 0, 1, 1, 0, 0, 0},
        {0, 0, 0, 1, 1, 0, 0, 0},
        {0, 0, 1, 0, 0, 1, 0, 0},
        {0, 1, 0, 0, 0, 0, 1, 0},
        {1, 0, 0, 0, 0, 0, 0, 1}
};//BEnja

// Manejo de señales

volatile sig_atomic_t senal_recibida = 0; // Almacenar la senal recibida

void sigint_handler(int signal) {
    senal_recibida = signal;
}

// ------ Control de LEDs ------

// Inicializa los LEDs configurando los pines como salida
void inicializar_leds(int pines_positivos[TAMANO], int pines_negativos[TAMANO]) {
    for (int pin = 0; pin < TAMANO; pin++) {
        gpioSetMode(pines_positivos[pin], PI_OUTPUT);
        gpioSetMode(pines_negativos[pin], PI_OUTPUT);
        gpioWrite(pines_positivos[pin], PI_LOW);
        gpioWrite(pines_negativos[pin], PI_HIGH);
    }
}

// Finaliza los LEDs configurando los pines como entrada
void finalizar_leds(int pines_positivos[TAMANO], int pines_negativos[TAMANO]) {
    for (int pin = 0; pin < TAMANO; pin++) {
        gpioWrite(pines_positivos[pin], PI_LOW);
        gpioWrite(pines_negativos[pin], PI_HIGH);
        gpioSetMode(pines_positivos[pin], PI_INPUT);
        gpioSetMode(pines_negativos[pin], PI_INPUT);
    }
}

// Controla un LED especifico de la matriz
void senal_led_coordinado(int fila, int columna, int estado) {
    int pin_positivo = pines_positivos[columna];
    int pin_negativo = pines_negativos[fila];
    if (estado) {
        gpioWrite(pin_positivo, PI_HIGH);
        gpioWrite(pin_negativo, PI_LOW);
    } else {
        gpioWrite(pin_positivo, PI_LOW);
        gpioWrite(pin_negativo, PI_HIGH);
    }
}

// Enciende un LED
void prender_led(int pin) {
    gpioWrite(pin, PI_HIGH);
}

// Apaga un LED
void apagar_led(int pin) {
    gpioWrite(pin, PI_LOW);
}

// Hace parpadear un LED durante un tiempo determinado
void parpadear_led(int pin, float milisegundos) {
    prender_led(pin);
    time_sleep(milisegundos / 1000.0); // Convertir milisegundos a segundos
    apagar_led(pin);
}

// Muestra una imagen en la matriz de LEDs y hace parpadear los LEDs
void testear_y_mostrar_leds() {//BENJA
    for (int columna = 0; columna < TAMANO; columna++) {
        for (int fila = 0; fila < TAMANO; fila++) {
            senal_led_coordinado(fila, columna, imagen[fila][columna]); // Mostrar la imagen
            time_sleep(0.1); // Esperar 100 milisegundos
            senal_led_coordinado(fila, columna, 0); // Apagar el LED
        } //BENJA
    }
}

int main() {
    // Inicializa la biblioteca pigpio
    if (gpioInitialise() == PI_INIT_FAILED) {
        printf("ERROR: No se pudo inicializar GPIO.\n");
        return 1;
    }

    // Inicializa los LEDs
    inicializar_leds(pines_positivos, pines_negativos);

    // Registra la función sigint_handler para manejar la senal de interrupcion
    signal(SIGINT, sigint_handler);
    printf("Presionar CTRL-C para salir.\n");

    // Prueba de los LEDs hasta que se reciba una senal de interrupcion
    while (!senal_recibida) {
        testear_y_mostrar_leds();//BENJA
    }

    // Finaliza los LEDs
    finalizar_leds(pines_positivos, pines_negativos);

    // Finaliza la biblioteca pigpio
    gpioTerminate();

    printf("\n");
    return 0;
}
