#define _GNU_SOURCE
#include <aio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))
#define SHIFT(counter, x) ((counter + x) % BYTES)

#define BYTES 8

void usage(char *);
// void siginthandler(int);
void sethandler(void (*)(int), int);
off_t getfilelength(int);
void fillaiostructs(struct aiocb, char *, int, int);
void suspend(struct aiocb *);
void readdata(struct aiocb *, off_t);
void writedata(struct aiocb *, off_t);
void syncdata(struct aiocb *);
void getindexes(int *, int);
// void cleanup(char **, int, int, int);
void reversebuffer(char *, int);
void processblocks(struct aiocb *, char **, int, int, int);

int main(int argc, char *argv[])
{
        char *a, *b, *out; //, *buffers[4][BYTES];
    char buffers[4][BYTES];
        int fd_a, fd_b, fd_out, n, blocksize, i;
        // struct aiocb aiocbs_a[4], aiocbs_b[4], aiocbs_out[4];
    struct aiocb aiocbs[4];

    if (argc != 5)
                usage(argv[0]);

    a = argv[1];
    b = argv[2];
        out = argv[3];
        n = atoi(argv[4]);
        if (n < 0)
                ERR("atoi");

        // work = 1;
        // sethandler(siginthandler, SIGINT);

        if ((fd_a= TEMP_FAILURE_RETRY(open(a, O_RDONLY))) == -1)
                ERR("Cannot open file");

    if ((fd_b= TEMP_FAILURE_RETRY(open(b, O_RDONLY))) == -1)
                ERR("Cannot open file");

    if ((fd_out = TEMP_FAILURE_RETRY(open(out, O_WRONLY | O_CREAT, 0777))) == -1)
                ERR("Cannot open file");

        // blocksize = (getfilelength(fd) - 1) / n;
    blocksize = 8;

        fprintf(stderr, "Blocksize: %d\n", blocksize);

    // for (i = 0; i < BYTES; i++)
    //     if ((buffer[i] = (char *)calloc(blocksize, sizeof(char))) == NULL)
    //         error("Cannot allocate memory");

    fillaiostructs(aiocbs[0], buffers[0], fd_a, blocksize);
    fillaiostructs(aiocbs[1], buffers[1], fd_b, blocksize);
    fillaiostructs(aiocbs[2], buffers[2], fd_out, blocksize);
    // srand(time(NULL));
    // processblocks(aiocbs, buffer, n, blocksize, 1);

    printf("---\n");
    readdata(&aiocbs[0], 0);
    printf("aaa\n");
    readdata(&aiocbs[1], 0);
    // readdata(&aiocbs[2], 0);

    int sum_a = 0, sum_b = 0;
    for(i=0; i<BYTES; i++)
    {
        sum_a += buffers[0][i] % 64;
        sum_b += buffers[1][i] % 64;
    }

    printf("suma a: %d\n", sum_a);

    printf("suma b: %d\n", sum_b);

        if (TEMP_FAILURE_RETRY(close(fd_a)) == -1)
                ERR("Cannot close file");

    if (TEMP_FAILURE_RETRY(close(fd_b)) == -1)
                ERR("Cannot close file");

    if (TEMP_FAILURE_RETRY(close(fd_out)) == -1)
                ERR("Cannot close file");

        return EXIT_SUCCESS;
}

void usage(char *progname)
{
        fprintf(stderr, "%s workfile blocksize\n", progname);
        fprintf(stderr, "workfile - path to the file to work on\n");
        fprintf(stderr, "n - number of blocks\n");
        fprintf(stderr, "k - number of iterations\n");
        exit(EXIT_FAILURE);
}

// void siginthandler(int sig)
// {
//      work = 0;
// }

void sethandler(void (*f)(int), int sig)
{
        struct sigaction sa;
        memset(&sa, 0x00, sizeof(struct sigaction));
        sa.sa_handler = f;
        if (sigaction(sig, &sa, NULL) == -1)
                ERR("Error setting signal handler");
}

off_t getfilelength(int fd)
{
        struct stat buf;
        if (fstat(fd, &buf) == -1)
                ERR("Cannot fstat file");
        return buf.st_size;
}

