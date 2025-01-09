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

typedef unsigned int UINT;

typedef struct argsEstimation {
	pthread_t tid;
	UINT seed;
	int samplesCount;
} argsEstimation_t;

#define ERR(source)                                                                                                    \
	(fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

void usage(char *name)
{
	fprintf(stderr, "USAGE: %s k n\n", name);
	exit(EXIT_FAILURE);
}

void *fun(void *res)
{
    rand_r(time(NULL) + pthread_self());

    double res;



    res = 3.14;
}

void create_threads(int k, int n)
{
    // pthread_t *restrict threads[k];
    argsEstimation_t *estimations = (argsEstimation_t *)malloc(sizeof(argsEstimation_t) * k);
    int res[k];

    for(int i = 0; i < k; i++)
    {
        phtread_create(threads[i], NULL, fun, res[i]);
    }
}

int main(int argc, char **argv)
{
    if( argc != 3)
        usage("prog17")
    
    int k = atoi(argv(1)), n = atoi(argv(2));

    if( k < 0 || n < 0 )
        usage("prog17");

    create_threads(k, n);

	return EXIT_SUCCESS;
}