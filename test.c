#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

char *gettoken(char *line, char *token);
int is_operater(char op);
void infix(char *ptr);
int prior(char op);

int main(){
    char line[8192];
    char *ptr;
    while(fgets(line, 8192, stdin)){
        ptr = line;
        infix(ptr);
        //puts("");
    }
}

char *gettoken(char *line, char *token){
    char *ptr = line;
    char *qtr = token;
    char *lineend = line + strlen(line);
    *token = '\0';
    while(isspace(*ptr)){
        ptr++;
    }
    if(is_operater(*ptr)){
        *qtr++ = *ptr++;
    }else{
        while(!isspace(*ptr)&& !is_operater(*ptr) && ptr < lineend){
            *qtr++ = *ptr++;
        }
    }
    *qtr = '\0';
    if(*token == '\0') return NULL;
    else return ptr;
}

int is_operater(char op){
    switch(op){
        case '+':
        case '-':
        case '*':
        case '/':
        case '(':
        case ')':
            return 1;
        default:
            return 0;
    }
}

void infix(char *ptr){
    char opstack[1024];
    int top = 0;
    char token[256];
    while((ptr = gettoken(ptr, token))){
        if(!is_operater(*token)){
            printf("%s ", token);
            continue;
        }
        //token is operator
        char op = *token;
        switch(op){
            case '(': //add into the stack np matter what
                opstack[++top] = op;
                break;
            case ')': //take the operator out of the stack until meeting'('
                while(opstack[top]!='('){
                    printf("%c ", opstack[top--]);
                }
                top--;
                break;
            default:
                while(prior(op) <= prior(opstack[top]) ){//compare the priority to see where i can add the oprator
                    printf("%c ", opstack[top]);
                    top--;
                }
                opstack[++top] = op;
                break;
        }
    }
    while(top>1){
        printf("%c ", opstack[top--]);
    }
    if(top==1){
        printf("%c\n",opstack[1]);
    }
}

int prior(char op){
    if(op=='+'||op=='-'){
        return 1;
    }
    else if(op=='*'||op=='/'){
        return 2;
    }else{
        return 0;
    }
}