void suspend(struct aiocb *aiocbs)
{
        struct aiocb *aiolist[1];
        aiolist[0] = aiocbs;
        // if (!work)
        //      return;
        while (aio_suspend((const struct aiocb *const *)aiolist, 1, NULL) == -1) {
                // if (!work)
                //      return;
                if (errno == EINTR)
                        continue;
                ERR("Suspend error");
        }
        if (aio_error(aiocbs) != 0)
                ERR("Suspend error");
        if (aio_return(aiocbs) == -1)
                ERR("Return error");
}

void fillaiostructs(struct aiocb aiocb, char *buffer, int fd, int blocksize)
{
        // int i;
        // for (i = 0; i < BLOCKS; i++) {
    memset(&aiocb, 0, sizeof(struct aiocb));
    aiocb.aio_fildes = fd;
    aiocb.aio_offset = 0;
    aiocb.aio_nbytes = blocksize;
    aiocb.aio_buf = (void *)buffer;
    aiocb.aio_sigevent.sigev_notify = SIGEV_NONE;
        // }
}

void readdata(struct aiocb *aiocbs, off_t offset)
{
        // if (!work)
        //      return;
        aiocbs->aio_offset = offset;
        if (aio_read(aiocbs) == -1)
                ERR("Cannot read");
}

void writedata(struct aiocb *aiocbs, off_t offset)
{
        // if (!work)
        //      return;
        aiocbs->aio_offset = offset;
        if (aio_write(aiocbs) == -1)
                ERR("Cannot write");
}

void syncdata(struct aiocb *aiocbs)
{
        // if (!work)
        //      return;
        suspend(aiocbs);
        if (aio_fsync(O_SYNC, aiocbs) == -1)
                ERR("Cannot sync\n");
        suspend(aiocbs);
}

void getindexes(int *indexes, int max)
{
        indexes[0] = rand() % max;
        indexes[1] = rand() % (max - 1);
        if (indexes[1] >= indexes[0])
                indexes[1]++;
}

// void cleanup(int fd_a, int fd_b, int fd_out)
// {
//      int i;
//      // if (!work)
//      //      if (aio_cancel(fd, NULL) == -1)
//      //              ERR("Cannot cancel async. I/O operations");

//     // for(int j=0; j<3; j++)
//     // {
//     //     for (i = 0; i < BYTES; i++)
//     //         free(buffers[j][i]);
//     // }

//      if (TEMP_FAILURE_RETRY(fclose(fd_a)) == -1)
//              ERR("Error running fsync");

//     if (TEMP_FAILURE_RETRY(fclose(fd_b)) == -1)
//              ERR("Error running fsync");

//     if (TEMP_FAILURE_RETRY(fclose(fd_out)) == -1)
//              ERR("Error running fsync");
// }

void reversebuffer(char *buffer, int blocksize)
{
        int k;
        char tmp;
        for (k = 0; k < blocksize / 2; k++) {
                tmp = buffer[k];
                buffer[k] = buffer[blocksize - k - 1];
                buffer[blocksize - k - 1] = tmp;
        }
}

void processblocks(struct aiocb *aiocbs, char **buffer, int bcount, int bsize, int iterations)
{
        int curpos, j, index[2];
        iterations--;
        curpos = iterations == 0 ? 1 : 0;
        readdata(&aiocbs[1], bsize);
        suspend(&aiocbs[1]);
        for (j = 0; j < iterations; j++) {
                getindexes(index, bcount);

        writedata(&aiocbs[curpos], index[0] * bsize);
        readdata(&aiocbs[SHIFT(curpos, 2)], index[1] * bsize);

                // reversebuffer(buffer[SHIFT(curpos, 1)], bsize);

        // if (j > 0)
                //      syncdata(&aiocbs[curpos]);
                if (j < iterations - 1)
                        suspend(&aiocbs[SHIFT(curpos, 2)]);
                curpos = SHIFT(curpos, 1);
        }
        // if (iterations == 0)
        //      reversebuffer(buffer[curpos], bsize);
        writedata(&aiocbs[curpos], bsize * (rand() % bcount));
        suspend(&aiocbs[curpos]);
}