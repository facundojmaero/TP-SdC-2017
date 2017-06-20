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

char stringToSend[STRING_LEN];
char recieve[STRING_LEN];

int check_input(int number);
void print_header1(void);
void print_header2(void);

int main(){

	int result, mode;
	char mode_str[STRING_LEN];

	print_header1();

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
			break;

		default :
			printf("Modo incorrecto\n");
	}

	print_header2();


	if( scanf("%s", stringToSend) < 0){
		perror("Error al leer el mensaje\n");
		exit(EXIT_FAILURE);
	}

	result = atoi(stringToSend);

	switch(check_input(result)) {

		case NEGATIVE_NUMBER  :
		printf("Error, negative number entered\n");
		break;

		case NUMBER_TOO_BIG :
		printf("Error, number too big. Sleep limit is %d\n", MAX_VALUE);
		break;

		default :
		printf("Tiempo ingresado: %d ms\n", result);
	}

	sprintf(stringToSend, "%d\n", result);

	int ret;
	int fdEnc = open("/dev/my_timer1", O_RDWR);             // Abro el archivo con permisos de lectoescritura
	if (fdEnc < 0){
		perror("Error al abrir el Encryptor...");
		return errno;
	}

	ret = write(fdEnc, stringToSend, strlen(stringToSend)); // Envio el string al encriptador escribiendo en el archivo dev
	if (ret < 0){
		perror("Error al escribir el archivo del modulo.");
		return errno;
	}

	while(1)
	{
		ret = read(fdEnc, recieve, 100);        // Leo la respuesta del modulo
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

		usleep(500000);
	}

	printf("Timer finalizado, mensaje recibido: %s ms\n", recieve);

	close(fdEnc);
	return 0;
}

void print_header1(void){
	printf("Trabajo Practico de Sistemas de Computacion\n");
	printf("Ingrese el metodo deseado:\n");
	printf(	"1 -> Polling\n"
			"2 -> Sleep\n"
			"3 -> Interrupcion\n");
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