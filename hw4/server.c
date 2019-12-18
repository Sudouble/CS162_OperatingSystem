#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <unistd.h>

#include "libhttp.h"
#include "dispatch.h"

/*
 * Global configuration variables.
 * You need to use these in your implementation of handle_files_request and
 * handle_proxy_request. Their values are set up in main() using the
 * command line arguments (already implemented for you).
 */
int server_fd;
int server_port;
char *server_files_directory;
dispatcher_t *dispatcher;

/*
 * Serves the file at path to the the socket fd.
 * The `lseek()` syscall may be useful to find the
 * size of a file. `snprintf()` may be useful to
 * convert integers -> strings. The functions provided
 * in libhttp.c may be useful.
 */
void serve_file(int socket_fd, char *path) {
  /* BEGIN TASK 1 SOLUTION */
  int fd = open(path, O_RDONLY);
  if (fd == -1)
	  exit(0);
  off_t off = lseek(fd, 0, SEEK_END);
  if (off == -1)
	  exit(0);
  off = lseek(fd, 0, SEEK_SET);
  char *buff = (char*)malloc(sizeof(char)*(off+1));
  ssize_t nResult = read(fd, buff, off);
  if (nResult == -1)
	  exit(0);
  
  char lenBuff[200];
  snprintf(lenBuff, 200, "%ld", off);
  
  http_start_response(socket_fd, 200);
  http_send_header(socket_fd, "Content-Type", "text/html");
  http_send_header(socket_fd, "Content-Length", lenBuff); // filesize
  http_end_headers(socket_fd);
  http_send_string(socket_fd, buff);
  
  free(buff);
  close(fd);
  /* END TASK 1 SOLUTION */
}

/*
 * Serves the directory at path to the the socket fd.
 * The `opendir()` and `readdir()` syscalls may be
 * useful here. The function provided in libhttp.c
 * might also be useful.
 */
void serve_directory(int socket_fd, char *path) {
  /* BEGIN TASK 1 SOLUTION */
  char* pFile = "index.html";
  char* pBuff = (char*)malloc(sizeof(char)*(strlen(path)+strlen(pFile)+2));
  strcat(pBuff, path);
  strcat(pBuff, "/");
  strcat(pBuff, pFile);
  
  if(access(pBuff, X_OK) != 1) { // path contain index.html
	serve_file(socket_fd, pBuff);
  }else {				
	int nCount = 0;
	char* CRLF = "\r\n";
	char* pBuffSend = (char*)malloc(sizeof(char)*(1000));	
	
	DIR* pDir = opendir(path);
	if (pDir == NULL)
		exit(0);	
	struct dirent *stDir = readdir(pDir);
	while (stDir) {
		if (stDir->d_type == DT_DIR) {
			strcat(pBuffSend, stDir->d_name);
			nCount += strlen(stDir->d_name);
			strcat(pBuffSend, CRLF);
			nCount += strlen(CRLF);
		}
		stDir = readdir(pDir);
	}
	char* pParent = "<a href=\"../\">Parent directory</a>";
	strcat(pBuffSend, pParent);
	nCount += strlen(pParent);
	
	// length;
	char lengBuff[200] = {0};
	snprintf(lengBuff, 200, "%d", nCount);
	
	http_start_response(socket_fd, 200);
	http_send_header(socket_fd, "Content-Type", "text/html");
	http_send_header(socket_fd, "Content-Length", lengBuff); // directorysize
	http_end_headers(socket_fd);
	http_send_string(socket_fd, pParent);
	
	free(pBuffSend);
  }
  free(pBuff);  
  /* END TASK 1 SOLUTION */
}


/*
 * Reads an HTTP request from stream (fd), and writes an HTTP response
 * containing:
 *
 *   1) If user requested an existing file, respond with the file
 *   2) If user requested a directory and index.html exists in the directory,
 *      send the index.html file.
 *   3) If user requested a directory and index.html doesn't exist, send a list
 *      of files in the directory with links to each.
 *   4) Send a 404 Not Found response.
 */
