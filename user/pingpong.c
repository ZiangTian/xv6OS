#include"kernel/types.h"
#include"kernel/stat.h"
#include"user/user.h"

int main(int argc, char* argv[])
{
    // create a pair of file descriptors
    int p[2]; // p[0] as read fd, p[1] as write fd
    char* buf[2];

    const char *ping = "ping";
    const char *pong = "pong";

    pipe(p);
    
    
    // fork a process
    int pid = fork();
    if (pid == 0){ // child function
        read(p[0], buf, 8); // read 8 bytes
        // close read
        close(p[0]);
        printf("%d: received %s\n", getpid(), buf);
        write(p[1], pong, 8); // write 8 bytes
        close(p[1]);
    } else { // parent function, pid is the child pid
        // write a ping
        write(p[1], ping, 8); // write 8 bytes
        // close write
        close(p[1]);
        // wait for the child to finish
        wait(0);
        read(p[0], buf, 8); // read 8 bytes
        // close read
        close(p[0]);
        printf("%d: received %s\n", getpid(), buf);     
    }
    exit(0);    
}