#define _GNU_SOURCE
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

volatile sig_atomic_t last_signal = 0;

void usage(char *name)
{
	fprintf(stderr, "USAGE: %s n\n", name);
	exit(EXIT_FAILURE);
}

void wait_children()
{
	pid_t pid;
	for (;;) {
		pid = waitpid(0, NULL, WNOHANG);
		if (0 == pid)
			return;
		if (0 >= pid) {
			if (ECHILD == errno)
				return;
			ERR("waitpid:");
		}
	}
}

void close_fd(int r, int w, int nd, int fd[3][2])
{
    printf("%d closing %d ...\n", getpid(), fd[nd][0]);
    if(close(fd[nd][0]) != 0) ERR("close");
    
    printf("%d closing %d ...\n", getpid(), fd[nd][1]);
    if(close(fd[nd][1]) != 0) ERR("close");
    
    printf("%d closing %d ...\n", getpid(), fd[r][1]);
    if(close(fd[r][1]) != 0) ERR("close");

    printf("%d closing %d ...\n", getpid(), fd[w][0]);
    if(close(fd[w][0]) != 0) ERR("close");
}

// void add_10(int dig, char *buf)
// {
//     dig -= 2;
//     while(dig > 0 )
//     {
//         buf[dig]++;
//         if(buf[dig] > '9')
//         {
//             buf[dig] = '0'
//             dig--;
//         }
//         else break;
//     }
// }

int dig_cnt(int num)
{
    if( num >= 1000) return 4;
    if( num >= 100) return 3;
    if( num >= 10) return 2;
    if( num >= 0) return 1;

    if( num <= -1000) return 5;
    if( num <= -100) return 4;
    if( num <= -10) return 3;
    if( num <= -1) return 2;

    return -1;
}

void work(int r, int w)
{
    int num;
    char *buf;
    unsigned char dig;

    srand(getpid());

    if(read(r, buf, dig) < 0)
        ERR("write");
    
    num = atoi(buf);

    printf("%d received %d\n", getpid(), num);

    num += rand()%21 - 10;

    sprintf(buf, "%d", num);

    if(write(w, buf, dig_cnt(num)) < 0)
        ERR("write");
    
}

int main(int argc, char **argv)
{
    int fd[3][2];
    unsigned char dig;
    char *buf;

    if(pipe(fd[0]) != 0) ERR("pipe");
    printf("Parent (%d) created %d ...\n", getpid(), fd[0][0]);
    printf("Parent (%d) created %d ...\n", getpid(), fd[0][1]);

    if(pipe(fd[1]) != 0) ERR("pipe");
    printf("Parent (%d) created %d ...\n", getpid(), fd[1][0]);
    printf("Parent (%d) created %d ...\n", getpid(), fd[1][1]);

    if(pipe(fd[2]) != 0) ERR("pipe");
    printf("Parent (%d) created %d ...\n", getpid(), fd[2][0]);
    printf("Parent (%d) created %d ...\n", getpid(), fd[2][1]);
    
    // if(pipe(fd23) != 0)
	// 	ERR("pipe"); 

    // if(pipe(fd31) != 0)
	// 	ERR("pipe");

    if(fork() == 0) // vertex 1
    {
        close_fd(2, 0, 1, fd);
        
        work(fd[2][0], fd[0][1]);

        exit(EXIT_SUCCESS);
    }

    if(fork() == 0) // vertex 2
    {
        close_fd(0, 1, 2, fd);
        
        work(fd[0][0], fd[1][1]);

        exit(EXIT_SUCCESS);
    }

    close_fd(1, 2, 0, fd);

    dig = 1;
    *buf = '1';
    
    if(write(fd[2][1], buf, dig) < 0)
        ERR("write");
    
    work(fd[1][0], fd[2][1]);

	wait_children();


    return EXIT_SUCCESS;
}