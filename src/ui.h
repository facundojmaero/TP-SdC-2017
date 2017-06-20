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