CC := gcc
CFLAGS := -std=c99 -Wall -Werror -pedantic
OBJ := main.o

.PHONY: clean

all: app1 app2 app3 app4

app1: main_01.c
	$(CC) main_01.c -o app1.out

app2: main_02.c
	$(CC) main_02.c -o app2.out

app3: main_03.c matrices.c multies.c
	$(CC) matrices.c -o matrices.out
	$(CC) multies.c -o multies.out
	$(CC) main_03.c -o app3.out

app4: main_04.c
	$(CC) main_04.c -o app4.out

clean:
	rm -rf *.o *.out
