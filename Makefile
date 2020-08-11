
mandelbrot: main.c
	$(CC) `pkg-config --libs --cflags glfw3 glew` -lm $< -o $@ -Wall -Wextra -pedantic -O3

run: mandelbrot
	./$<

clean:
	rm -f mandelbrot

.PHONY: run clean
