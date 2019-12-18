#ifndef __DISPATCH__
#define __DISPATCH__

#include <pthread.h>
#include "wq.h"

/* DISPATCH defines a common interface for which requests are handed
   to workers to be served. Each type of server will have its own
   way to serve responses to the client. */

/* Not all attributes may be needed for all parts. The work_queue
   and workers fields are only used to implement the thread pool. */
typedef struct dispatcher {
  pthread_t *workers;
  wq_t work_queue;
  void (*request_handler)(int);
} dispatcher_t;

/*
 * Mallocs and returns a new dispatcher
 */
dispatcher_t *new_dispatcher(int concurrency, void (*request_handler)(int));

/*
 * Using the provided dispatcher, handles an accepted request.
 */
void dispatch(dispatcher_t* dispatcher, int client_socket_number);

#endif
