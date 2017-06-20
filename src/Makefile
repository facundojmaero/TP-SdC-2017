CC=gcc
FLAGS=-c -Wall -pedantic -O2 -Werror

obj-m += my_timer1.o

all: drivers ui.o

drivers:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

ui.o: ui.c
	$(CC) $(FLAGS) $< -o $@
	$(CC) $@ -o ui

cppcheck:
	@echo
	@echo Realizando verificacion CppCheck
	@echo Los resultados de la misma se encuentran en err.txt
	cppcheck --enable=all . 2> err.txt

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f ui err.txt