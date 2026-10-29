#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tests/syscall_mock.h"
#include "execute.h"
#include "builtin.h"

#define MAX_ARGUMENT_AMOUNT 257

static void redirection_in(char *redir_in){
    /* create a read only file descriptor to redir_in  */
    int file_desc = open(redir_in, O_RDONLY, S_IRUSR | S_IWUSR);
    if(file_desc == -1){
        perror("open");
        exit(EXIT_FAILURE);
    }
    /* redirection of stdin to file_desc */
    int result = dup2(file_desc, STDIN_FILENO);
    if(result == -1){
        perror("dup2");
        exit(EXIT_FAILURE);
    }
    /* don't need it anymore, so closed it */
    close(file_desc);
}

static void redirection_out(char *redir_out){
    /* create a write only file descriptor to redir_out
     * if it doesn't exist is created*/
    int file_desc = open(redir_out, O_WRONLY | O_CREAT | O_TRUNC , S_IWUSR | S_IRUSR);
    if(file_desc == -1){
        perror("open");
        exit(EXIT_FAILURE);
    }
    /* redirection of stdout to file_desc */
    int result = dup2(file_desc, STDOUT_FILENO);
    if(result == -1){
        perror("dup2");
        exit(EXIT_FAILURE);
    }
    /* don't need it anymore, so closed it */
    close(file_desc);
}

static void execution(scommand command){
    int argumet_amount = scommand_length(command); 
    char *argv[MAX_ARGUMENT_AMOUNT];
    /* fill argv with command's arguments and name */
    for(int i = 0; i < argumet_amount; i++){
        argv[i] = strdup(scommand_front(command));
        scommand_pop_front(command);
    }
    argv[argumet_amount] = NULL; //null-terminated

    execvp(argv[0], argv); //execute
    perror("execvp");
    exit(EXIT_FAILURE);
}

void execute_pipeline(pipeline apipe){
    assert(apipe != NULL);
    if(pipeline_is_empty(apipe)){
        printf("Empty pipeline\n");
    }
    /* internal command */
    else if(builtin_alone(apipe)){
        builtin_run(pipeline_front(apipe));
    }
    /* external command */
    else{
        /* -------execution without pipe (|)------- */
        if(pipeline_length(apipe) == 1){
            int pid = fork(); //create "child" process
            if(pid == -1){
                perror("fork");
                exit(EXIT_FAILURE);
            }
            scommand command = pipeline_front(apipe);
            
            /* -------"child" process------- */
            if(pid == 0){ //command execution
            
                /* -----redirections----- */
                char *redir_in = scommand_get_redir_in(command);
                char *redir_out = scommand_get_redir_out(command); 
                if(redir_in != NULL){
                    redirection_in(redir_in);
                }
                if(redir_out != NULL){
                    redirection_out(redir_out);
                }
                
                 execution(command);//command execution
                }
                
                /* -------"parent" process------- */
            else{ //background and foreground management
                if(pipeline_get_wait(apipe)){
                    wait(NULL);
                } else{
                    /* doesn't need parent to clean up child process
                     * when it has terminated, avoiding zombie processes*/
                    signal(SIGCHLD, SIG_IGN);
                    printf("[pid] %d\n", pid);
                }
            }
        }
        
        /* -------execution with pipe (|)------- */
        
        else if(pipeline_length(apipe) == 2){
        
            int file_desc[2];
            int pipe_res = pipe(file_desc); //create the pipe
            if(pipe_res == -1){
                perror("pipe");
                exit(EXIT_FAILURE);
            }
            
            int pid = fork(); //create "first child"
            if(pid == -1){
                perror("fork");
                exit(EXIT_FAILURE);
            } 
            
            scommand command1 = pipeline_front(apipe);
            
            /* -----"first child" process----- */
            
           if(pid == 0){ //first command execution
               close(file_desc[0]);// don't need it , so closed it
               
               /* -----redirections----- */
               char *redir_in = scommand_get_redir_in(command1);
               if(redir_in != NULL){ //just if is needed
                   redirection_in(redir_in);
               }
               /* redirection of stdout to pipe's write file descriptor */
                int dup_res = dup2(file_desc[1], STDOUT_FILENO);
                if(dup_res == -1){
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
                close(file_desc[1]);//don't need it anymore, so closed it
               
                /* -----first command execution----- */
                execution(command1);
            }
           
            /* ------- "parent" process ------- */
        
            else{
                int pid2 = fork(); //create "second child"
                if(pid2 == -1){
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                
                pipeline_pop_front(apipe); //keeps second command
                scommand command2 = pipeline_front(apipe);
                
                /* -----"second child" process----- */
                if(pid2 == 0){ //second command execution
                    close(file_desc[1]);// don't need it, so closed it
                
                    /* -----Redirections---- */
                    char *redir_out = scommand_get_redir_out(command2);
                    if(redir_out != NULL){ //just if is needed
                        redirection_out(redir_out);
                    }
                    
                    /* redirection of stdin to pipe's read file descriptor */
                    int dup_res = dup2(file_desc[0], STDIN_FILENO);
                    if(dup_res == -1){
                        perror("dup2");
                        exit(EXIT_FAILURE);
                    }
                    close(file_desc[0]);
                
                    /* ----- second command execution----- */
                    execution(command2);
                }     
                /* -------"parent" process------- */
                
                else{ //background and foreground management
                    
                    close(file_desc[0]);//don't need it, so closed it
                    close(file_desc[1]);//don't need it, so closed it
                    if(pipeline_get_wait(apipe)){
                        waitpid(pid,NULL,0);
                        waitpid(pid2,NULL,0);
                    } else{
                        /* doesn't need parent to clean up child process
                         * when it has terminated, avoiding zombie processes*/
                        signal(SIGCHLD, SIG_IGN);
                        printf("[pid] %d\n", pid);
                    }
                }
            }
        }
    }
}