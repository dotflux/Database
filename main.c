#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "commands/create/create.h"
#include "commands/input/input.h"
#include "commands/select/select.h"
#include "commands/update/update.h"
#include "commands/delete/delete.h"

#define INPUT_BUFFER_SIZE 1024
#define MAX_TOKENS 64

int main(){
    char input[INPUT_BUFFER_SIZE];
    
    while (1){
        printf(">> ");
        if (!fgets(input, INPUT_BUFFER_SIZE, stdin)){
            printf("Error reading input\n");
        }
        input[strcspn(input, "\n")] = '\0';

        // Check for exit
        if (strcmp(input, "exit") == 0) {
            printf("Exiting...\n");
            break;
        }

        // Tokenize
        char* tokens[MAX_TOKENS] = {NULL};
        char* token = strtok(input, " ():,\n");
        int token_count = 0;

        while (token && token_count < MAX_TOKENS - 1) {
            tokens[token_count++] = token;
            token = strtok(NULL, " ():,\n");
        }
        tokens[token_count] = NULL;

        if (tokens[0] == NULL) continue;

        // Commands 
        if (strcmp(tokens[0], "create") == 0) {
            if (token_count <= 1){
                printf("Syntax error: create <tableName> (column:dataType,column2:datatype)\n");
            }
            handle_create(tokens);
        }
        if (strcmp(tokens[0], "input") == 0) {
            if (token_count <= 1){
                printf("Syntax error: input <tableName> (column:value,column2:value)\n");
            }
            handle_input(tokens);
        }
        if (strcmp(tokens[0], "select") == 0) {
            if (token_count <= 1){
                printf("Syntax error: select <tableName> (column=value && column2>value || column3!=value)\n");
            }
            handle_select(tokens);
        }
        if (strcmp(tokens[0], "update") == 0) {
            if (token_count <= 1) {
                printf("Syntax error: update <tableName> (<whereClause>) values (column:value)\n");
            }
            handle_update(tokens);
        }
        if (strcmp(tokens[0], "delete") == 0) {
            if (token_count <= 1) {
                printf("Syntax error: delete <tableName> (<whereClause>)\n");
            }
            handle_delete(tokens);
        }
    };

    return 0;
}