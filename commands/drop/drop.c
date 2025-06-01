#include "drop.h"
#include <stdio.h>
#include <string.h>
#include "../../db/schemas/schema_utils.h"
#include "../../db/schemas/schema.h"

void handle_drop(char** tokens){
    const char* table_name = tokens[1];
    if (!table_name) {
        return;
    }
    drop_a_table(table_name);
}