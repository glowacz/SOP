// #include <sys/types.h>
#define _GNU_SOURCE
#include <errno.h>
#include <mqueue.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define ERR(source)                                                                                                    \
        (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

#define MAX_MSG 10

void usage(void)
{
        fprintf(stderr, "USAGE: lab2_dzie\n");
        exit(EXIT_FAILURE);
}

void open_queues(mqd_t d[3], mqd_t *wyd)
{
    if( (d[0] = mq_open("/DANIE1", O_RDWR))  == (mqd_t)-1) ERR("mq_open");
    if( (d[1] = mq_open("/DANIE2", O_RDWR))  == (mqd_t)-1) ERR("mq_open");
    if( (d[2] = mq_open("/DANIE3", O_RDWR))  == (mqd_t)-1) ERR("mq_open");
    if( (*wyd = mq_open("/WYDANIE", O_RDWR))  == (mqd_t)-1) ERR("mq_open");
}

void close_queues(mqd_t d[3], mqd_t *wyd)
{
    if(mq_close(d[0]) == -1) ERR("mq_close");
    if(mq_close(d[1]) == -1) ERR("mq_close");
    if(mq_close(d[2]) == -1) ERR("mq_close");
    if(mq_close(*wyd) == -1) ERR("mq_close");
}

void child_main(mqd_t d[3], mqd_t *wyd)
{
    int q_num;
    pid_t pid = getpid(), rec;
    unsigned prio;
    struct timespec timeout = {5, 0};

    srand(time(NULL) + getpid());
    q_num = rand() % 3;

    if(TEMP_FAILURE_RETRY(mq_send(d[q_num], (const char *)&pid, sizeof(pid_t), 0)) < 0) ERR("mq_send");

    printf("[%d] sent to queue %d\n", pid, q_num);

    for(int i=0; i<5; i++)
    {
        if(TEMP_FAILURE_RETRY(mq_timedreceive(*wyd, (char *)&rec, sizeof(pid_t), &prio, &timeout)) < sizeof(pid_t))
        {
            if(errno == EAGAIN)
                break;
            else
                ERR("mq_receive");
        }
        printf("[%d] got %d\n", pid, rec);
        if(rec == pid)
        {
            printf("MNIAM\n");
            close_queues(d, wyd);
            exit(EXIT_SUCCESS);
        }
        sleep(1);
    }

    printf("Idę do baru mlecznego\n");
    close_queues(d, wyd);
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    printf("hello child\n");
    mqd_t d[3], wyd;

    if(argc != 1)
        usage();

    open_queues(d, &wyd);

    child_main(d, &wyd);

    // close_queues(d, wyd);
}