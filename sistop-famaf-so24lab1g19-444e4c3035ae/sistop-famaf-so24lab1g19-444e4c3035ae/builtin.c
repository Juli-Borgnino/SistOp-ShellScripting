#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "builtin.h"
#include "command.h"
#include "strextra.h"
#include "tests/syscall_mock.h"


#define MAX_PATH_NAME 1024

// List of internal commands
static const char* builtin_command[] = {
    "cd",
    "help",
    "exit",
};

static const unsigned int builtin_size = 3;

bool builtin_is_internal(scommand cmd) {
    assert(cmd != NULL);
    const char *command_name = scommand_front(cmd);
    for (unsigned int i = 0; i < builtin_size; ++i) {
        if (strcmp(command_name, builtin_command[i]) == 0) {
            return true;
        }
    }

    return false;
}


bool builtin_alone(pipeline p){
	assert(p != NULL);
	return (pipeline_length(p) == 1) && builtin_is_internal(pipeline_front(p));
}

void builtin_run(scommand cmd) {
    assert(builtin_is_internal(cmd));

    const char *command_name = scommand_front(cmd);

    if (strcmp(command_name, "cd") == 0) {
        scommand_pop_front(cmd); // Remove "cd"
        char *dir = NULL;
        if (!scommand_is_empty(cmd)) {
            dir = scommand_front(cmd);
        }
        if (dir == NULL) {
            dir = getenv("HOME"); // Default to home directory
        }
        // Only concatenate if it is a relative path
        if (dir[0] != '/') {
            char actual_dir[MAX_PATH_NAME];
            char *temp = strmerge("/", dir);
            dir = strmerge(getcwd(actual_dir, MAX_PATH_NAME), temp);
            free(temp);
        }

        if (chdir(dir) != 0) {
            perror("cd failed");
        }
    } else if (strcmp(command_name, "help") == 0) {
        printf("Lovelace: A simple shell implementation by [Evangelina Alvarez,Rosario Angulo,Julieta Borgino,Sofia Tonda].\n");
        printf("Available internal commands:\n");
        printf("  cd <dir>: Change the current directory to <dir>.\n");
        printf("  help: Show this help message.\n");
        printf("  exit: Exit the shell.\n");
    }
      else if (strcmp(command_name, "exit") == 0) {
        printf("bye! (:\n");
        exit(EXIT_SUCCESS);
    } 
}

