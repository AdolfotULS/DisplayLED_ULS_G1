// --------------- LIBRERIAS ---------------

#include <pigpio.h> // Biblioteca para controlar GPIO de Raspberry Pi
#include <signal.h> // Biblioteca para manejar señales
#include <stdio.h>  // Biblioteca para entrada/salida

// --------------- GLOBALES ---------------

#define TAMANO 8                                                                // Tamaño matriz de LEDs
#define FOTOGRAMAS_POR_SEGUNDO 65                                               // Velocidad de otoframas por segundo para imagenes
#define TIEMPO_POR_FOTOGRAMA (1.0 / FOTOGRAMAS_POR_SEGUNDO)                     // Tiempo de demora de un frame
#define FOTOGRAMAS_POR_SEGUNDO_ANIMACION 3                                      // Velocidad de fotogramas para animaciones
#define TIEMPO_POR_FOTOGRAMA_ANIMACION (1.0 / FOTOGRAMAS_POR_SEGUNDO_ANIMACION) // Teimpo de demora por imagen de la animacion

const int pines_positivos[TAMANO] = {26, 19, 13, 6, 5, 11, 9, 10}; // Pines Positivos GPIO | Columnas

const int pines_negativos[TAMANO] = {21, 20, 16, 12, 7, 8, 25, 24}; // Pines Negativos GPIO | Filas

volatile sig_atomic_t senal_recibida = 0; // Almacenar la senal recibida de consola

// --------------- DECLARACIONES ---------------

// Declaraciones de todas las funciones para llamarlas de cualquier parte del codigo
// Funciones del menu
void mostrar_menu();
double pedir_tiempo_duracion();
void sub_menu_imagen();
void sub_menu_animacion();

// Funciones del controlador
void testear_leds();
void leds_sucesion();
void leds_en_x();
void leds_en_circular();
void senal_led_coordinado(int fila, int columna, int estado);
void control_led(int pin_positivo, int pin_negativo, int estado);
int inicializar_gpio();
void finalizar_gpio();
void verificar_estado_leds();

// Funciones del display
void renderizar_animacion_tiempo(double seg_duracion, int *animacion[], int frames_animacion);
void renderizar_imagen_tiempo(double seg_duracion, int imagen[TAMANO][TAMANO], int es_animacion);

// Funciones de utilidad
void sigint_handler(int signal);
int interrupcion_consola();
void extraer_frame(int frame, int *animacion[], int frame_extraido[TAMANO][TAMANO]);

// Declaracion de arreglos con las imagenes
int corazon[TAMANO][TAMANO];
int estrella[TAMANO][TAMANO];
int barco_papel[TAMANO][TAMANO];
int flecha_arriba[TAMANO][TAMANO];
int twiter_x[TAMANO][TAMANO];
int testx[TAMANO][TAMANO];

// Declaracion de la animacion
int *animacion_1[5];

// --------------- MAIN ---------------/

int main()
{
    // Inicializacon de los GPIO y tablero de leds.
    if (inicializar_gpio() == 0)
    {
        printf("No se ha podido Inicializar GPIO.\n");
        return 1;
    }
    signal(SIGINT, sigint_handler); // Iniciar el escucha de la consola
    testear_leds();                 // Inicial animacion de prendido del display

    mostrar_menu(); // Llamar al menu

    printf("Finalizando programa...");
    finalizar_gpio(); // Finalizar todos los procesos de las LEDs.

    return 0;
}

// --------------- MENU ---------------

/*
    Funcion: Mostrar Menu
    Ingreso: NADA
    Salida: NADA
    Detalles: Muestra un menu interactivo con opciones para controlar la matriz de leds.
    Autores: Bernardo C. | Ayuda: Jeremy R. | Mejoras: Adolfo T.
*/
void mostrar_menu()
{
    char opcion;
    while (interrupcion_consola() != 1)
    {
        printf("\n--- Menu Principal ---\n1. Mostrar imagen\n2. Mostrar animacion\nQ. Salir\nSeleccione una opcion: ");
        scanf(" %c", &opcion);
        switch (opcion)
        {
        case '1':
            sub_menu_imagen();
            break;
        case '2':
            sub_menu_animacion();
            break;
        case 'Q':
            printf("Saliendo del programa...\n");
            return;
        default:
            printf("Opcion no valida. Intente de nuevo.\n");
            break;
        }
    }
}

