#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

typedef struct
{
    long darts_for_this_thread;
    long hits;                 
} parameters;

double random_double()
{
    return random() / ((double)RAND_MAX + 1);
}

void *throw_darts(void *param)
{
    parameters *data = (parameters *) param;
    long count_in_circle = 0;
    double x, y;

    for (long i = 0; i < data->darts_for_this_thread; i++)
    {
        x = random_double() * 2.0 - 1.0;
        y = random_double() * 2.0 - 1.0;

        if (sqrt(x * x + y * y) <= 1.0)
        {
            count_in_circle++;
        }
    }

    data->hits = count_in_circle;

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "usage: pi <integer value for NUMBER_OF_DARTS> <integer value for NUMBER_OF_THREADS>\n");
        exit(1);
    }

    long NUMBER_OF_DARTS = atol(argv[1]);
    int NUMBER_OF_THREADS = atoi(argv[2]);

    if (NUMBER_OF_DARTS < 5000000)
    {
        fprintf(stderr, "The number of darts must be >= 5000000\n");
        exit(1);
    }

    if (NUMBER_OF_THREADS < 2)
    {
        fprintf(stderr, "The number of threads must be >= 2\n");
        exit(1);
    }

    srandom((unsigned int) time(NULL));

    pthread_t workers[NUMBER_OF_THREADS];
    parameters *thread_data[NUMBER_OF_THREADS];

    long base_share = NUMBER_OF_DARTS / NUMBER_OF_THREADS;
    long remainder = NUMBER_OF_DARTS % NUMBER_OF_THREADS;

    for (int i = 0; i < NUMBER_OF_THREADS; i++)
    {
        thread_data[i] = (parameters *) malloc(sizeof(parameters));
        if (thread_data[i] == NULL)
        {
            fprintf(stderr, "Error: malloc failed for thread %d data\n", i);
            exit(1);
        }

        thread_data[i]->darts_for_this_thread = base_share + (i < remainder ? 1 : 0);
        thread_data[i]->hits = 0;

        int rc = pthread_create(&workers[i], NULL, throw_darts, (void *) thread_data[i]);
        if (rc != 0)
        {
            fprintf(stderr, "Error: pthread_create failed for thread %d\n", i);
            exit(1);
        }
    }

    long total_in_circle = 0;

    for (int i = 0; i < NUMBER_OF_THREADS; i++)
    {
        int rc = pthread_join(workers[i], NULL);
        if (rc != 0)
        {
            fprintf(stderr, "Error: pthread_join failed for thread %d\n", i);
            exit(1);
        }

        total_in_circle += thread_data[i]->hits;
        free(thread_data[i]);
    }

    double pi_estimate = 4.0 * (double) total_in_circle / (double) NUMBER_OF_DARTS;

    printf("Pi = %f\n", pi_estimate);

    return 0;
}
