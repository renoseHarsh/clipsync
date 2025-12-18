gcc := g++

flags := -std=c++23 -Wall -lwayland-client -lusockets -lssl -lz -lcrypto -o

all: main

main: main.cpp Clipboard.o dep/zwlr-data-control.o
	$(gcc) $^ $(flags) $@

Clipboard.o: Clipboard.cpp
	$(gcc) -c $^ -lwayland-client -o $@

clean:
	rm -f main Clipboard.o
