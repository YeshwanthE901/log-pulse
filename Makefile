CC = gcc

CFLAGS = -Wall -Wextra -std=c17 -O2 -Iinclude -pthread

TARGET = logpulse

SRC = src/main.c \
      src/parser.c \
      src/hashmap.c \
      src/anomaly.c \
      src/queue.c \
      src/worker.c \
      src/json.c

OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

test_parser: tests/test_parser.c src/parser.c
	$(CC) $(CFLAGS) tests/test_parser.c src/parser.c -o test_parser

test: test_parser
	./test_parser

clean:
	rm -f $(OBJ) $(TARGET) test_parser
