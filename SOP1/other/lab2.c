#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define ERR(source)                                                                                                    \
	(fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

volatile sig_atomic_t last_signal, sig_count;

void usage(char *name)
{
	fprintf(stderr, "USAGE: %s n1 n2 n3 ...\n", name);
	// fprintf(stderr, "m - number of 1/1000 milliseconds between signals [1,999], "
	// 		"i.e. one milisecond maximum\n");
	// fprintf(stderr, "p - after p SIGUSR1 send one SIGUSER2  [1,999]\n");
	exit(EXIT_FAILURE);
}

ssize_t bulk_write(int fd, char *buf, size_t count)
{
	ssize_t c;
	ssize_t len = 0;
	do {
		c = TEMP_FAILURE_RETRY(write(fd, buf, count));
		if (c < 0)
			return c;
		buf += c;
		len += c;
		count -= c;
	} while (count > 0);
	return len;
}

void sethandler(void (*f)(int), int sigNo)
{
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = f;
	if (-1 == sigaction(sigNo, &act, NULL))
		ERR("sigaction");
}

void sig_handler(int sig)
{
    last_signal = sig;
    if(sig == SIGUSR1)
        printf("[%d] %d signals received\n", getpid(), ++sig_count);
	// sig_count++;
}

void child_work(char n)
{
    sethandler(sig_handler, SIGUSR1);

    alarm(1);

    srand(time(NULL) + getpid());
    int s = rand()%91 + 10;
    s *= 1024;
    int fd, count = s;

    printf("n = %c, s = %d\n", n, s);
    
    char *buf = malloc(s);

    char name[20];
    sprintf(name, "%d", getpid());
    strcat(name, ".txt");

    fd = open(name, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0777);

    bulk_write(fd, buf, count);

    struct timespec t = { 0, 1000*1000*1000 };
    nanosleep(&t, NULL);
    // sleep(1);

    close(fd);

    printf("exit\n");

    exit(EXIT_SUCCESS);
}

void create_children(int argc, char **argv) {
    for(int i=1; i<argc; i++) {
        pid_t pid;
        if ((pid = fork()) < 0)
            ERR("fork");
        if (0 == pid)
            child_work(argv[i][0]);
    }
}

void parent_work()
{
    // printf("a\n");
    sethandler(SIG_IGN, SIGUSR1);

    // sigset_t mask, oldmask;
	// sigemptyset(&mask);
	// sigaddset(&mask, SIGALRM);
    // sigprocmask(SIG_BLOCK, &mask, &oldmask);

    struct timespec t = { 0, 10*1000*1000 };

    alarm(1);

    while(last_signal != SIGALRM) {
        // printf("b\n");
        nanosleep(&t, NULL);
        kill(0, SIGUSR1);
        // sigsuspend(&oldmask);
        // printf("d\n");
    }

}

int main(int argc, char **argv)
{
    sethandler(sig_handler, SIGALRM);
    
    // sigset_t mask, oldmask;
	// sigemptyset(&mask);
	// sigaddset(&mask, SIGALRM);
    // sigprocmask(SIG_BLOCK, &mask, &oldmask);

    create_children(argc, argv);

    parent_work();

    while(wait(NULL) > 0)
        ;

	return EXIT_SUCCESS;
}