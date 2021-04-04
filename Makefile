CC = clang
CFLAGS = # -Wall -Wextra -Werror -Wpedantic
LFLAGS = -lm

all: ray

ray: ray.o
	$(CC) -o ray ray.o $(LFLAGS)

ray.o: ray.c
	$(CC) $(CFLAGS) -c ray.c

clean:
	rm -f ray ray.o

format:
	clang-format -i -style=file *.[ch]

scan-build: clean
	scan-build make

