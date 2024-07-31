CC = gcc
CFLAGS = -Wall -Werror -Wextra -g

buildrun: bin/test
	bin/test

build: bin/test

clean:
	rm -r bin

bin/test: $(addprefix bin/, $(addsuffix .o, $(basename $(wildcard *.c))))
	$(CC) $(CFLAGS) -o $@ $^

bin:
	mkdir bin

bin/%.o: %.c bin
	$(CC) $(CFLAGS) -c -o $@ $<
