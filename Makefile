CC = gcc
CFLAGS = -Wall -Wextra -g

SRC = main.c commands/create/create.c commands/input/input.c db/schemas/schema_utils.c commands/select/select.c db/schemas/bplustree.c
OBJ = $(SRC:.c=.o)
TARGET = myDb

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
