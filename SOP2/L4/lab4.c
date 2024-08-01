#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <time.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

#define BACKLOG 3
#define MAX_CLIENTS 10
#define FS_NUM 10
#define WAFFLE_COUNT 4
volatile sig_atomic_t do_work = 1;

typedef struct {
        pthread_barrier_t *barrier;
} client_thread_arg;

void sigint_handler(int sig)
{
        do_work = 0;
}

int sethandler(void (*f)(int), int sigNo)
{
        struct sigaction act;
        memset(&act, 0, sizeof(struct sigaction));
        act.sa_handler = f;
        if (-1 == sigaction(sigNo, &act, NULL))
                return -1;
        return 0;
}

int make_socket(int domain, int type)
{
        int sock;
        sock = socket(domain, type, 0);
        if (sock < 0)
                ERR("socket");
        return sock;
}

int bind_tcp_socket(uint16_t port)
{
        struct sockaddr_in addr;
        int socketfd, t = 1;
        socketfd = make_socket(PF_INET, SOCK_STREAM);
        memset(&addr, 0x00, sizeof(struct sockaddr_in));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(t)))
                ERR("setsockopt");
        if (bind(socketfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
                ERR("bind");
        if (listen(socketfd, BACKLOG) < 0)
                ERR("listen");
        return socketfd;
}

void usage(char *name)
{
        fprintf(stderr, "USAGE: %s  port\n", name);
}

// struct arguments {
//      int fd;
//      int16_t time;
//      struct sockaddr_in addr;
//      sem_t *semaphore;
// };

int add_new_client(int sfd)
{
        int nfd;
        if ((nfd = TEMP_FAILURE_RETRY(accept(sfd, NULL, NULL))) < 0) {
                if (EAGAIN == errno || EWOULDBLOCK == errno)
                        return -1;
                ERR("accept");
        }
        return nfd;
}

void *client_thread_func(void *arg)
{
    client_thread_arg *targ = (client_thread_arg*) arg;
    //memcpy(&targ, arg, sizeof(targ));

        fprintf(stderr, "client waiting for waffle...\n");

    pthread_barrier_wait(targ->barrier);

    fprintf(stderr, "client after barrier\n");

    return NULL;
}

void *baking_thread_func(void *arg)
{
    // client_thread_arg *targ = (client_thread_arg*) arg;

    srand(getpid());

        int m = rand() % 40 + 21;

    struct timespec baking_time = {m / 10, m % 10}, remaining;

    nanosleep(&baking_time, &remaining);

    return NULL;
}

void doServer(int fd)
{
    int i = 0;
    int clientfd;
    pthread_t client_thread[MAX_CLIENTS];
    pthread_barrier_t barrier;// = pthread_barrier_init(4);
    // pthread_barrierattr_t bar_attr;
    // pthread_barrier_init(&barrier, &bar_attr, 4);
    pthread_barrier_init(&barrier, NULL, 4);
    client_thread_arg thr_arg[MAX_CLIENTS];
    for(int j = 0; j < MAX_CLIENTS; j++)
        thr_arg[j].barrier = &barrier;

    while(1){
        if ((clientfd = add_new_client(fd)) == -1){
            fprintf(stderr, "continue\n");
            continue;
        }
        if (pthread_create(&client_thread[i], NULL, client_thread_func, &thr_arg[i]) != 0)
                        ERR("pthread_create");
        i++;
        fprintf(stderr, "client added\n");
    }
}

int main(int argc, char **argv)
{
        int fd;
        if (argc != 1) {
                usage(argv[0]);
                return EXIT_FAILURE;
        }

        // if (sethandler(SIG_IGN, SIGPIPE))
        //      ERR("Seting SIGPIPE:");
        // if (sethandler(sigint_handler, SIGINT))
        //      ERR("Seting SIGINT:");

    // fd = bind_inet_socket(atoi(argv[1]), SOCK_DGRAM);
    fd = bind_tcp_socket(41234);
        doServer(fd);

    if (TEMP_FAILURE_RETRY(close(fd)) < 0)
                ERR("close");

    fprintf(stderr, "Server has terminated.\n");

    return EXIT_SUCCESS;
}