void handle_files_request(int socket_fd) {
  
  struct http_request *request = http_request_parse(socket_fd);

  if (request == NULL || request->path[0] != '/') {
    http_start_response(socket_fd, 400);
    http_send_header(socket_fd, "Content-Type", "text/html");
    http_end_headers(socket_fd);
    http_send_string(socket_fd, "<center><h1>400 Bad Request</h1><hr></center>");
    return;
  }

  if (strstr(request->path, "..") != NULL) {
    http_start_response(socket_fd, 403);
    http_send_header(socket_fd, "Content-Type", "text/html");
    http_end_headers(socket_fd);
    http_send_string(socket_fd, "<center><h1>403 Forbidden</h1><hr></center>");
    return;
  }

  /* Remove beginning / */
  char *path = malloc(2 + strlen(request->path) + 1);
  path[0] = '.';
  path[1] = '/';
  memcpy(path + 2, request->path, strlen(request->path) + 1);

  // The `stat()` syscall might be useful to make the distinction
  // between files and directories. Call `serve_file()`, `serve_directory()`,
  // or a 404 Not Found when appropriate.
  struct stat file_stat;

  /* BEGIN TASK 1 SOLUTION */
  if(stat(path, &file_stat) == -1) {
	  perror("stat");
	  return;
  }
  switch (file_stat.st_mode & S_IFMT) {
	 case S_IFDIR:		 // directory
		serve_directory(socket_fd, path);
		return;
	 case S_IFREG:  	// files
		serve_file(socket_fd, path);
		return;
	 default:
		break;
  }
  /* END TASK 1 SOLUTION */

  http_start_response(socket_fd, 404);
  http_send_header(socket_fd, "Content-Type", "text/html");
  http_end_headers(socket_fd);
  http_send_string(socket_fd, "<center><h1>404 Not Found</h1><hr></center>");

  return;
}

/*
 * Opens a TCP stream socket on all interfaces with port number PORTNO. Saves
 * the fd number of the server socket in *socket_number. For each accepted
 * connection, calls request_handler with the accepted fd number.
 */
void serve_forever(int *socket_number) {

  struct sockaddr_in server_address, client_address;
  size_t client_address_length = sizeof(client_address);
  int client_socket_number;

  *socket_number = socket(PF_INET, SOCK_STREAM, 0);
  if (*socket_number == -1) {
    perror("Failed to create a new socket");
    exit(errno);
  }

  int socket_option = 1;
  if (setsockopt(*socket_number, SOL_SOCKET, SO_REUSEADDR, &socket_option,
        sizeof(socket_option)) == -1) {
    perror("Failed to set socket options");
    exit(errno);
  }

  memset(&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(server_port);

  if (bind(*socket_number, (struct sockaddr *) &server_address,
        sizeof(server_address)) == -1) {
    perror("Failed to bind on socket");
    exit(errno);
  }

  if (listen(*socket_number, 1024) == -1) {
    perror("Failed to listen on socket");
    exit(errno);
  }

  printf("Listening on port %d...\n", server_port);

  while (1) {
    client_socket_number = accept(*socket_number,
        (struct sockaddr *) &client_address,
        (socklen_t *) &client_address_length);
    if (client_socket_number < 0) {
      perror("Error accepting socket");
      continue;
    }

    printf("Accepted connection from %s on port %d\n",
        inet_ntoa(client_address.sin_addr),
        client_address.sin_port);
    
    dispatch(dispatcher, client_socket_number);
  }
}

void signal_callback_handler(int signum) {
  printf("Caught signal %d: %s\n", signum, strsignal(signum));
  printf("Closing socket %d\n", server_fd);
  if (shutdown(server_fd, SHUT_RDWR) < 0) perror("Failed to shutdown socket at server_fd (ignoring)\n");
  if (close(server_fd) < 0) perror("Failed to close server_fd (ignoring)\n");
  exit(EXIT_SUCCESS);
}

char *USAGE =
  "--files www_directory/ [--port 8000 --concurrency 5]\n";

void exit_with_usage(char *executable_name) {
	fprintf(stderr, "Usage:");
	fprintf(stderr, " %s ", executable_name);
  fprintf(stderr, "%s", USAGE);
  exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
  signal(SIGINT, signal_callback_handler);

  /* Default settings */
  server_port = 8000;
  int concurrency = 1;

  int i;
  for (i = 1; i < argc; i++) {
    if (strcmp("--files", argv[i]) == 0) {
      server_files_directory = argv[++i];
      if (!server_files_directory) {
        fprintf(stderr, "Expected argument after --files\n");
        exit_with_usage(argv[0]);
      }
    } else if (strcmp("--port", argv[i]) == 0) {
      char *server_port_string = argv[++i];
      if (!server_port_string) {
        fprintf(stderr, "Expected argument after --port\n");
        exit_with_usage(argv[0]);
      }
      server_port = atoi(server_port_string);
    } else if (strcmp("--concurrency", argv[i]) == 0) {
      char *concurrency_str = argv[++i];
      if (!concurrency_str || (concurrency = atoi(concurrency_str)) < 1) {
        fprintf(stderr, "Expected positive integer after --concurrency\n");
        exit_with_usage(argv[0]);
      }
    } else if (strcmp("--help", argv[i]) == 0) {
      exit_with_usage(argv[0]);
    } else {
      fprintf(stderr, "Unrecognized option: %s\n", argv[i]);
      exit_with_usage(argv[0]);
    }
  }

  if (server_files_directory == NULL) {
    fprintf(stderr, "Please specify \"--files [DIRECTORY]\"\n");
    exit_with_usage(argv[0]);
  }
  
  chdir(server_files_directory);
  dispatcher = new_dispatcher(concurrency, handle_files_request);
  serve_forever(&server_fd);
  
  exit(EXIT_SUCCESS);
}
