// --------------- LIBRERIAS ---------------

#include <signal.h>  // Biblioteca para manejar señales
#include <stdio.h>   // Biblioteca para entrada/salida
#include <pigpio.h>  // Biblioteca para controlar GPIO de Raspberry Pi

// --------------- GLOBALES ---------------

#define TAMANO 8 // Tamaño matriz de LEDs

const int pines_positivos[TAMANO] = {26, 19, 13, 6, 5, 11, 9, 10}; // Pines GPIO columnas

const int pines_negativos[TAMANO] = {21, 20, 16, 12, 7, 8, 25, 24}; // Pines GPIO filas

volatile sig_atomic_t senal_recibida = 0; // Almacenar la senal recibida

// --------------- DECLARACIONES ---------------



// --------------- MAIN ---------------

void main() {
    signal(SIGINT, sigint_handler);
    if (inicializar_gpio == 0){
        printf("No se ha podido Inicializar GPIO.\n")
        return 1;
    }

    //Menu

    return 0;
}

// --------------- DISPLAY --------------

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

// --------------- CONTROLADOR ---------------

void inicializar_leds(int pines_positivos[TAMANO], int pines_negativos[TAMANO]) {
    for (int pin = 0; pin < TAMANO; pin++) {
        gpioSetMode(pines_positivos[pin], PI_OUTPUT);
        gpioSetMode(pines_negativos[pin], PI_OUTPUT);
        control_led(pines_positivos[pin], pines_negativos[pin], 0);
    }
}

void finalizar_leds(int pines_positivos[TAMANO], int pines_negativos[TAMANO]) {
    for (int pin = 0; pin < TAMANO; pin++) {
        control_led(pines_positivos[pin], pines_negativos[pin], 0);
        gpioSetMode(pines_positivos[pin], PI_INPUT);
        gpioSetMode(pines_negativos[pin], PI_INPUT);
    }
}

void senal_led_coordinado(int fila, int columna, int estado) {
    int pin_positivo = pines_positivos[columna];
    int pin_negativo = pines_negativos[fila];
    control_led(pin_positivo, pin_negativo, estado);
}

void control_led(int pin_positivo, int pin_negativo, int estado) {
    if (estado) {
        gpioWrite(pin_positivo, PI_HIGH);
        gpioWrite(pin_negativo, PI_LOW);
    } else {
        gpioWrite(pin_positivo, PI_LOW);
        gpioWrite(pin_negativo, PI_HIGH);
    }
}

int inicializar_gpio() {
    if (gpioInitialise() == PI_INIT_FAILED) {
        return 0;
    }
    inicializar_leds(pines_positivos, pines_negativos);
    return 1;
}

int finalizar_gpio() {
    finalizar_leds(pines_positivos, pines_negativos);
    gpioTerminate();
}


// --------------- MENU ---------------

void menu(){
    
    int opcion;
    printf("IN");

}



// --------------- UTILS ---------------

void sigint_handler(int signal) {
    senal_recibida = signal;
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
