#include <math.h>
#include <pthread.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

#define MAX 100

typedef unsigned int UINT;
typedef struct args {
        pthread_t tid;
  UINT seed;
  int *k;
  pthread_mutex_t *mxCells;
  double *a1;
  double *a2;
} args_t;

void usage(char *name);
void ReadArguments(int argc, char **argv, int *n, int *k);
void FillArrays(double *a1, double *a2, int k);
void CreateThreads(args_t *args_arr, int n, int k, pthread_mutex_t *mxCells, double *a1, double *a2);
void *fun(void *voidPtr);
void JoinThreads(args_t *args_arr, int n);

int main(int argc, char **argv)
{
        int n, k;
        ReadArguments(argc, argv, &n, &k);

  double *a1, *a2;

  if (NULL == (a1 = (double *)malloc(sizeof(double) * k))) ERR("Malloc error for array!");
  if (NULL == (a2 = (double *)malloc(sizeof(double) * k))) ERR("Malloc error for array!");

  FillArrays(a1, a2, k);

  args_t *args_arr = (args_t *)malloc(sizeof(args_t) * n);

  pthread_mutex_t mxCells[k];
        for (int i = 0; i < k; i++) {
                if (pthread_mutex_init(&mxCells[i], NULL))
                        ERR("Couldn't initialize mutex!");
        }

  CreateThreads(args_arr, n, k, mxCells, a1, a2);

  JoinThreads(args_arr, n);

  printf("\nin:\n");
  for(int i = 0; i < k; i++)
    printf("%f\n", a1[i]);

  printf("\nout:\n");
  for(int i = 0; i < k; i++)
    printf("%f\n", a2[i]);

  for (int i = 0; i < k; i++) pthread_mutex_destroy(&mxCells[i]);

  free(a1);
  free(a2);
  free(args_arr);
}

void usage(char *name)
{
        fprintf(stderr, "USAGE: %s n k\n", name);
        exit(EXIT_FAILURE);
}

void ReadArguments(int argc, char **argv, int *n, int *k)
{
        if (argc < 3)
        usage("lab3");

    *n = atoi(argv[1]);
    *k = atoi(argv[2]);
    if (*n <= 0) {
        printf("Invalid value for 'n'");
        exit(EXIT_FAILURE);
    }
    if (*k <= 0) {
        printf("Invalid value for 'k'");
        exit(EXIT_FAILURE);
    }
}

void FillArrays(double *a1, double *a2, int k)
{
  srand(time(NULL)); // only 1 thread exitst ATM

  for(int i = 0; i < k; i++)
      a1[i] = rand() % MAX;

  for(int i = 0; i < k; i++)
      a2[i] = -1;
}

void CreateThreads(args_t *args_arr, int n, int k, pthread_mutex_t *mxCells, double *a1, double *a2)
{
    if (args_arr == NULL)
    ERR("Malloc error for estimation arguments!");

  for (int i = 0; i < n; i++)
  {
    args_arr[i].seed = rand();
    args_arr[i].k = &k;
    args_arr[i].mxCells = mxCells;
    args_arr[i].a1 = a1;
    args_arr[i].a2 = a2;
  }

  for (int i = 0; i < n; i++) {
    int err = pthread_create(&(args_arr[i].tid), NULL, fun, &args_arr[i]);
    if (err != 0)
      ERR("Couldn't create thread");
  }
}

void *fun(void *voidPtr)
{
        args_t *args = voidPtr;
  // printf("*");

  int ind = rand_r(&args->seed) % *(args->k);

  pthread_mutex_lock(&args->mxCells[ind]);

  if(args->a2[ind] != -1)
  {
    pthread_mutex_unlock(&args->mxCells[ind]);
    return NULL;
  }

  printf("%d\n", ind);

  args->a2[ind] = log10(args->a1[ind]);

  pthread_mutex_unlock(&args->mxCells[ind]);

        return NULL;
}

void JoinThreads(args_t *args_arr, int n)
{
  for (int i = 0; i < n; i++) {
    int err = pthread_join(args_arr[i].tid, NULL);
    if (err != 0)
      ERR("Can't join with a thread");
  }
}