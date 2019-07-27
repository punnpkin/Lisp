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

//Enumeration of Possible Error Types
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM};

//Enumeration of Possible lval Types
enum { LVAL_NUM, LVAL_ERR};

//Declare New lval Struct
typedef struct
{
    int type;
    long num;
    int err;
}lval;

// New number type lval
lval lval_num(long x){
    lval v;
    v.type = LVAL_NUM;
    v.num = x;
    return v;
}

// New error type lval
lval lval_err(int x){
    lval v;
    v.type = LVAL_ERR;
    v.err = x;
    return v;
}

//Print an lval
void lval_print(lval v){
    switch (v.type){
        case LVAL_NUM: printf("%li", v.num);break;
        case LVAL_ERR:
            if(v.err == LERR_DIV_ZERO){
                printf("Error: Division By Zero!");
            }
            if(v.err == LERR_BAD_OP){
                printf("Error: Invalid Operator!");
            }
            if(v.err == LERR_BAD_NUM){
                printf("Error: Invalid Number!");
            }
        break;
    }
}
//Print an "lval" followed by a newline
void lval_println(lval v){lval_print(v);putchar('\n');}

//Use operator string to see which operation to perform
lval eval_op(lval x, char* op, lval y){
    if (x.type == LVAL_ERR){return x;}
    if (y.type == LVAL_ERR){return y;}
    
    if(strcmp(op,"+") == 0){return lval_num(x.num + y.num);}
    if(strcmp(op,"-") == 0){return lval_num(x.num - y.num);}
    if(strcmp(op,"*") == 0){return lval_num(x.num * y.num);}
    if(strcmp(op,"/") == 0){return y.num == 0?lval_err(LERR_DIV_ZERO):lval_num(x.num / y.num);

    }
    return lval_err(LERR_BAD_OP);
}

//Calc
lval eval(mpc_ast_t* t){
    //If tagged as number return it directly.
    if(strstr(t->tag,"number")){
        errno = 0;
        long x = strtol(t->contents, NULL, 10);
        return errno != ERANGE?lval_num(x):lval_err(LERR_BAD_NUM);
    }

    //The operator is always second child.
    char* op = t->children[1]->contents;
    
    //Store the third child in 'x'
    lval x = eval(t->children[2]);

    //Iterate the remaining children and combining.
    int i = 3;
    while (strstr(t->children[i]->tag,"expr"))
    {
        x = eval_op(x, op, eval(t->children[i]));
        i++;
    }
    return x;
}

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
    
    puts("Lisp Version 0.0.0.0.3");
    puts("Press Ctrl+c to Exit\n");

    while (1)
    {

        char* input=readline("lispy>>");

        add_history(input);

        //printf("Now you are %s\n", input);
        
        //Parse the Input
        mpc_result_t r;

        if(mpc_parse("<stdin>", input, Lispy, &r)){
            lval result = eval(r.output);
            lval_println(result);
            mpc_ast_delete(r.output);
        }else{
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    mpc_cleanup(4,Number,Operator,Expr,Lispy);

    return 0;
}
