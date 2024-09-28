#include"kernel/types.h"
#include"kernel/stat.h"
#include"user/user.h"

int isAMultipleOf(int big, int small){
    return big % small == 0;
}

int seive(int* oldfd)  // writes back
{  // this seives s;

    close(oldfd[1]); // dont write to old pipe
    int s;  // what this func is seiving
    int newp[2];

    int flag = read(oldfd[0], &s, 4);
    if (flag == 0) {
        exit(0);    
    }

    pipe(newp);

    int pid = fork();
    if (pid > 0){
        close(newp[0]); // never read from new pipe
        // print prime
        printf("prime %d\n", s);
        int received = 0;
        while(read(oldfd[0], &received, 4) != 0){
            if (!isAMultipleOf(received, s)){
                write(newp[1], &received, 4);
            }
        }
        close(newp[1]);
        close(oldfd[0]);
        wait((int*)0);
    } else {
        seive(newp);
    }
    exit(0);
}

int main(int argc, char* argv[])
{
    // the first process feeds the numbers 2 to 35 into the pipeline,
    // for each prime number, create a process that reads from its left neighbor over a pipe to its right neighbor.

    int p[2]; // p[0] as read fd, p[1] as write fd

    pipe(p);
    int pid = fork();
    if(pid > 0){ // parent process
        // close read end
        close(p[0]);
        // feeds number 2 - 35 into the pipe
        for (int i = 2; i <= 35; i++)
        {
            write(p[1], &i, 4);
        }
        close(p[1]);
        wait((int*)0);
        
    } else {
        seive(p);
        // write to its right neighbor all that's not a multiple of i. i here = 2
    }

    exit(0);

}