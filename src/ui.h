#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <string.h>

#define STRING_LEN 100
#define MAX_VALUE 60000
#define NEGATIVE_NUMBER -1
#define NUMBER_TOO_BIG -2
#define POLLING_INTERVAL 500000
#define OFFSET 50000


   /**
    * @brief Check input from user.
    *
    * Esta funcion controla que los datos que ingresa el usuario sean del formato correcto. Devuelve un codigo que determina 
    * si el numero es negativo, muy grande o esta en los limites definidos.
    * @param number Es el numero que se esta controlando.
    */
int check_input(int number);
void print_header1(void);
void print_header2(void);
 /**
    * @brief Define el modo de ejecucion del programa a partir de lo ingresado por el usuario: Polling, Interrupcion, Sleep o Incorrecto (El modo ingresado no fue correcto).
    */
int get_mode(void);
   /**
    * @brief Obtiene el numero ingresado por el usuario y llama a la funcion check_input para determinar si el numero esta en los limites definidos.
    *
    * @see check_input(int number)
    */
int get_time(void);
 /**
    * @brief Llama a la funcion read() del driver hasta que el timer finalice.
    *
    * @param fd Es un identificador del file descriptor que se usa para comunicarse con el driver.
    */
int read_polling(int fd);
 /**
    * @brief El proceso se duerme un tiempo y luego llama a la funcion read() del driver.
    * @param fd Es un identificador del file descriptor que se usa para comunicarse con el driver.
    * @param time_to_sleep es el tiempo que se duerme el proceso.
    */
int read_sleep(int fd, int time_to_sleep);

/**
    * @brief El proceso intenta leer el driver y si el timer no esta listo, se duerme esperando una interrupcion.
    * @param fd Es un identificador del file descriptor que se usa para comunicarse con el driver.
    */
int read_interrupcion(int fd);