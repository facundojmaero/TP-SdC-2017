#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <string.h>

#define STRING_LEN 100
#define MAX_VALUE 1000000000
#define NEGATIVE_NUMBER -1
#define NUMBER_TOO_BIG -2
#define POLLING_INTERVAL 500000
#define OFFSET 50000

int check_input(int number);
void print_header1(void);
void print_header2(void);
int get_mode(void);
int get_time(void);
int read_polling(int fd);
int read_sleep(int fd, int time_to_sleep);

void open_driver(int* fd){
	*fd = open("/dev/my_timer1", O_RDWR);
	if (*fd < 0){
		perror("Error al abrir el driver");
		exit(EXIT_FAILURE);
	}
}

int main(){

	int time_to_sleep, mode, ret;
	char time_to_sleep_str[STRING_LEN];

	int fd;
	open_driver(&fd);

	print_header1();

	mode = get_mode();
	if(mode == -1){
		exit(EXIT_FAILURE);
	}

	print_header2();

	time_to_sleep = get_time();
	if(time_to_sleep == -1){
		exit(EXIT_FAILURE);
	}

	sprintf(time_to_sleep_str, "%d\n", time_to_sleep);

	ret = write(fd, time_to_sleep_str, strlen(time_to_sleep_str));
	if (ret < 0){
		perror("Error al escribir el archivo del modulo.");
		return errno;
	}

	switch(mode) {
		case 1  :
			ret = read_polling(fd);
			break;

		case 2 :
			ret = read_sleep(fd, time_to_sleep);
			break;

		case 3 :
			break;

		default :
			break;
	}

	if(ret != 0){
		printf("Error en lectura de file descriptor\n");
		exit(EXIT_FAILURE);
	}

	close(fd);
	return 0;
}

int read_polling(int fd){
	int ret;
	char recieve[STRING_LEN];

	while(1){
		ret = read(fd, recieve, STRING_LEN);
		if (ret < 0){
			perror("Error al leer la respuesta del modulo.");
			return errno;
		}

		printf(".");
		fflush(stdout);
		if(strcmp(recieve, "Not yet...") != 0){
			printf("\n");
			break;
		}

		usleep(POLLING_INTERVAL);
	}

	printf("Timer finalizado, mensaje recibido: %s", recieve);
	return 0;
}

int read_sleep(int fd, int time_to_sleep){
	int ret;
	char recieve[STRING_LEN];

	usleep(time_to_sleep * 1000 + OFFSET);
	
	ret = read(fd, recieve, STRING_LEN);
	if (ret < 0){
		perror("Error al leer la respuesta del modulo.");
		return errno;
	}

	printf("Timer finalizado, mensaje recibido: %s", recieve);
	return 0;	
}

void print_header1(void){
	printf("Trabajo Practico de Sistemas de Computacion\n");
	printf("Ingrese el metodo deseado:\n");
	printf(	"1 -> Polling\n"
			"2 -> Sleep\n"
			"3 -> Interrupcion\n");
}

int get_mode(void){

	char mode_str[STRING_LEN];
	int mode;
	
	if( scanf("%s", mode_str) < 0){
		perror("Error al leer el mensaje\n");
		exit(EXIT_FAILURE);
	}

	mode = atoi(mode_str);

	switch(mode) {
		case 1  :
			printf("Uso Polling\n");
			break;

		case 2 :
			printf("Uso Sleep\n");
			break;

		case 3 :
			printf("Uso Interrupcion\n");
			printf("To do\n");
			exit(EXIT_SUCCESS);
			break;

		default :
			printf("Modo incorrecto\n");
			return -1;
	}
	return mode;
}

int get_time(void){

	char time_to_sleep_str[STRING_LEN];
	int result;

	if( scanf("%s", time_to_sleep_str) < 0){
		perror("Error al leer el mensaje\n");
		exit(EXIT_FAILURE);
	}

	result = atoi(time_to_sleep_str);

	switch(check_input(result)) {

		case NEGATIVE_NUMBER  :
		printf("Error, negative number entered\n");
		break;

		case NUMBER_TOO_BIG :
		printf("Error, number too big. Sleep limit is %d\n", MAX_VALUE);
		break;

		default :
		printf("Tiempo ingresado: %d ms\n", result);
		return result;
	}
	return -1;
}

void print_header2(void){
	printf("Ingrese el tiempo a setear en el timer [ms]\n");
}

int check_input(int number){
	if (number <= 0)
		return NEGATIVE_NUMBER;
	if (number > MAX_VALUE)
		return NUMBER_TOO_BIG;
	return 0;
}