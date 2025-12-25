CC = gcc
CFLAGS = -O2 -std=c23 -Wall -Werror -lsqlite3 -lssl -lcrypto
BIN_DIR = ./bin
SRC_DIR = ./src
OBJ = $(BIN_DIR)/cli.o $(BIN_DIR)/server.o $(BIN_DIR)/database.o $(BIN_DIR)/ssl.o
LOOKUP_OBJ = $(BIN_DIR)/ssl.o

LOOKUP_FLAGS = -O2 -std=c23 -Wall -Werror -lm -lssl -lcrypto

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: server clean

chat-cli: $(SRC_DIR)/main.c $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

lookup: $(SRC_DIR)/lookup.c $(LOOKUP_OBJ)
	$(CC) -o $@ $^ $(LOOKUP_FLAGS)

keygen:
	yes AI | openssl req -x509 -newkey rsa:4096 -keyout ~/.chat-cli/key.pem -out ~/.chat-cli/cert.pem -days 36500 -nodes

clean:
	rm -f ./bin/*
	rm -f chat-cli
	rm -f lookup
	rm -f vgcore.*
