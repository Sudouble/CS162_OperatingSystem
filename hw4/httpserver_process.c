/*
 * Defines an HTTP Server that creates a child process to send HTTP Responses.
 * The parent process should return to listening and accepting HTTP Requests.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "dispatch.h"

dispatcher_t *new_dispatcher(int concurrency, void (*request_handler)(int)) {
  /* BEGIN TASK 2 SOLUTION */

  /*END TASK 2 SOLUTION */
}

void dispatch(dispatcher_t* dispatcher, int client_socket_number) {
  /* BEGIN TASK 2 SOLUTION */

  /*END TASK 2 SOLUTION */
}
