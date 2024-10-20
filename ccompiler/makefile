CC = gcc
CFLAGS = -Wall -Werror -Wextra -g #-fsanitize=address

buildrun: clean build
	bin/test

build: bin/test

clean:
	rm -rf bin

bin/test: $(addprefix bin/, $(addsuffix .o, $(basename $(wildcard *.c))))
	$(CC) $(CFLAGS) -o $@ $^

bin:
	mkdir bin

bin/%.o: %.c bin
	$(CC) $(CFLAGS) -c -o $@ $<
