#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"

// Win
#ifdef _WIN32
#include <string.h>

static char buffer[2048];
//fake readline function
char* readline(char* prompt){
    fputs(prompt, stdout);
    fgets(buffer,2048,stdin);
    char* cpy=malloc(strlen(buffer)+1);
    strcpy(cpy,buffer);
    cpy[strlen(cpy)-1]='\0';
    return cpy;
}

//fake add_history
void add_history(char* unused){}

#else
#include <editline/readline.h>
#include <editline/history.h>
#endif

int main(int argc, char const *argv[])
{
    //Parsers
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");

    //language
    mpca_lang(MPCA_LANG_DEFAULT,
    "\
    number:/-?[0-9]+/;\
    operator:'+'|'-'|'*'|'/';\
    expr:<number>|'('<operator><expr>+')';\
    lispy:/^/<operator><expr>+/$/;\
    ",
    Number,Operator,Expr,Lispy);
    
    puts("Lisp Version 0.0.0.0.1");
    puts("Press Ctrl+c to Exit\n");

    while (1)
    {

        char* input=readline("lispy>>");

        add_history(input);

        //printf("Now you are %s\n", input);
        
        //Parse the Input
        mpc_result_t r;
        if(mpc_parse("<stdin>", input, Lispy, &r)){
            mpc_ast_print(r.output);
            mpc_ast_delete(r.output);
        }else{
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    return 0;
}
