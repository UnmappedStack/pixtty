CFLAGS=-Wall -Wextra -Werror -O2 -I include -D_XOPEN_SOURCE=600 -lX11 -g

all: build run

build:
	clang $(CFLAGS) src/* -o pixtty

run:
	./pixtty

clean:
	rm pixtty
