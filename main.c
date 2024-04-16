// --------------- LIBRERIAS ---------------

#include <signal.h> // Biblioteca para manejar señales
#include <stdio.h>  // Biblioteca para entrada/salida
#include <pigpio.h> // Biblioteca para controlar GPIO de Raspberry Pi
// #include <unistd.h> //Testing
// #include <time.h> //Testing

// --------------- GLOBALES ---------------

#define TAMANO 8                                                                // Tamaño matriz de LEDs
#define FOTOGRAMAS_POR_SEGUNDO 30                                               // Velocidad de otoframas por segundo para imagenes
#define TIEMPO_POR_FOTOGRAMA (1.0 / FOTOGRAMAS_POR_SEGUNDO)                     // Tiempo de demora de un frame
#define FOTOGRAMAS_POR_SEGUNDO_ANIMACION 3                                      // Velocidad de fotogramas para animaciones
#define TIEMPO_POR_FOTOGRAMA_ANIMACION (1.0 / FOTOGRAMAS_POR_SEGUNDO_ANIMACION) // Teimpo de demora por imagen de la animacion

const int pines_positivos[TAMANO] = {26, 19, 13, 6, 5, 11, 9, 10}; // Pines Positivos GPIO | Columnas

const int pines_negativos[TAMANO] = {21, 20, 16, 12, 7, 8, 25, 24}; // Pines Negativos GPIO | Filas

volatile sig_atomic_t senal_recibida = 0; // Almacenar la senal recibida de consola

// --------------- DECLARACIONES ---------------
// Declaraciones de todas las funciones para llamarlas de cualquier parte del codigo

int inicializar_gpio();
void finalizar_gpio();
void sub_menu_imagen();
void sub_menu_animacion();
void copiar_imagen();
void extraer_frame();
void renderizar_animacion();
void renderizar_animacion_2();
void renderizar_imagen();
void renderizar_imagen_2();
void mostrar_fotograma();
void testear_y_mostrar_leds();
int interrupcion_consola();
void senal_led_coordinado();
void control_led();
void mostrar_menu();
void leds_en_circular();
void leds_en_x();
void leds_sucesion();
void testear_leds();
void sigint_handler(int signal);

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

    mostrar_menu();

    printf("Finalizando programa...");
    finalizar_gpio(); // Finalizar todos los procesos de las LEDs.

    return 0;
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
void renderizar_animacion_2(double seg_duracion, int animacion[][TAMANO][TAMANO], int frames_animacion)
{
    int frame_actual = 0;                      // Frame actual que se esta mostrando
    int frames_totales = frames_animacion - 1; // Frames totales de la animacion
    double tiempo_inicio = time_time();        // Asignar tiempo actual

    while ((time_time() - tiempo_inicio) < seg_duracion) // Ejecutar mientras el tiempo transcurrido sea menor a la duracion
    {
        if (frame_actual > frames_totales)
        {                     // Si se recorreieron todos los frames de la animacion
            frame_actual = 0; // Se devuelve al primero
        }

        int imagen_actual[TAMANO][TAMANO];                                // Guardado de la imagen a mostrar
        extraer_frame(frame_actual, animacion, imagen_actual);            // Extraer el frame que corresponde de la animacion
        renderizar_imagen(TIEMPO_POR_FOTOGRAMA_ANIMACION, imagen_actual); // Renderizar imagen por el tiempo correspondiente
        frame_actual++;                                                   // Avanzar el contador del frame actual
    }
}