/*
    Funcion: Pedir tiempo de duracion
    Ingreso: NADA
    Salida: Duracion en segundos
    Detalles: Solicita al usuario ingresar el tiempo de duracion en segundos para mostrar una imagen o animacion.
    Autores: Adolfo T.
*/
double pedir_tiempo_duracion()
{
    double duracion_imagen = 1;

    printf("\nTiempo de duracion (segundos): ");
    scanf("%lf", &duracion_imagen); // Lee el valor ingresado por el usuario y lo guarda en la variable duracion_imagen

    while (duracion_imagen < 0) // Mientras el valor ingresado sea valido
    {
        printf("Duracion invalida. Ingrese un valor positivo: ");
        scanf("%lf", &duracion_imagen); // Lee nuevamente el valor ingresado por el usuario y lo guarda en la variable duracion_imagen
    }

    return duracion_imagen; // Retorna duración ingresada por el usuario
}

/*
    Funcion: Sub Menu de Imagenes
    Ingreso: NADA
    Salida: NADA
    Detalles: Muestra un submenu para seleccionar y renderizar una imagen en la matriz de leds.
    Autores: Bernardo C. | Ayuda: Jeremy R. | Mejoras: Adolfo T y Ignacia M.
*/
void sub_menu_imagen()
{
    char opcion;
    while (interrupcion_consola() != 1)
    {                                           // Muestra el titulo y las opciones
        printf("\n--- Submenu Imagenes ---\n"); // Nombre de las imagenes - Igancia M
        printf("1. Corazon\n");
        printf("2. Estrella \n");
        printf("3. Barco \n");
        printf("4. Flecha\n");
        printf("5. Letra X \n");
        printf("B. Retroceder\n");
        printf("Seleccione una imagen o B para retroceder: ");
        scanf(" %c", &opcion); // Lee la seleccion del usuario y la almacena en la variable 'opcion'

        switch (opcion)
        { // Comienza el bloque switch para manejar la seleccion del usuario
        case '1':
            renderizar_imagen_tiempo(pedir_tiempo_duracion(), corazon, 0); // Opcion que llama a la funcion
            break;
        case '2':
            renderizar_imagen_tiempo(pedir_tiempo_duracion(), estrella, 0); // Opcion que llama a la funcion
            break;
        case '3':
            renderizar_imagen_tiempo(pedir_tiempo_duracion(), barco_papel, 0); // Opcion que llama a la funcion
            break;
        case '4':
            renderizar_imagen_tiempo(pedir_tiempo_duracion(), flecha_arriba, 0); // Opcion que llama a la funcion
            break;
        case '5':
            renderizar_imagen_tiempo(pedir_tiempo_duracion(), twiter_x, 0); // Opcion que llama a la funcion  Nueva imagen - Ignacia M
            break;
        case 'B':   // Opcion de retornar
            return; // Retorna al menu principal
        default:
            printf("Opcion no valida.\n");
            break;
        }
    }
}

/*
    Funcion: Sub menu de animaciones
    Ingreso: NADA
    Salida: NADA
    Detalles: Muestra un submenu para seleccionar y renderizar una animacion en la matriz de leds.
    Autores: Bernardo C. | Ayuda: Jeremy R. | Mejoras: Adolfo T y Ignacia M.
*/
void sub_menu_animacion()
{
    char opcion;
    while (interrupcion_consola() != 1)
    { // Muestra el titulo y las opciones
        printf("\n--- Submenu Animaciones ---\n");
        printf("1. Animacion 1\n");
        printf("2. Animacion 2\n");
        printf("B. Retroceder\n");
        printf("Seleccione una animacion o B para retroceder: ");
        scanf(" %c", &opcion); // Lee la seleccion del usuario y la almacena en la variable 'opcion'

        switch (opcion)
        {
        case '1':
            renderizar_animacion_tiempo(pedir_tiempo_duracion(), animacion_1, 5);
            break;
        case '2':
            renderizar_animacion_tiempo(pedir_tiempo_duracion(), animacion_1, 5);
            break;
        case 'B':   // Opcion de retornar
            return; // Salir del submenu
        default:
            printf("Opcion no valida.\n");
            break;
        }
    }
}

