/***************************************************************************//**
  @file         main.c
  @author       Vlatka Lausic

*******************************************************************************/

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
/*
  Function Declarations for builtin shell commands:
 */
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

int lsh_echo(char **args);
int lsh_pwd(char **args);
int lsh_ls(char **args);
int lsh_mkdir(char **args);
int lsh_touch(char **args);
int lsh_rm(char **args);
int lsh_clear(char **args);
int lsh_copyFile(char **args);
/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "echo",
  "pwd",
  "ls",
  "mkdir",
  "touch",
  "rm",
  "clear",
  "copyFile"
};

int(*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_help,
  &lsh_exit,
  &lsh_echo,
  &lsh_pwd,
  &lsh_ls,
  &lsh_mkdir,
  &lsh_touch,
  &lsh_rm,
  &lsh_clear,
  &lsh_copyFile
};

int lsh_num_builtins() {
	return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/
int lsh_echo(char **args) {
	int i = 1;
	while (args[i] != NULL) {
		printf("%s ", args[i]);
		i++;
	}
	printf("\n");
	return 1;
}
int lsh_pwd(char **args)
{
	char a[1024];
	getcwd(a, sizeof(a));
	printf("Current directory: %s\n", a);
	return 1;
}

int lsh_ls(char **args)
{
	DIR *d;
	struct dirent *p;

	if (args[1] == NULL)
	{
		d = opendir("."); 
	}
	else
	{
		d = opendir(args[1]);

	}
	if (d != NULL) {
		while (p = readdir(d)) {
			printf("%s\n", p->d_name);
		}
		closedir(d);
	}
	else {
		perror("Couldn't open the directory.");
	}
	return 1;
}
int lsh_mkdir(char **args)
{
	if (args[1] == NULL) {
		fprintf(stderr, "lsh_mkdir: expected argument to \"mkdir\"\n");
	}
	else {
		if (mkdir(args[1], 0755) != 0) {
			perror("lsh");
		}
	}
	return 1;
}
int lsh_touch(char **args)
{
	if (args[1] == NULL) {
		fprintf(stderr, "lsh_touch: expected argument to \"touch\"\n");
	}
	else {
		if (creat(args[1], 0755) != 0) {
			perror("lsh");
		}
	}
	return 1;
}
int lsh_rm(char **args)
{
	if (remove(args[1]) == 0) {
		printf("File deleted successfully");
	}
	else {
		printf("Error: unable to delete the file");
	}
	return 1;
}
int lsh_clear(char **args) {
	system("clear");
	return 1;
}
int lsh_copyFile(char **args) {
	FILE* copyFrom = fopen(args[1], "r");
	FILE* copyTo = fopen(args[2], "w");
	for (;;) {
		int caractereActuel = fgetc(copyFrom);
		if (caractereActuel != EOF) {
			fputc(caractereActuel, copyTo);
		}
		else {
			break;
		}
	}
	fclose(copyFrom);
	fclose(copyTo);
	return 1;
}
/***/
void hello()
{
	printf("\n\t********Welcome to Vlatka Lausic - shell*******\n\n\n");
	

}
/**
   @brief Bultin command: change directory.
   @param args List of args.  args[0] is "cd".  args[1] is the directory.
   @return Always returns 1, to continue executing.
 */

int lsh_cd(char **args)
{
	if (args[1] == NULL) {
		fprintf(stderr, "lsh_cd: expected argument to \"cd\"\n");
	}
	else {
		if (chdir(args[1]) != 0) {
			perror("lsh");
		}
	}
	return 1;
}


/**
   @brief Builtin command: print help.
   @param args List of args.  Not examined.
   @return Always returns 1, to continue executing.
 */
int lsh_help(char **args)
{
	int i;
	printf("Stephen Brennan's LSH\n");
	printf("Type program names and arguments, and hit enter.\n");
	printf("The following are built in:\n");

	for (i = 0; i < lsh_num_builtins(); i++) {
		printf("  %s\n", builtin_str[i]);
	}

	printf("Use the man command for information on other programs.\n");
	return 1;
}

/**
   @brief Builtin command: exit.
   @param args List of args.  Not examined.
   @return Always returns 0, to terminate execution.
 */
int lsh_exit(char **args)
{
	return 0;
}

/**
  @brief Launch a program and wait for it to terminate.
  @param args Null terminated list of arguments (including program).
  @return Always returns 1, to continue execution.
 */

int lsh_launch(char **args)
{
	pid_t pid;
	int status;

	pid = fork();
	if (pid == 0) {
		// Child process
		if (execvp(args[0], args) == -1) {
			perror("lsh");
		}
		exit(EXIT_FAILURE);
	}
	else if (pid < 0) {
		// Error forking
		perror("lsh");
	}
	else {
		// Parent process
		do {
			waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;
}

/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
 */
int lsh_execute(char **args)
{
	int i;

	if (args[0] == NULL) {
		// An empty command was entered.
		return 1;
	}

	for (i = 0; i < lsh_num_builtins(); i++) {
		if (strcmp(args[0], builtin_str[i]) == 0) {
			return (*builtin_func[i])(args);
		}
	}

	return lsh_launch(args);
}

#define LSH_RL_BUFSIZE 1024
/**
   @brief Read a line of input from stdin.
   @return The line from stdin.
 */
char *lsh_read_line(void)
{
	int bufsize = LSH_RL_BUFSIZE;
	int position = 0;
	char *buffer = malloc(sizeof(char) * bufsize);
	int c;

	if (!buffer) {
		fprintf(stderr, "lsh: allocation error\n");
		exit(EXIT_FAILURE);
	}

	while (1) {
		// Read a character
		c = getchar();

		if (c == EOF) {
			exit(EXIT_SUCCESS);
		}
		else if (c == '\n') {
			buffer[position] = '\0';
			return buffer;
		}
		else {
			buffer[position] = c;
		}
		position++;

		// If we have exceeded the buffer, reallocate.
		if (position >= bufsize) {
			bufsize += LSH_RL_BUFSIZE;
			buffer = realloc(buffer, bufsize);
			if (!buffer) {
				fprintf(stderr, "lsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
/**
   @brief Split a line into tokens (very naively).
   @param line The line.
   @return Null-terminated array of tokens.
 */
char **lsh_split_line(char *line)
{
	int bufsize = LSH_TOK_BUFSIZE, position = 0;
	char **tokens = malloc(bufsize * sizeof(char*));
	char *token, **tokens_backup;

	if (!tokens) {
		fprintf(stderr, "lsh: allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, LSH_TOK_DELIM);
	while (token != NULL) {
		tokens[position] = token;
		position++;

		if (position >= bufsize) {
			bufsize += LSH_TOK_BUFSIZE;
			tokens_backup = tokens;
			tokens = realloc(tokens, bufsize * sizeof(char*));
			if (!tokens) {
				free(tokens_backup);
				fprintf(stderr, "lsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, LSH_TOK_DELIM);
	}
	tokens[position] = NULL;
	return tokens;
}

/**
   @brief Loop getting input and executing it.
 */
void lsh_loop(void)
{
	char *line;
	char **args;
	int status;
	char cwd[1024];
	do {
		getcwd(cwd, sizeof(cwd));
		printf("%s > ",cwd);
		line = lsh_read_line();
		args = lsh_split_line(line);
		status = lsh_execute(args);

		free(line);
		free(args);
	} while (status);
}

/**
   @brief Main entry point.
   @param argc Argument count.
   @param argv Argument vector.
   @return status code
 */
int main(int argc, char **argv)
{
	// Load config files, if any.

	// Run command loop.
	hello();
	lsh_loop();

	// Perform any shutdown/cleanup.

	return EXIT_SUCCESS;
}
