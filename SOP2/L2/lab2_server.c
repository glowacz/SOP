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
        fprintf(stderr, "USAGE: lab2_ser\n");
        exit(EXIT_FAILURE);
}

void open_queues(mqd_t d[3], mqd_t *wyd)
{
    struct mq_attr attr;
    attr.mq_maxmsg = MAX_MSG;
    attr.mq_msgsize = sizeof(pid_t);

    if( (d[0] = mq_open("/DANIE1", O_RDWR | O_CREAT, 0666, &attr))  == (mqd_t)-1) ERR("mq_open");
    if( (d[1] = mq_open("/DANIE2", O_RDWR | O_CREAT, 0666, &attr))  == (mqd_t)-1) ERR("mq_open");
    if( (d[2] = mq_open("/DANIE3", O_RDWR | O_CREAT, 0666, &attr))  == (mqd_t)-1) ERR("mq_open");
    if( (*wyd = mq_open("/WYDANIE", O_RDWR | O_CREAT, 0666, &attr))  == (mqd_t)-1) ERR("mq_open");
}

void close_queues(mqd_t d[3], mqd_t *wyd)
{
    if(mq_close(d[0]) == -1) ERR("mq_close");
    if(mq_close(d[1]) == -1) ERR("mq_close");
    if(mq_close(d[2]) == -1) ERR("mq_close");
    if(mq_close(*wyd) == -1) ERR("mq_close");
}

void unlink_queues()
{
    if(mq_unlink("/DANIE1") == -1) ERR("mq_unlink");
    if(mq_unlink("/DANIE2") == -1) ERR("mq_unlink");
    if(mq_unlink("/DANIE3") == -1) ERR("mq_unlink");
    if(mq_unlink("/WYDANIE") == -1) ERR("mq_unlink");
}

void server_main(mqd_t d[3], mqd_t *wyd)
{
    pid_t pid;
    unsigned prio;

    for(;;)
    {
        for(int i=0; i<3; i++)
        {
            if(TEMP_FAILURE_RETRY(mq_receive(d[i], (char *)&pid, sizeof(pid_t), &prio)) < sizeof(pid_t))
            {
                if(errno == EAGAIN)
                    break;
                else
                    ERR("mq_receive");
            }
            if(TEMP_FAILURE_RETRY(mq_send(*wyd, (const char *)&pid, sizeof(pid_t), 0)) < 0) ERR("mq_send");

            sleep(1);
        }
    }
}

int main(int argc, char **argv)
{
    // int n;
    printf("hello ser\n");
    mqd_t d[3], wyd;

    if(argc != 1)
        usage();

    unlink_queues();

    open_queues(d, &wyd);

    server_main(d, &wyd);

    close_queues(d, &wyd);
    unlink_queues();
}