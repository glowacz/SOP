// #define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#define ERR(source)                                                                                                    \
        (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

const int MAX = 10000;
volatile sig_atomic_t last_signal = 0; //, last_signal = 0;
volatile sig_atomic_t cur_i;
int N;

void usage(char *name)
{
        fprintf(stderr, "USAGE: %s N\nmax N is %d\nn", name, MAX);
        exit(EXIT_FAILURE);
}


void sethandler(void (*f)(int), int sigNo)
{
        struct sigaction act;
        memset(&act, 0, sizeof(struct sigaction));
        act.sa_handler = f;
        if (-1 == sigaction(sigNo, &act, NULL))
                ERR("sigaction");
}

void sig1_handler(int sig)
{
    // printf("hello sh\n");
    printf("han 1, {%d}\n", getpid());
        last_signal = sig;
}

void sig2_handler(int sig)
{
    printf("han 2, {%d}\n", getpid());
        last_signal = sig;
}

void par_handler(int sig)
{
    // printf("par han\n");
    cur_i++;
    if(cur_i >= N)
        cur_i = 0;
    printf("%d\n", cur_i);
}

void child_work(int i)
{
    sethandler(sig1_handler, SIGUSR1);
    sethandler(sig2_handler, SIGUSR2);

    printf("%d\n", getpid());

    // if(!first) return;

    sigset_t mask, oldmask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGUSR1);
        // sigaddset(&mask, SIGUSR2);
        sigprocmask(SIG_BLOCK, &mask, &oldmask);

    // sigsuspend(&oldmask);

    srand(time(NULL) + getpid());

    int count = 0;
    // if(last_signal == SIGUSR1){

    while(1)
    {
        // printf("hello w1 {%d}\n", i);
        // if(last_signal == SIGUSR1){
        sigsuspend(&oldmask);
        if(last_signal == SIGUSR2){
            continue;
        }

        // while(1){
        while(last_signal != SIGUSR2){
            // printf("last signal: %d\n", last_signal);
            // printf("hello w2, cur_i = %d\n", cur_i);
            int ms = rand() % 101 + 100;
            struct timespec t = {0, ms * 1000000};
            nanosleep(&t, NULL);

            printf("{%d}: {%d}\n", getpid(), ++count);
        }

        //last_signal = 0;
    }
    // printf("exit?\n");
}

void parent_work(pid_t *children)
{
    cur_i = 0;
    // printf("hello pw\n");
    sethandler(par_handler, SIGUSR1);
    sethandler(SIG_IGN, SIGUSR2);
    kill(children[0], SIGUSR1);

    sigset_t mask, oldmask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGUSR1);
        // sigaddset(&mask, SIGUSR2);
        sigprocmask(SIG_BLOCK, &mask, &oldmask);

    while(1){
        sigsuspend(&oldmask);
        if(0 == cur_i)
            kill(children[N-1], SIGUSR2);
        else
            kill(children[cur_i - 1], SIGUSR2);

        kill(children[cur_i], SIGUSR1);
    }

}

pid_t* create_children(int N)
{
    // pid_t first_child;
    pid_t *children = malloc(sizeof(pid_t) * N);

    for(int i = 0; i < N; i++)
    {
        pid_t s = fork();
        if(s == 0)
        {
            // if(i == 0)
            //     child_work(1);
            // else
            //     child_work(0);
            child_work(i);
            exit(EXIT_SUCCESS);
        }
        else
        {
            children[i] = s;
        }
        // else if(i == 0)
        //     first_child = s;
    }
    // return first_child;
    return children;
}

int main(int argc, char** argv)
{
    if (argc != 2)
                usage(argv[0]);

    N = atoi(argv[1]);

    if(N < 0 || N > MAX)
        usage(argv[0]);

    // printf("parent PID: %d\n", getpid());
    printf("%d\n", getpid());
    sleep(3);

    pid_t *children = create_children(N);

    parent_work(children);

    while(wait(NULL) > 0 )
        ;
}