// --------------- CONTROLADOR ---------------

/*
    Funcion: Probar los leds.
    Ingreso: NADA.
    Salida: NADA.
    Detalles: Llama las 3 funciones para testear leds
    Autores: Benjamin M.
    */
void testear_leds()
{
    // Llamar a la funcion para probar los LEDs en sucesion
    leds_sucesion();
    // Llamar a la funcion para probar los LEDs en forma de X
    leds_en_x();
    // Llamar a la funcion para probar los LEDs en circular
    leds_en_circular();
}

/*
    Funcion: Probar los leds.
    Ingreso: NADA.
    Salida: NADA.
    Detalles: Prende todos los leds en una sucesion lenta
    Autores: Benjamin M.
*/
void leds_sucesion()
{
    for (int columna = 0; columna < TAMANO; columna++)
    {
        for (int fila = 0; fila < TAMANO; fila++)
        {
            senal_led_coordinado(fila, columna, 1); // Encender el LED
            time_sleep(0.02);                       // Esperar
            senal_led_coordinado(fila, columna, 0); // Apagar el LED
        }
    }
    verificar_estado_leds();
}

/*
    Funcion: Probar los leds.
    Ingreso: NADA.
    Salida: NADA.
    Detalles: Prende los leds en X
    Autores: Benjamin M.
*/
void leds_en_x()
{
    for (int columna = 0; columna < TAMANO; columna++)
    {
        for (int fila = 0; fila < TAMANO; fila++)
        {
            int valor_pixel = testx[fila][columna];
            if (valor_pixel == 1)
            {
                senal_led_coordinado(fila, columna, 1); // Mostrar la X
                time_sleep(0.05);                       // Tiempo que estara prendido el led
            }
            senal_led_coordinado(fila, columna, 0); // Apagar el LED
        }
    }
    verificar_estado_leds();
}

/*
    Funcion: Probar los leds.
    Ingreso: NADA.
    Salida: NADA.
    Detalles: Prende los leds en sucesion en circulos
    Autores: Benjamin M.
*/
void leds_en_circular()
{
    int leds[TAMANO][TAMANO] = {0}; // Inicializar el arreglo de LEDs apagados
    int fila_inicio = 0;
    int fila_fin = TAMANO - 1;
    int columna_inicio = 0;
    int columna_fin = TAMANO - 1;

    while (fila_inicio <= fila_fin && columna_inicio <= columna_fin)
    {
        // Prende el led en columna positiva
        for (int columna = columna_inicio; columna <= columna_fin; columna++)
        {
            senal_led_coordinado(fila_inicio, columna, 1);
            time_sleep(0.05);
            senal_led_coordinado(fila_inicio, columna, 0);
        }
        fila_inicio++;

        // Prende el led en fila positiva
        for (int fila = fila_inicio; fila <= fila_fin; fila++)
        {
            senal_led_coordinado(fila, columna_fin, 1);
            time_sleep(0.05);
            senal_led_coordinado(fila, columna_fin, 0);
        }
        columna_fin--;

        // Prende el led en columna negativa
        for (int columna = columna_fin; columna >= columna_inicio; columna--)
        {
            senal_led_coordinado(fila_fin, columna, 1);
            time_sleep(0.05);
            senal_led_coordinado(fila_fin, columna, 0);
        }
        fila_fin--;

        // Prende el led en fila negativa
        for (int fila = fila_fin; fila >= fila_inicio; fila--)
        {
            senal_led_coordinado(fila, columna_inicio, 1);
            time_sleep(0.05);
            senal_led_coordinado(fila, columna_inicio, 0);
        }
        columna_inicio++;
    }
    verificar_estado_leds();
}

