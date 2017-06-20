CC=gcc
FLAGS=-c -Wall -pedantic -O2 -Werror

obj-m += my_timer1.o

all: drivers ui.o

drivers:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

ui.o: ui.c
	$(CC) $(FLAGS) $< -o $@
	$(CC) $@ -o ui


clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean