/*
 * Defines an HTTP Server that spins off a new thread to send HTTP Responses.
 * The main thread should continue listening and accepting HTTP Requests.
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dispatch.h"

struct client_info {
    void (*request_handler)(int);
    int client_socket_number;
};

void handle_client(void * arg) {
  /* BEGIN TASK 3 SOLUTION */

  /* END TASK 3 SOLUTION */
}

dispatcher_t *new_dispatcher(int concurrency, void (*request_handler)(int)) {
  /* BEGIN TASK 3 SOLUTION */

  /* END TASK 3 SOLUTION */
}

void dispatch(dispatcher_t* dispatcher, int client_socket_number) {
  /* BEGIN TASK 3 SOLUTION */

  /* END TASK 3 SOLUTION */
}
