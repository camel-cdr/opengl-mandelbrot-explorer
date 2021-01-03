.POSIX:

mandelbrot: main.c
	${CC} main.c -o $@ `pkg-config --libs --cflags glfw3 glew` -lm -Wall -Wextra -pedantic -O3

run: mandelbrot
	./$<

clean:
	rm -f mandelbrot

.PHONY: run clean