/*
    Funcion: Prender y apagar LEDs con respecto a su posicion en dos dimensiones.
    Ingreso: Fila y columna de la posicion, estado de la led.
    Salida: NADA.
    Detalles: Facilita el prendido de apagado de las leds segun una posicion
    virtual de las leds en un arreglo de dos dimensiones.
    Autores: Benjamin M. | Mejoras: Adolfo T.
*/
void senal_led_coordinado(int fila, int columna, int estado)
{
    int pin_positivo = pines_positivos[columna];     // Obtener el valor del pin positivo en la posicion
    int pin_negativo = pines_negativos[fila];        // Obtener el valor del pin negativo en la posicion
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
void control_led(int pin_positivo, int pin_negativo, int estado)
{
    if (estado)
    {                                     // Si el estado es 1 entonces se quiere pender el led.
        gpioWrite(pin_positivo, PI_HIGH); // Enviar Señal Positiva al pin positivo
        gpioWrite(pin_negativo, PI_LOW);  // Enviar señal negativa al pin negativo
    }
    else
    {                                     // El estado no es 1 entonces apagara el led
        gpioWrite(pin_positivo, PI_LOW);  // Enviar Señal negativa al pin positivo
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
int inicializar_gpio()
{
    // Inicializar GPIO
    if (gpioInitialise() == PI_INIT_FAILED)
    {
        return 0; // Devolver 0 si no se pudo inicializar.
    }
    // Inicializar LEDs
    for (int pin = 0; pin < TAMANO; pin++)
    {                                                               // Indice del pin
        gpioSetMode(pines_positivos[pin], PI_OUTPUT);               // Poner pines GPIO positivos en OUTPUT
        gpioSetMode(pines_negativos[pin], PI_OUTPUT);               // Poner pines GPIO positivos en OUTPUT
        control_led(pines_positivos[pin], pines_negativos[pin], 0); // Apagar LED
    }
    return 1; // Devolver 1 si todo se realizo correctamente.
}

/*
    Funcion: Finalizar GPIO y LEDs
    Ingreso: NADA
    Salida: NADA
    Detalles: Tiene como objetivo finalizar a gpio y las
    leds.
    Autores: Adolfo T. & Benjamin M.
*/
void finalizar_gpio()
{
    // Finalizar LEDs
    for (int pin = 0; pin < TAMANO; pin++)
    {                                                               // Indice del pin
        control_led(pines_positivos[pin], pines_negativos[pin], 0); // Apagar LED
        gpioSetMode(pines_positivos[pin], PI_INPUT);                // Poner pines GPIO positivos en INPUT
        gpioSetMode(pines_negativos[pin], PI_INPUT);                // Poner pines GPIO negativos en INPUT
    }
    // Terminar los pines GPIO
    gpioTerminate();
}

/*
    Funcion: Verificar el estado de los LEDs.
    Ingreso: NADA.
    Salida: NADA.
    Detalles: Verifica el estado de los pines GPIO asociados a los LEDs.
    Los configura como salida y apaga el LED correspondiente.
    Autores: Adolfo T.
*/
void verificar_estado_leds()
{
    for (int pin = 0; pin < TAMANO; pin++) // Itera sobre los pines GPIO
    {
        // Verifica si el pin positivo y el pin negativo no estan configurados como salida
        if (gpioGetMode(pin_positivo[pin]) != PI_OUTPUT || gpioGetMode(pin_negativo[pin]) != PI_OUTPUT)
        {
            // Si algún pin no esta configurado como salida, lo configura como salida
            gpioSetMode(pines_positivos[pin], PI_OUTPUT);
            gpioSetMode(pines_negativos[pin], PI_OUTPUT);
        }

        // Apaga el LED correspondiente
        control_led(pines_positivos[pin], pines_negativos[pin], 0);
    }
}

// --------------- DISPLAY --------------

/*
    Funcion: Renderizar un conjunto de imagenes, logrando una animacion.
    Ingreso: Duracion de la animacion, arreglo de la animacion, numero de frames de dicha animacion.
    Salida: NADA.
    Detalles: Muestra un conjunto de arreglos los cuales serian las imagenes, logrando
    visualizar una animacion por un tiempo determinado.
    Autores: Jeremy R. | Mejoras: Adolfo T.
*/
void renderizar_animacion_tiempo(double seg_duracion, int *animacion[], int frames_animacion)
{
    int frame_actual = 0;                      // Frame actual que se esta mostrando
    int frames_totales = frames_animacion - 1; // Frames totales de la animacion
    double tiempo_inicio = time_time();        // Asignar tiempo actual

    while (((time_time() - tiempo_inicio) < seg_duracion) || seg_duracion == 0) // Ejecutar mientras el tiempo transcurrido sea menor a la duracion
    {
        if (interrupcion_consola() == 1)
        {
            break; // Termina el proceso de renderizado
        }          // En caso de que se quiera interrumpir el proceso desde la consola
        if (frame_actual > frames_totales)
        {                     // Si se recorrieron todos los frames de la animacion
            frame_actual = 0; // Se devuelve al primero
        }

        int(*imagen_actual)[TAMANO] = (int(*)[TAMANO])animacion[frame_actual];      // Extraer el frame que corresponde de la animacion
        renderizar_imagen_tiempo(TIEMPO_POR_FOTOGRAMA_ANIMACION, imagen_actual, 1); // Renderizar imagen por el tiempo correspondiente
        frame_actual++;                                                             // Avanzar el contador del frame actual
    }
}

/*
    Funcion: Renderizar un arreglo, llamado imagen en este caso.
    Ingreso: Duracion de la imagen en pantalla, arreglo de la imagen.
    Salida: NADA.
    Detalles: Muestra en el display una imagen, es decir un arreglo
    por un determinada  cantidad de frames basado en el tiempo.
    Autores: Jeremy R. | Mejoras: Adolfo T.
*/
void renderizar_imagen_tiempo(double seg_duracion, int imagen[TAMANO][TAMANO], int es_animacion)
{
    double espera_por_led = TIEMPO_POR_FOTOGRAMA / (TAMANO * TAMANO); // Calcula el tiempo de espera por cada LED
    double tiempo_inicio = time_time();                               // Obtiene el tiempo de inicio

    while ((time_time() - tiempo_inicio) < seg_duracion || seg_duracion == 0) // Mientras no se haya pasado el tiempo de duracion
    {
        if (interrupcion_consola() == 1 && !es_animacion) // En caso de que se quiera interrumpir el proceso desde la consola
        {
            break; // Termina el proceso de renderizado
        }

        for (int columna = 0; columna < TAMANO; columna++) // Itera sobre las columnas de la matriz de LEDs
        {
            for (int fila = 0; fila < TAMANO; fila++) // Itera sobre las filas de la matriz de LEDs
            {
                int valor_pixel = imagen[fila][columna]; // Obtiene el valor del pixel de la imagen

                if (valor_pixel) // Si el valor del pixel es 1 (encendido)
                {
                    senal_led_coordinado(fila, columna, 1); // Enciende el LED correspondiente
                    time_sleep(espera_por_led);             // Espera el tiempo correspondiente
                    senal_led_coordinado(fila, columna, 0); // Apaga el LED correspondiente
                }
                else // Si el valor del pixel es 0 (apagado)
                {
                    senal_led_coordinado(fila, columna, 0); // Apaga el LED correspondiente
                    time_sleep(espera_por_led);             // Espera el tiempo correspondiente
                }
            }
        }
    }

    verificar_estado_leds(); // Verifica el estado de los LEDs despues de mostrar la imagen
}

// --------------- UTILS ---------------

/*
    Funcion:Regristrar ctrl c y terminar el proceso
    Ingreso: Señal recibida de signal.h
    Salida: NADA.
    Detalles:Almacena el numero de senal recibida en la variable global.
    Autores: Adolfo T.
*/
void sigint_handler(int signal)
{
    senal_recibida = signal; // Almacena el numero de senal recibida en la variable global
}

/*
    Funcion:Regristrar Ctrl C y terminar el proceso
    Ingreso: NADA.
    Salida: NADA.
    Detalles: Registra la interaccion con la consola y pregunta si desea
    interrumpir el proceso actual.
    Autores: Adolfo T.
*/
int interrupcion_consola()
{
    int opcion = 0;
    // Verificar si se ha recibido una señal
    if (senal_recibida)
    {
        // Pedir al usuario que elija una opcion
        printf("Menu de Interrupcion:\nDesea Interrumpir? -> [0] No, [1] Si\nOpcion: ");
        // Leer la opcion elegida por el usuario
        scanf("%i", &opcion);
        // Validar la opcion ingresada
        while (opcion < 0 || opcion > 1)
        {
            printf("[!] Opcion Invalida.\n[0] No, [1] Si\nOpcion: ");
            scanf("%i", &opcion);
        }
    }
    senal_recibida = 0; // Restablecemos la senal
    return opcion;      // Retorna 0 si no se eligio interrumpir
}

/*
    Funcion: Extrae un frame de la animacion
    Ingreso: Frame a extraer, arreglo de la animacion, donde se guardara el frame extraido.
    Salida: NADA.    Detalles: Extrae del arreglo de la animacion una sola imagen y la guarda en una variable.
    Autores: Adolfo T.
*/
void extraer_frame(int frame, int *animacion[], int frame_extraido[TAMANO][TAMANO])
{
    // Obtener el puntero al frame actual en la animacion
    int(*frame_actual)[TAMANO] = (int(*)[TAMANO])animacion[frame];

    // Copiar el frame actual al frame extraido
    for (int i = 0; i < TAMANO; i++)
    {
        for (int j = 0; j < TAMANO; j++)
        {
            frame_extraido[i][j] = frame_actual[i][j];
        }
    }
}

// --------------- IMAGENES ---------------

/*
    Detalles: Muestran imagenes de 8x8
    Autores: Todas Ignacia M.
*/

int corazon[TAMANO][TAMANO] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 1, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 0, 0, 0},
    {0, 1, 1, 1, 1, 0, 0, 0},
    {0, 0, 1, 1, 0, 0, 0, 0},
    {0, 0, 0, 1, 0, 0, 0, 0}};

int estrella[TAMANO][TAMANO] = {
    {1, 0, 1, 0, 1, 0, 1, 0},
    {0, 1, 0, 1, 0, 1, 0, 1},
    {1, 0, 1, 0, 1, 0, 1, 0},
    {0, 1, 0, 1, 0, 1, 0, 1},
    {1, 0, 1, 0, 1, 0, 1, 0},
    {0, 1, 0, 1, 0, 1, 0, 1},
    {1, 0, 1, 0, 1, 0, 1, 0},
    {0, 1, 0, 1, 0, 1, 0, 1}};

int barco_papel[TAMANO][TAMANO] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 0, 0, 0, 0, 1, 1},
    {0, 1, 1, 0, 0, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 0, 0},
    {0, 0, 1, 1, 1, 1, 0, 0},
    {1, 1, 0, 0, 0, 0, 1, 1}};

