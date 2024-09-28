#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#include "kernel/param.h"

// what's being passed into xargs is: xargs COMMAND arg1 arg2 ... + stdin
int main(int argc, char *argv[])
{
    if (argc <= 2){
        fprintf(2, "Usage: xargs COMMAND arg1 arg2 ...\n");
        exit(1);
    }

    char* cmd = argv[1];
    char* args[MAXARG];
    for (int i = 2; i < argc; i++){
        args[i-2] = argv[i];  // the arguments following xargs
        printf("args[%d]: %s\n", i-2, args[i-2]);
    }

    char buf[512]; // reads stdin into this buffer

    // read from stdin
    // int n;
    while(read(0, buf, sizeof(buf)) > 0){
        // split the input into lines
        for (int i = 0; i < sizeof(buf); i++){
            if (buf[i] == '\n'){
                buf[i] = '\0';  // replace newline with null terminator
                break;
            }
        }

        // we have obtained a line from stdin

        // fork a child process
        int pid = fork();
        if (pid < 0){
            fprintf(2, "fork failed\n");
            exit(1);
        } else if (pid == 0){
            // child process
            // execute the command
            char* args2[MAXARG];
            args2[0] = cmd;
            for (int i = 0; i < argc-2; i++){
                args2[i+1] = args[i];
            }
            args2[argc-1] = buf;
            args2[argc] = 0;  // null terminator
            exec(cmd, args2);
            fprintf(2, "exec failed\n");
            exit(1);
        } else {
            // parent process
            wait(0);
        }
    }

    exit(0);


}