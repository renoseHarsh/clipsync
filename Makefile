# gcc := g++
#
# flags := -std=c++23 -Wall -lwayland-client -lusockets -lssl -lz -lcrypto -o
#
# all: main
#
# main: main.cpp Clipboard.o dep/wlr-data-control-unstable-v1-protocol.o
# 	$(gcc) $^ $(flags) $@
#
# Clipboard.o: Clipboard.cpp dep/wlr-data-control-unstable-v1-client-protocol.h
# 	$(gcc) -c $< -lwayland-client -o $@
#
# dep/wlr-data-control-unstable-v1-protocol.o: dep/wlr-data-control-unstable-v1-protocol.c
# 	gcc -c $^ -o $@
#
# dep/wlr-data-control-unstable-v1-protocol.c: dep/wlr-data-control-unstable-v1.xml
# 	wayland-scanner private-code dep/wlr-data-control-unstable-v1.xml dep/wlr-data-control-unstable-v1-protocol.c
#
# dep/wlr-data-control-unstable-v1-client-protocol.h: dep/wlr-data-control-unstable-v1.xml
# 	wayland-scanner client-header dep/wlr-data-control-unstable-v1.xml dep/wlr-data-control-unstable-v1-client-protocol.h
#
#
# clean:
# 	rm -f main Clipboard.o
# 	rm -f dep/wlr-data-control-unstable-v1-client-protocol.h dep/wlr-data-control-unstable-v1-protocol.c dep/wlr-data-control-unstable-v1-protocol.o

CXX      := g++
CC       := gcc
CXXFLAGS := -std=c++23 -Wall -Wextra -Idep
LDLIBS   := -lwayland-client -lusockets -lssl -lz -lcrypto
GEN_DIR  := dep
CERT_DIR := misc

SRCS := main.cpp Clipboard.cpp
OBJS := $(SRCS:.cpp=.o) $(GEN_DIR)/wlr-data-control-unstable-v1-protocol.o

all: $(CERT_DIR)/key.pem main

$(CERT_DIR)/key.pem:
	@mkdir -p $(CERT_DIR)
	@echo "Generating self-signed certificates for demo..."
	openssl req -newkey rsa:2048 -nodes -keyout $(CERT_DIR)/key.pem \
		-x509 -days 365 -out $(CERT_DIR)/cert.pem \
		-subj "/C=US/ST=Dev/L=Dev/O=Dev/CN=localhost"

# LINKING
main: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

$(OBJS): $(GEN_DIR)/wlr-data-control-unstable-v1-client-protocol.h

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(GEN_DIR)/%-protocol.c: $(GEN_DIR)/%.xml
	@mkdir -p $(GEN_DIR)
	wayland-scanner private-code $< $@

$(GEN_DIR)/%-client-protocol.h: $(GEN_DIR)/%.xml
	@mkdir -p $(GEN_DIR)
	wayland-scanner client-header $< $@

$(GEN_DIR)/%.o: $(GEN_DIR)/%.c
	$(CC) -c $< -o $@

-include $(SRCS:.cpp=.d)

clean:
	rm -f main $(OBJS) $(SRCS:.cpp=.d)
	rm -f $(GEN_DIR)/*.h $(GEN_DIR)/*.c

.PHONY: all clean