/*
    Funcion: Renderizar un conjunto de imagenes, logrando una animacion.
    Ingreso: Duracion de la animacion, arreglo de la animacion, numero de frames de dicha animacion.
    Salida: NADA.
    Detalles: Muestra un conjunto de arreglos los cuales serian las imagenes, logrando
    visualizar una cantidad de fotogramas en un determinado tiempo dando como resultado
    a una animacion.
    Autores: Adolfo T. | Ayuda: Jeremy R.
*/
void renderizar_animacion(double seg_duracion, int animacion[][TAMANO][TAMANO], int frames_animacion)
{
    int frames_totales = seg_duracion * FOTOGRAMAS_POR_SEGUNDO_ANIMACION; // Cuantos frames se pueden mostrar en ese tiempo
    int frame_contador = 0;                                               // Contador de cuantos frames se han mostrado
    int frame_actual = 0;                                                 // Frame actual que se esta mostrando
    int frames_anim = frames_animacion - 1;                               // Frames totales de la animacion

    while (frame_contador < frames_totales) // Mientras no se hayan mostrado todos los frames se ejecuta
    {
        if (frame_actual > frames_anim)
        {                     // Si se recorreieron todos los frames de la animacion
            frame_actual = 0; // Se devuelve al primero
        }

        int imagen_actual[TAMANO][TAMANO];                                // Guardado de la imagen a mostrar
        extraer_frame(frame_actual, animacion, imagen_actual);            // Extraer el frame que corresponde de la animacion
        renderizar_imagen(TIEMPO_POR_FOTOGRAMA_ANIMACION, imagen_actual); // Renderizar imagen por el tiempo correspondiente
        frame_actual++;                                                   // Avanzar el contador del frame actual
        frame_contador++;                                                 // Avanzar el contador del frame actual
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
void renderizar_imagen(double seg_duracion, int imagen[TAMANO][TAMANO])
{
    int frames_totales = seg_duracion * FOTOGRAMAS_POR_SEGUNDO; // Cantidad de frames posibles en el tiempo de duración
    int frames = 0;                                             // Contador de frames mostrados

    int imagen_actual[TAMANO][TAMANO];    // Guardado de la imagen a mostrar
    copiar_imagen(imagen, imagen_actual); // Clonar la imagen ingresada a una variable local

    while (frames < frames_totales) // Mientras no se hayan mostrado todos los frames
    {
        if (interrupcion_consola() == 1)
        {
            break; // Termina el proceso de renderizado
        } // En caso de que se quiera interrumpir el proceso desde la consola
        mostrar_fotograma(imagen_actual); // Mostrar frame de la imagen
        frames++;                         // Aumentar el contador de los frames mostrados
    }

    printf("Se mostraron %i de %i frames.", frames, frames_totales);
}

/*
    Funcion: Renderizar un arreglo, llamado imagen en este caso.
    Ingreso: Duracion de la imagen en pantalla, arreglo de la imagen.
    Salida: NADA.
    Detalles: Muestra en el display una imagen, es decir un arreglo
    por un determinado tiempo.
    Autores: Adolfo T. | Ayuda: Jeremy R.
*/
void renderizar_imagen_2(double seg_duracion, int imagen[TAMANO][TAMANO])
{
    double tiempo_inicio = time_time(); // Guardar tiempo actual

    int imagen_actual[TAMANO][TAMANO];    // Guardado de la imagen a mostrar
    copiar_imagen(imagen, imagen_actual); // Clonar la imagen ingresada a una variable local

    while (((time_time() - tiempo_inicio) seg_duracion))
    { // Mientras no haya transfucrrido el tiempo ingresado
        if (interrupcion_consola() == 1)
        {
            break; // Termina el proceso de renderizado
        } // En caso de que se quiera interrumpir el proceso desde la consola
        mostrar_fotograma(imagen_actual); // Mostrar frame de la imagen
    }

    double tiempo_transcurrido = (time_time() - tiempo_inicio); // Tiempo transcurrido
    printf("Se mostro la imagen por %d segundos.", tiempo_transcurrido);
}

void mostrar_fotograma(int imagen[TAMANO][TAMANO])
{
    // Calcular el tiempo de espera para cada LED
    double tiempo_espera = TIEMPO_POR_FOTOGRAMA / (TAMANO * TAMANO);

    // Mostrar el fotograma leyendo la imagen
    for (int columna = 0; columna < TAMANO; columna++)
    { // Itera por las columnas
        for (int fila = 0; fila < TAMANO; fila++)
        { // Itera por las filas
            // Obtiene si del arreglo si la led debe de prenserse
            int valor_pixel = imagen[fila][columna];
            if (valor_pixel)
            {                                           // Si el pixel es 1 entonces se prende
                senal_led_coordinado(fila, columna, 1); // Prende la led de la posicion correspondiente
                time_sleep(tiempo_espera);              // Espera el tiempo necesario
                senal_led_coordinado(fila, columna, 0); // Apaga la led de la posicion correspondiente
            }
            else
            {                                           // Esa parte de la imagen debe de estar apagada
                senal_led_coordinado(fila, columna, 0); // Apaga la led
                time_sleep(tiempo_espera);              // Espera el tiemppo necesario
            }
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
            time_sleep(0.05);                       // Esperar 500 milisegundos
            senal_led_coordinado(fila, columna, 0); // Apagar el LED
        }
    }
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
            senal_led_coordinado(fila, columna, 1); // Mostrar la X
            time_sleep(0.05);                       // Tiempo que estara prendido el led
            senal_led_coordinado(fila, columna, 0); // Apagar el LED
        }
    }
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
        // Encender LEDs en la fila superior
        for (int columna = columna_inicio; columna <= columna_fin; columna++)
        {
            senal_led_coordinado(fila_inicio, columna, 1);
            time_sleep(0.05);
            senal_led_coordinado(fila_inicio, columna, 0);
        }
        fila_inicio++;

        // Encender LEDs en la columna derecha
        for (int fila = fila_inicio; fila <= fila_fin; fila++)
        {
            senal_led_coordinado(fila, columna_fin, 1);
            // Esperar un corto periodo de tiempo
            time_sleep(0.05);
            senal_led_coordinado(fila, columna_fin, 0);
        }
        columna_fin--;

        // Encender LEDs en la fila inferior
        for (int columna = columna_fin; columna >= columna_inicio; columna--)
        {
            senal_led_coordinado(fila_fin, columna, 1);
            // Esperar un corto periodo de tiempo
            time_sleep(0.05);
            senal_led_coordinado(fila_fin, columna, 0);
        }
        fila_fin--;

        // Encender LEDs en la columna izquierda
        for (int fila = fila_fin; fila >= fila_inicio; fila--)
        {
            senal_led_coordinado(fila, columna_inicio, 1);
            // Esperar un corto periodo de tiempo
            time_sleep(0.05);
            senal_led_coordinado(fila, columna_inicio, 0);
        }
        columna_inicio++;
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
void inicializar_leds(int pines_positivos[TAMANO], int pines_negativos[TAMANO])
{
    for (int pin = 0; pin < TAMANO; pin++)
    {                                                               // Indice del pin
        gpioSetMode(pines_positivos[pin], PI_OUTPUT);               // Poner pines GPIO positivos en OUTPUT
        gpioSetMode(pines_negativos[pin], PI_OUTPUT);               // Poner pines GPIO positivos en OUTPUT
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
void finalizar_leds(int pines_positivos[TAMANO], int pines_negativos[TAMANO])
{
    for (int pin = 0; pin < TAMANO; pin++)
    {                                                               // Indice del pin
        control_led(pines_positivos[pin], pines_negativos[pin], 0); // Apagar LED
        gpioSetMode(pines_positivos[pin], PI_INPUT);                // Poner pines GPIO positivos en INPUT
        gpioSetMode(pines_negativos[pin], PI_INPUT);                // Poner pines GPIO negativos en INPUT
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
    inicializar_leds(pines_positivos, pines_negativos);
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
    // Finaliza las LEDs
    finalizar_leds(pines_positivos, pines_negativos);
    // Terminar los pines GPIO
    gpioTerminate();
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
    char opcion; // Opcion seleccionada
    while (1)
    { // Muestra el titulo y las opciones
        printf("\n--- Menu Principal ---\n");
        printf("1. Mostrar imagen\n");
        printf("2. Mostrar animacion\n");
        printf("Q. Salir\n");
        printf("Seleccione una opcion: ");
        scanf(" %c", &opcion); // Se utiliza " %c" para ignorar el salto de linea

        switch (opcion)
        { // Seleccion del usuario
        case '1':
            sub_menu_imagen(); // Llama al submenu
            break;
        case '2':
            sub_menu_animacion(); // Opcion para la funcion sub_menu_animacion
            break;
        case 'Q': // Opcion de salir
            printf("Saliendo del programa...\n");
            return; // Sale del programa completamente
        default:
            printf("Opcion no valida. Intente de nuevo.\n");
            break;
        }
    }
}

/*
    Funcion: Sub Menu de Imagenes
    Ingreso: NADA
    Salida: NADA
    Detalles: Muestra un submenu para seleccionar y renderizar una imagen en la matriz de leds.
    Autores: Bernardo C. | Ayuda: Jeremy R. | Mejoras: Adolfo T.
*/
void sub_menu_imagen()
{
    char opcion;
    double duracion_imagen = 1;
    while (1)
    { // Muestra el titulo y las opciones
        printf("\n--- Submenu Imagenes ---\n");
        printf("1. Imagen 1\n");
        printf("2. Imagen 2\n");
        printf("3. Imagen 3\n");
        printf("4. Imagen 4\n");
        printf("B. Retroceder\n");
        printf("Seleccione una imagen o B para retroceder: ");
        scanf(" %c", &opcion); // Lee la seleccion del usuario y la almacena en la variable 'opcion'
        printf("\nTiempo de duracion (segundos): ");
        scanf("%lf", &duracion_imagen);
        while (duracion_imagen < 0)
        {
            printf("Duracion invalida. Ingrese un valor positivo: ");
            scanf("%lf", &duracion_imagen);
        }

        switch (opcion)
        { // Comienza el bloque switch para manejar la seleccion del usuario
        case '1':
            renderizar_imagen(duracion_imagen, seta_orejas_arriba); // Opcion que llama a la funcion
            break;
        case '2':
            renderizar_imagen(duracion_imagen, seta_arriba_2); // Opcion que llama a la funcion
            break;
        case '3':
            renderizar_imagen_2(duracion_imagen, cerdo_raro); // Opcion que llama a la funcion
            break;
        case '4':
            renderizar_imagen_2(duracion_imagen, jefe_ojo); // Opcion que llama a la funcion
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
    Autores: Bernardo C. | Ayuda: Jeremy R. | Mejoras: Adolfo T.
*/
void sub_menu_animacion()
{
    char opcion;
    double duracion_animacion = 1;
    while (1)
    { // Muestra el titulo y las opciones
        printf("\n--- Submenu Animaciones ---\n");
        printf("1. Animacion 1\n");
        printf("2. Animacion 2\n");
        printf("3. Animacion 3\n");
        printf("B. Retroceder\n");
        printf("Seleccione una animacion o B para retroceder: ");
        scanf(" %c", &opcion); // Lee la seleccion del usuario y la almacena en la variable 'opcion'
        printf("\nTiempo de duracion (segundos): ");
        scanf("%lf", &duracion_animacion);
        while (duracion_animacion < 0)
        {
            printf("Duracion invalida. Ingrese un valor positivo: ");
            scanf("%lf", &duracion_animacion);
        }

        switch (opcion)
        {
        case '1':
            renderizar_animacion(duracion_animacion, animacion_1, 4);
            break;
        case '2':
            renderizar_animacion_2(duracion_animacion, animacion_1, 4);
            break;
        case 'B':   // Opcion de retornar
            return; // Salir del submenu
        default:
            printf("Opcion no valida.\n");
            break;
        }
    }
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
    int opcion;
    // Verificar si se ha recibido una señal
    if (senal_recibida)
    {
        // Pedir al usuario que elija una opcion
        printf("Menu de Interrupcion:\nDesea Interrumpir? -> [0] Si, [1] No, Salir: ");
        // Leer la opcion elegida por el usuario
        scanf("%i", &opcion);
        // Validar la opcion ingresada
        while (opcion < 0 || opcion > 1)
        {
            printf("[!] Opcion Invalida.\n[0] Si, [1] No, Salir: ");
            scanf("%i", &opcion);
        }
        return 1; // Retorna 1 si se eligio interrumpir
    }
    return 0; // Retorna 0 si no se eligio interrumpir
}

/*
    Funcion: Copiar una Imagen
    Ingreso: Arreglo a copiar, arreglo donde se copiara.
    Salida: NADA.
    Detalles: Funcion para copiar una imagen de una matriz original a una matriz copia.
    Autores: Adolfo T.
*/
void copiar_imagen(int imagen_original[TAMANO][TAMANO], int imagen_copia[TAMANO][TAMANO])
{
    // Itera sobre todas las filas y columnas de la matriz original
    for (int fila = 0; fila < TAMANO; fila++)
    {
        for (int columna = 0; columna < TAMANO; columna++)
        {
            // Copia el elemento de la matriz original a la matriz copia
            imagen_copia[fila][columna] = imagen_original[fila][columna];
        }
    }
}

/*
    Funcion: Extrae un frame de la animacion
    Ingreso: Frame a extraer, arreglo de la animacion, donde se guardara el frame extraido.
    Salida: NADA.
    Detalles: Extrae del arreglo de la animacion una sola imagen y la guarda en una variable.
    Autores: Adolfo T.
*/
void extraer_frame(int frame, int animacion[][TAMANO][TAMANO], int frame_extraido[TAMANO][TAMANO])
{
    // Matriz temporal para almacenar el frame extraido
    int(*frame_actual)[TAMANO] = animacion[frame];

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

int seta_orejas_arriba[TAMANO][TAMANO] = {
    {1, 0, 1, 0, 0, 1, 0, 1},
    {0, 1, 0, 1, 1, 0, 1, 0},
    {0, 0, 1, 0, 0, 1, 0, 0},
    {0, 0, 1, 1, 1, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {1, 1, 0, 1, 1, 0, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {0, 1, 1, 0, 0, 1, 1, 0}

};

int seta_arriba_2[TAMANO][TAMANO] = {
    {0, 1, 0, 0, 0, 0, 1, 0},
    {0, 1, 1, 0, 0, 1, 1, 0},
    {0, 0, 1, 0, 0, 1, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 1, 1, 1, 1, 0, 0},
    {0, 1, 0, 1, 1, 0, 1, 0},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {0, 1, 1, 0, 0, 1, 1, 0}

};

int cerdo_raro[TAMANO][TAMANO] = {
    {0, 1, 0, 0, 0, 0, 1, 0},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {0, 1, 0, 1, 1, 0, 1, 0},
    {0, 1, 1, 0, 0, 1, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 1, 0, 0, 1, 1, 0},
    {0, 1, 0, 0, 0, 0, 1, 0}

};

int jefe_ojo[TAMANO][TAMANO] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {0, 1, 0, 0, 0, 0, 1, 0},
    {0, 1, 0, 1, 1, 0, 1, 0},
    {0, 1, 0, 0, 0, 0, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 0, 1, 1, 0, 1, 0},
    {0, 0, 1, 0, 0, 1, 1, 0}

};
int testx[TAMANO][TAMANO] = {
    {1, 0, 0, 0, 0, 0, 0, 1},
    {0, 1, 0, 0, 0, 0, 1, 0},
    {0, 0, 1, 0, 0, 1, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 1, 0, 0, 1, 0, 0},
    {0, 1, 0, 0, 0, 0, 1, 0},
    {1, 0, 0, 0, 0, 0, 0, 1}};

//--------------- ANIMACION ---------------

int (*animacion_1[4])[TAMANO][TAMANO] = {
    &seta_orejas_arriba,
    &seta_arriba_2,
    &cerdo_raro,
    &jefe_ojo};