int flecha_arriba[TAMANO][TAMANO] = {
    {0, 0, 0, 1, 0, 0, 0, 0},
    {0, 0, 1, 1, 1, 0, 0, 0},
    {0, 1, 0, 1, 0, 1, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 0, 1, 0, 0, 0, 0},
    {0, 0, 0, 1, 0, 0, 0, 0},
    {0, 0, 0, 1, 0, 0, 0, 0},
    {0, 0, 0, 1, 0, 0, 0, 0}};

int twiter_x[TAMANO][TAMANO] = {
    {1, 0, 0, 0, 0, 0, 0, 1},
    {0, 1, 0, 0, 0, 0, 1, 0},
    {0, 0, 1, 0, 0, 1, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 1, 0, 0, 1, 0, 0},
    {0, 1, 0, 0, 0, 0, 1, 0},
    {1, 0, 0, 0, 0, 0, 0, 1}};

int testx[TAMANO][TAMANO] = { // imagen de inicializacion
    {1, 0, 0, 0, 0, 0, 0, 1},
    {0, 1, 0, 0, 0, 0, 1, 0},
    {0, 0, 1, 0, 0, 1, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 1, 0, 0, 1, 0, 0},
    {0, 1, 0, 0, 0, 0, 1, 0},
    {1, 0, 0, 0, 0, 0, 0, 1}};

//--------------- ANIMACION ---------------

int (*animacion_1[5])[TAMANO][TAMANO] = {
    &corazon,
    &estrella,
    &barco_papel,
    &flecha_arriba,
    &twiter_x};
