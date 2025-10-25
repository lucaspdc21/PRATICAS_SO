CC = gcc
CFLAGS = -Wall -Wextra -pedantic -g -Iinclude -Ilibs
TARGET = escalonador_server
SRCS = src/main.c src/escalonadores.c libs/cJSON.c
LIBS = -lm -lmicrohttpd

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean