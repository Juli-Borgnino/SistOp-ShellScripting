#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <glib.h>
#include <stdlib.h>
#include <strings.h>

#include "command.h"
#include "strextra.h"


struct scommand_s {
    GList *arguments;
    char *redirIn;       //input redirection
    char *redirOut;     //output redirection
};

scommand scommand_new(void){
    scommand result = NULL;
    result = malloc(sizeof(struct scommand_s));
    if(result==NULL){
        fprintf(stderr, "malloc faild");
        exit(EXIT_FAILURE);
    }
    result->arguments = NULL; 
    result->redirIn = NULL;
    result->redirOut = NULL;
    assert(result != NULL && scommand_is_empty (result) 
                          && scommand_get_redir_in (result) == NULL 
                          && scommand_get_redir_out (result) == NULL);
    return result;
}

scommand scommand_destroy(scommand self){
    assert(self != NULL);
    while(!scommand_is_empty(self)){
        scommand_pop_front(self);
    }
    free(self->redirIn);
    free(self->redirOut);
    g_list_free(self->arguments);
    free(self);
    self = NULL;
    assert(self == NULL);
    return self;
}

void scommand_push_back(scommand self, char * argument){
    assert(self != NULL && argument != NULL);
    self->arguments = g_list_append(self->arguments, argument);
    assert(!scommand_is_empty(self));
}

void scommand_pop_front(scommand self){
    assert(self != NULL && !scommand_is_empty(self));
    GList *first_elem = g_list_first(self->arguments);
    free(first_elem->data);
    self->arguments = g_list_delete_link(self->arguments,first_elem);
}

void scommand_set_redir_in(scommand self, char * filename){
    assert(self != NULL);
    //I free the memory used by redirIn and define redirection
    free(self->redirIn);
    if(filename != NULL){
        self->redirIn = filename;
    } else{
        self->redirIn = NULL;
    }
}

void scommand_set_redir_out(scommand self, char * filename){
    assert(self != NULL);
    //I free the memory used by redirOut and define redirection
    free(self->redirOut);
    if(filename != NULL){
        self->redirOut = filename;
    } else{
        self->redirOut = NULL;
    }
}

bool scommand_is_empty(const scommand self){
    assert(self != NULL);
    return (g_list_length(self->arguments) == 0);
}

unsigned int scommand_length(const scommand self){
    assert(self != NULL);
    return (g_list_length(self->arguments));

}

char * scommand_front(const scommand self){
 assert(self != NULL  && !scommand_is_empty(self));
 return (self->arguments->data); // that data comes from glib.h

}

char * scommand_get_redir_in(const scommand self){
    assert(self != NULL);
    return (self->redirIn);
}

char * scommand_get_redir_out(const scommand self){

    assert(self != NULL);

    return (self->redirOut);

}

char * scommand_to_string(const scommand self){
    assert(self != NULL);
    //if there are no commands I return NULL
    char *res = strdup("");
    if(self->arguments != NULL){
        //number of simple commands
        char *temp = NULL;
        char *comm_data = NULL;
        //concatenate the arguments
        for(GList *command=self->arguments; command!=NULL
                                          ; command=command->next){
            if(command->data!=NULL){
                comm_data = (char*)command->data;
                // Add the comm_data command
                temp = strmerge(res, comm_data); 
                free(res); // Release memory from previous chain
                res = strmerge(temp," "); //add space
                free(temp);
            }
        }
        //I concatenate the redirects with their respective sign (<,>)
        if(self->redirIn != NULL){
            temp = strmerge(res, "< ");
            free(res); // Release the previous chain
            res = strmerge(temp, self->redirIn);
            free(temp);
        }
        if(self->redirOut != NULL){
            temp = strmerge(res, " > ");
            free(res); // Release the previous chain
            res = strmerge(temp, self->redirOut);
            free(temp);
        }
    }
    assert(scommand_is_empty(self) || scommand_get_redir_in(self)==NULL
                                   || scommand_get_redir_out(self)==NULL
                                   || strlen(res)>0);
    return res;
}

struct pipeline_s{
    GList *scmd;      //Command List
    bool wait; 
    unsigned int length;
};

pipeline pipeline_new(void){
 
    pipeline result = malloc(sizeof(struct pipeline_s));
    if(result==NULL){
        fprintf(stderr, "malloc faild");
        exit(EXIT_FAILURE);
    }
    result->scmd      = NULL; 
    result->wait      = true; 
    result->length    = 0;

    assert(result != NULL && pipeline_is_empty(result) && pipeline_get_wait(result));

    return result; 
}


pipeline pipeline_destroy(pipeline self){

    assert(self != NULL);
    while(!pipeline_is_empty(self)){
        pipeline_pop_front(self);
    }
    g_list_free(self->scmd);
    free(self);
    self = NULL;
    return self; 
}

void pipeline_push_back(pipeline self, scommand sc){

   assert(self != NULL && sc != NULL);
   //g_slist_append adds a new element to the end of list
   self->scmd = g_list_append(self->scmd, sc);
   ++(self->length);

}

void pipeline_pop_front(pipeline self){

   assert(self!=NULL && !pipeline_is_empty(self));
   scommand_destroy(g_list_first(self->scmd)->data);
   self->scmd = g_list_delete_link(self->scmd, g_list_first(self->scmd));
   --(self->length);
}

void pipeline_set_wait(pipeline self, const bool w){
   assert(self != NULL);
   self->wait = w;
}
bool pipeline_is_empty(const pipeline self){
	assert(self != NULL);
	return self->length == 0;
}

unsigned int pipeline_length(const pipeline self) {
	assert(self != NULL);
	return (self->length);
}

scommand pipeline_front(const pipeline self){
	 assert(self != NULL && !pipeline_is_empty(self)); 
    return (scommand)self->scmd->data;
}

bool pipeline_get_wait(const pipeline self){
	assert(self != NULL);
    return self->wait;
}

char *pipeline_to_string(const pipeline self) {
    assert(self!=NULL);
    if (self->scmd == NULL) {
        return strdup(""); // Returns an empty string if there are no commands
    }

    GList *node = self->scmd;
    char *result = strdup(""); // empty initial string

    while (node != NULL) {
        //I pass the command to string
        char *command = scommand_to_string(node->data);
        char *temp = strmerge(result, command); // Add the current command
        free(result); // Release the previous chain
        result = temp;

        if (node->next != NULL) {
            temp = strmerge(result, " | "); // Add the `|` symbol
            free(result); // Release the previous chain
            result = temp;
        }
        //I release the command string
        free(command);
        node = node->next;
    }

    // Add `&` to the end if it should not be expected
    if (!self->wait) {
        char *temp2 = strmerge(result, " &");
        free(result);
        result = temp2;
    }

    return result;
}