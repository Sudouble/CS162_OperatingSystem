/*

  Performance, fairness, contention in locks

 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

/* Variables shared across threads */

pthread_mutex_t count_lock = PTHREAD_MUTEX_INITIALIZER;
long int global_count = 0;
long int global_limit;

bool watch = 0;

/* Thread work structure */

typedef struct twork {
  pthread_t thread;		/* Thread descriptor */
  int tid;			/* Local id  */
  int count;
} TWork;


void *threadfun(void *tw) {
  TWork *my_tw = (TWork *) tw;
  if (watch)   printf("Start Thread #%d\n", my_tw->tid);
  while (global_count < global_limit) {
    pthread_mutex_lock(&count_lock);
    if (watch) {
      printf("Thread #%d lock\n", my_tw->tid);
    }
    global_count++;
    my_tw->count++;
    pthread_mutex_unlock(&count_lock);
    if (watch) {
      printf("Thread #%d unlock\n", my_tw->tid);
    }
  }
  if (watch) {
    printf("Thread #%d done\n", my_tw->tid);
  }
  pthread_exit(NULL);
}

/* Main thread */

int main (int argc, char *argv[]) {
  int i, rc;
  if (argc != 4) {
    printf("Usage %s n_threads n_counts watch\n", argv[0]);
    return 0;
  }
  if (strcmp(argv[3], "True") == 0) {
    watch = 1;
  } else if (strcmp(argv[3], "False") == 0) {
    watch = 0;
  } else {
    printf("watch must be True or False\n");
    return 0;
  }

  int nthreads = atoi(argv[1]);
  TWork threads[nthreads];
  global_limit = atoi(argv[2]);
  clock_t start, end;
  double elapsed;

  if (watch) {
    printf("%d threads, %ld locks\n", nthreads, global_limit);
  }

  /* Spawn threads - with all waiting for lock to be released */
  pthread_mutex_lock(&count_lock);
  for (i=0; i < nthreads; i++) {
    threads[i].tid = i;
    threads[i].count=0;
    rc = pthread_create(&threads[i].thread, NULL, threadfun, (void *) &threads[i]);
    if (rc){
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
  }
  if (watch) {
    printf("%d Threads created\n", nthreads);
  }

  /* Fire them all off with the unlock - essentially using mutex as a signal */
  start = clock();
  pthread_mutex_unlock(&count_lock);
  if (watch) {
    printf("Go all threads\n");
  }

  /* Wait for each of the threads to complete. */
  for (i=0; i < nthreads; i++) {
    pthread_join(threads[i].thread, NULL);
  }

  /* Grab completion time and do analysis */
  end = clock();
  elapsed = 1000000.0 * (end-start) / CLOCKS_PER_SEC;
  printf("%ld at %f usec per count\n", global_count, elapsed/global_count);

  for (i=0; i < nthreads; i++) {
    printf("Thread %4d: %d\n", i, threads[i].count);
  }

  int min_count = threads[0].count;
  int max_count = threads[0].count;
  for (i=0; i < nthreads; i++) {
    min_count = min_count < threads[i].count ? min_count : threads[i].count;
    max_count = max_count > threads[i].count ? max_count : threads[i].count;
  }
  printf("Fairness: min=%d, ave=%f1, max=%d\n", min_count, 1.0 * global_limit/nthreads, max_count);
  return 0;
}
