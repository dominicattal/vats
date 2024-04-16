
#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h>
#include <math.h>
#include <time.h>

#define MAX_THREADS 20
#define MAX_N 1000000000

struct thread_args {
    int N;
    long NUM_THREADS;
    int id;
    float *buffer;
};

void *compute(void *vargp) 
{
    struct thread_args *args = vargp;
    for (int i = args->id; i < args->N; i += args->NUM_THREADS)
        args->buffer[i] = sin(i / 3.141592653589 / 2);
}

void test(long N, long NUM_THREADS) {
    float *buffer = malloc(N * sizeof(float));
    pthread_t *thread_ids = malloc(NUM_THREADS * sizeof(pthread_t));
    struct thread_args *args = malloc(NUM_THREADS * sizeof(struct thread_args));
    clock_t t = clock(); 
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].N = N;
        args[i].NUM_THREADS = NUM_THREADS;
        args[i].buffer = buffer;
        args[i].id = i;
        pthread_create(&thread_ids[i], NULL, compute, &args[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(thread_ids[i], NULL);
    t = clock() - t; 
    double time_taken = ((double)t) / CLOCKS_PER_SEC;
    printf("%2d | %10d | %9.7f\n", NUM_THREADS,  N, time_taken);
    free(buffer);
    free(thread_ids);
    free(args);
}
   
int main() 
{
    /* for (int t = 1; t <= MAX_THREADS; t++)
        for (int n = 100; n <= MAX_N; n *= 10)
            test(n, t); */
    test(1000000000, 40);
    return 0;
}