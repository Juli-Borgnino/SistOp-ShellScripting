#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "command.h"
#include "execute.h"
#include "parser.h"
#include "parsing.h"
#include "builtin.h"
#include "obfuscated.h"

#define MAX_DIR_PATH 1024

#define RESET_COLOR    "\x1b[0m"
#define MAGENTA_T      "\x1b[35m"
#define CYAN_T         "\x1b[36m"



static void show_prompt(void) {
    char actual_dir_path[MAX_DIR_PATH];
    getcwd(actual_dir_path, MAX_DIR_PATH);
    printf (CYAN_T "%s: " MAGENTA_T "Lovelace> " RESET_COLOR ,actual_dir_path);
    fflush (stdout);
}


int main(int argc, char *argv[]) {
    pipeline pipe;
    Parser input;
    bool quit = false;

    input = parser_new(stdin);
    while (!quit) {
        ping_pong_loop("GarnetLlama");
        show_prompt();
        pipe = parse_pipeline(input);

        /* Do I have to exit after executing? */
        quit = parser_at_eof(input); 
        
        if(pipe != NULL) {
            execute_pipeline(pipe);
            pipe = pipeline_destroy(pipe); 
        }
        
    }
    printf("\n"); 
    parser_destroy(input); input = NULL;
    return EXIT_SUCCESS;
}