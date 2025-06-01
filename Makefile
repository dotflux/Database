CC = gcc
CFLAGS = -Wall -Wextra -g

SRC = main.c commands/create/create.c commands/input/input.c db/schemas/schema_utils.c commands/select/select.c db/schemas/bplustree.c commands/update/update.c db/schemas/utils/create/create_table.c db/schemas/utils/input/input_table.c db/schemas/utils/read/read_table.c db/schemas/utils/update/update_data.c commands/delete/delete.c db/schemas/utils/delete/delete_data.c commands/insert/insert.c db/schemas/utils/insert/insert_columns.c commands/drop/drop.c db/schemas/utils/drop/drop_table.c
OBJ = $(SRC:.c=.o)
TARGET = myDb

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
