/*
 * Defines an HTTP server that uses a thread pool to respond to HTTP requests.
 * Upon initialization, a fixed amount of threads are created. They will block
 * until an HTTP request is received. After serving an HTTP request, the thread
 * will return to the pool and await for more requests to be received.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dispatch.h"
#include "wq.h"

/* Worker routine for each thread in the pool. */
void *handle_clients(void *arg) {
  /* (Valgrind) Detach so thread frees its memory on completion, since we won't
   * be joining on it. */
  pthread_detach(pthread_self());

  /* BEGIN TASK 4 SOLUTION */

  /* END TASK 4 SOLUTION */
}

dispatcher_t *new_dispatcher(int concurrency, void (*request_handler)(int)) {
  /* BEGIN TASK 4 SOLUTION */

  /* END TASK 4 SOLUTION */
}

void dispatch(dispatcher_t* dispatcher, int client_socket_number) {
  /* BEGIN TASK 4 SOLUTION */

  /* END TASK 4 SOLUTION */
}
