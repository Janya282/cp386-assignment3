/*
 * pi.c
 *
 * CP386 Assignment 3 - Question 1
 *
 * Multithreaded Monte Carlo estimation of Pi.
 *
 * Usage: ./pi <NUMBER_OF_DARTS> <NUMBER_OF_THREADS>
 *
 * The program splits NUMBER_OF_DARTS equally among NUMBER_OF_THREADS
 * threads. Each thread generates its share of random (x, y) points in
 * the range [-1, 1] and counts how many fall inside the unit circle.
 * Each thread stores its own count in a shared/global array. After all
 * threads finish, the main thread sums the counts and estimates Pi as:
 *
 *      Pi = 4 * (number of points in circle) / (total number of points)
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

/* Structure for passing data to each thread */
typedef struct
{
    long darts_for_this_thread; /* number of darts this thread must throw */
    long hits;                  /* number of darts that landed in the circle */
} parameters;

/*
 * Helper function: returns a random double in the range [0, 1)
 */
double random_double()
{
    return random() / ((double)RAND_MAX + 1);
}

/*
 * Thread routine: throws "darts_for_this_thread" random darts and counts
 * how many land inside the unit circle. The result is stored back into
 * the thread's own parameters structure (hits field), which is visible
 * to the main thread since it is passed by pointer.
 */
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
    /* Validate number of command-line arguments */
    if (argc != 3)
    {
        fprintf(stderr, "usage: pi <integer value for NUMBER_OF_DARTS> <integer value for NUMBER_OF_THREADS>\n");
        exit(1);
    }

    long NUMBER_OF_DARTS = atol(argv[1]);
    int NUMBER_OF_THREADS = atoi(argv[2]);

    /* Validate NUMBER_OF_DARTS */
    if (NUMBER_OF_DARTS < 5000000)
    {
        fprintf(stderr, "The number of darts must be >= 5000000\n");
        exit(1);
    }

    /* Validate NUMBER_OF_THREADS */
    if (NUMBER_OF_THREADS < 2)
    {
        fprintf(stderr, "The number of threads must be >= 2\n");
        exit(1);
    }

    /* Seed the random number generator */
    srandom((unsigned int) time(NULL));

    pthread_t workers[NUMBER_OF_THREADS];
    parameters *thread_data[NUMBER_OF_THREADS];

    /* Split darts as equally as possible among the threads.
     * Any remainder is distributed one-by-one to the first few threads
     * so that the total always sums to exactly NUMBER_OF_DARTS. */
    long base_share = NUMBER_OF_DARTS / NUMBER_OF_THREADS;
    long remainder = NUMBER_OF_DARTS % NUMBER_OF_THREADS;

    /* Create the worker threads */
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

    /* Wait for all threads to finish and accumulate results */
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

    /* Compute and output the estimated value of Pi */
    double pi_estimate = 4.0 * (double) total_in_circle / (double) NUMBER_OF_DARTS;

    printf("Pi = %f\n", pi_estimate);

    return 0;
}
