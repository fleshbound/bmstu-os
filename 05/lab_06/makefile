.PHONY : all clean

CC := gcc
CFLAGS := -std=c99 -I$(INC_DIR) -Wall -Wpedantic -Wextra -Wfloat-equal -Wfloat-conversion -Wvla -pthread
LD_FLAGS := `pkg-config --cflags --libs check`

all: 1.out 2.out

1.out: main_01.o apue.o
	$(CC) $(CFLAGS) -o $@ $^ -lm $(LD_FLAGS)

2.out: main_02.o apue.o
	$(CC) $(CFLAGS) -o $@ $^ -lm $(LD_FLAGS)

apue.o: apue.c
	$(CC) $(CFLAGS) -c $< -o $@

main_01.o: main_01.c
	$(CC) $(CFLAGS) -c $< -o $@

main_02.o: main_02.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.o *.out *.exe
