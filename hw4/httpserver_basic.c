/*
 * Defines a single-process, single-threaded HTTP server. The same server process that
 * listens and accepts requets is the same process that sends the HTTP response.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dispatch.h"

dispatcher_t *new_dispatcher(int concurrency, void (*request_handler)(int)) {
  dispatcher_t *dispatcher =  malloc(sizeof(dispatcher_t));
  dispatcher->request_handler = request_handler;
  return dispatcher;
}

void dispatch(dispatcher_t* dispatcher, int client_socket_number) {
  dispatcher->request_handler(client_socket_number);
  close(client_socket_number);
}
