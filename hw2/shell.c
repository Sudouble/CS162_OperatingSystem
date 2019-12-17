#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>

#include "tokenizer.h"

/* Convenience macro to silence compiler warnings about unused function parameters. */
#define unused __attribute__((unused))

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;

/* File descriptor for the shell input */
int shell_terminal;

/* Terminal mode settings for the shell */
struct termios shell_tmodes;

/* Process group id for the shell */
pid_t shell_pgid;

int cmd_exit(struct tokens *tokens);
int cmd_help(struct tokens *tokens);
int cmd_pwd(struct tokens *tokens);
int cmd_cd(struct tokens *tokens);

int cmd_exe(struct tokens *tokens);
char* get_path_resolution(char *fileName);
bool redirect_stdin(struct tokens *tokens);
bool redirect_stdout(struct tokens *tokens);

/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(struct tokens *tokens);

/* Built-in command struct and lookup table */
typedef struct fun_desc {
  cmd_fun_t *fun;
  char *cmd;
  char *doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
  {cmd_help, "?", "show this help menu"},
  {cmd_exit, "exit", "exit the command shell"},
  {cmd_pwd, "pwd", "current working directory."},
  {cmd_cd, "cd", "change the current working directory."},
  {cmd_exe, "", "execute the program: program argvs"},
};

/* Prints a helpful description for the given command */
int cmd_help(unused struct tokens *tokens) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
  return 1;
}

/* Exits this shell */
int cmd_exit(unused struct tokens *tokens) {
  exit(0);
}

int cmd_pwd(struct tokens *tokens)
{
	char abs_path_buff[PATH_MAX];
	getcwd(abs_path_buff, PATH_MAX-1);
	printf("%s\n", abs_path_buff);
	return 1;
}

int cmd_cd(struct tokens *tokens)
{
	int nResult = chdir(tokens_get_token(tokens, 1));
	if (nResult == 0)
	{
		cmd_pwd(tokens);
		return 1;
	}
	else
	{
		printf("Error Directory.\n");
		return 0;
	}
}

int cmd_exe(struct tokens *tokens)
{
	pid_t pid = fork();
	int exitcode;
	if (pid == 0) { // child process
		setpgid(0, 0);
		tcsetpgrp(0, getpgrp());
		
		// Reset signal handlers
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGTSTP, SIG_DFL);
		signal(SIGTTIN, SIG_DFL);
		signal(SIGTTOU, SIG_DFL);
		signal(SIGCHLD, SIG_DFL);
	
		size_t nSize = tokens_get_length(tokens);
		if (redirect_stdin(tokens) || redirect_stdout(tokens)) {
			nSize -= 2;
		}
			
		char* pProgram = tokens_get_token(tokens, 0);
		char** pArgv = (char**)malloc(sizeof(char*)*(nSize+1));
		for (int i = 0; i < nSize; i++) {
			pArgv[i] = tokens_get_token(tokens, i);
		}
		pArgv[nSize] = NULL;
		
		char *fileName = get_path_resolution(pProgram);
		if (fileName == NULL)
			return 0;
		// printf("%s-%s\n", pProgram, pArgv[1]);
		execv(fileName, pArgv);
		
		free(pArgv);
	} else {
		waitpid(pid, &exitcode, 0);
		tcsetpgrp(0, getpgrp());
	}
	return 1;
}

// Redirect STDIN if redirect in tokens
bool redirect_stdin(struct tokens *tokens)
{
	int num_args = tokens_get_length(tokens);
	if (num_args >= 3 && strcmp(tokens_get_token(tokens, num_args-2), "<") == 0) {
		int fd = open(tokens_get_token(tokens, num_args-1), O_RDONLY);
		dup2(fd, 0);
		close(fd);
		return true;
	}
	return false;
}

// Redirect STDOUT if redirect in tokens
bool redirect_stdout(struct tokens *tokens)
{
	int num_args = tokens_get_length(tokens);
	if (num_args >= 3 && strcmp(tokens_get_token(tokens, num_args-2), ">") == 0) {
		printf("Opening file %s.\n", tokens_get_token(tokens, num_args-1));
		int fd = open(tokens_get_token(tokens, num_args-1), O_RDWR|O_TRUNC|O_CREAT, 0777);
		dup2(fd, 1);
		close(fd);
		return true;
	}
	return false;
}

char* get_path_resolution(char *fileName)
{
	if (strlen(fileName) <= 0)
		return NULL;	
	
	if (fileName[0] == '/') {
		if (access(fileName, X_OK) != -1)
			return fileName;
		else
			return NULL;
	}
	
	char* pPath = getenv("PATH");
	const char delim[2] = ":";
	char *buff = (char*)malloc(sizeof(char)*PATH_MAX);
	if (buff == NULL)
	{
		//fail("malloc failed.\n", 1);
		return NULL;
	}
	
	char *token = strtok(pPath, delim);
	while (token != NULL)
	{
		strcpy(buff, token);
		strcat(buff, "/");
		strcat(buff, fileName);
		
		// printf("trying: %s\n", buff);
		int nRet = access(buff, X_OK);
		if (nRet != -1)
		{
			// printf("find one: %s\n", buff);
			return buff;
		}
		
		token = strtok(NULL, delim);
	}
	free(buff);	
	return NULL;
}

/* Looks up the built-in command, if it exists. */
int lookup(char cmd[]) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0))
      return i;
  return -1;
}

/* Intialization procedures for this shell */
void init_shell() {
  /* Our shell is connected to standard input. */
  shell_terminal = STDIN_FILENO;

  /* Check if we are running interactively */
  shell_is_interactive = isatty(shell_terminal);

  if (shell_is_interactive) {
    /* If the shell is not currently in the foreground, we must pause the shell until it becomes a
     * foreground process. We use SIGTTIN to pause the shell. When the shell gets moved to the
     * foreground, we'll receive a SIGCONT. */
    while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
      kill(-shell_pgid, SIGTTIN);

    /* Saves the shell's process id */
    shell_pgid = getpid();

    /* Take control of the terminal */
    tcsetpgrp(shell_terminal, shell_pgid);

    /* Save the current termios to a variable, so it can be restored later. */
    tcgetattr(shell_terminal, &shell_tmodes);
  }
}

int main(unused int argc, unused char *argv[]) {
  init_shell();

  static char line[4096];
  int line_num = 0;

  /* Please only print shell prompts when standard input is not a tty */
  if (shell_is_interactive)
    fprintf(stdout, "%d: ", line_num);

  while (fgets(line, 4096, stdin)) {
    /* Split our line into words. */
    struct tokens *tokens = tokenize(line);

    /* Find which built-in function to run. */
    int fundex = lookup(tokens_get_token(tokens, 0));

    if (fundex >= 0) {
      cmd_table[fundex].fun(tokens);
    } else {
      /* REPLACE this to run commands as programs. */
      //fprintf(stdout, "This shell doesn't know how to run programs.\n");
	  cmd_exe(tokens);
    }

    if (shell_is_interactive)
      /* Please only print shell prompts when standard input is not a tty */
      fprintf(stdout, "%d: ", ++line_num);

    /* Clean up memory */
    tokens_destroy(tokens);
  }

  return 0;
}
