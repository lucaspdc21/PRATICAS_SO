CC = gcc
CFLAGS = -Wall -Wextra -pedantic
TARGET = main
SRCS = main.c escalonadores.c cJSON.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)
