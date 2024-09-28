#include"kernel/types.h"
#include"kernel/stat.h"
#include"user/user.h"
// #include<ctype.h>

int isdigit(char c){
    return c >= '0' && c <= '9';
}

int is_int(const char *str){
    if(str == 0 || *str == '\0'){
        return 0;
    }
    while (*str)
    {
        if(!isdigit(*str)){
            // printf("this char is not a digit: %c\n", *str);
            return 0;
        }
        str++;
    }
    return 1;    
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: Sleep <time>, put in the time for sleep\n");
        exit(1);
    }
    // sleep

    // make sure is an integer
    if (!is_int(argv[1])) {
        printf("Error: %s is not an integer\n", argv[1]);
        exit(1);
    }

    sleep(atoi(argv[1]));
    exit(0);
}