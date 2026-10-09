
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

TARGET = chronodb
TEST_TARGET = test_storage

SRC = src/main.c src/parser.c src/storage.c
TEST_SRC = tests/test_storage.c src/storage.c

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(SRC) include/storage.h
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

$(TEST_TARGET): $(TEST_SRC) include/storage.h
	$(CC) $(CFLAGS) $(TEST_SRC) -o $(TEST_TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET) *.o core core.*

