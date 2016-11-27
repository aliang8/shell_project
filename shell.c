/*****************************************************************************
CShell Project
Systems 
Anthony Liang, Sam Xu, Shaeq Ahmed								   
*******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/**
  @brief Function headers for builtin shell commands
*/
int cshell_cd(char **args);
int cshell_help(char **args);
int cshell_exit(char **args);

char *cmds[] = {
  "cd"
  "help"
  "exit"
};

/**
  @brief Builtin function implementation
*/
int cshell_cd(char **args){
  if (args[1] == NULL) {
    fprintf(stderr, "cd expects argument");
  } else {
    chdir(args[1]);
    return 1;
  }
}

//Easier implementation of reading input dynamically
char *cshell_read_line(){
  char *line = NULL;
  ssize_t bufsize = 0; // have getline allocate a buffer for us
  getline(&line, &bufsize, stdin);
  return line;
}

//Initial buffer size
#define CSHELL_BUFSIZE 256
/**
   @brief Read input from stdin
   @return Line from stdin
*/

/*
char *cshell_read_line(){
  int bufsize = CSHELL_BUFSIZE, pos = 0;
  char *line = malloc(sizeof(char)* bufsize);
  char c;

  while(1){
    //Reads in a character
    c = getc(stdin);
    
    //If we hit EOF, replace it with null and return
    if (c == EOF || c == '\n'){
      line[pos] = '\0';
      return line;
    } else {
      line[pos] = c;
    }
    pos++;

    //If we exceed the buffer size, dynamically reallocate memory.
    if (pos > bufsize){
      bufsize += CSHELL_BUFSIZE;
      line = realloc(line, bufsize);
    }
  }
}
*/


//Token size
/**
   @brief Split line 
   @param Line read from cshell_read_line
   @return Array of args
*/
#define CSHELL_TOKEN_BUFSIZE 64
#define CSHELL_TOKEN_DELIM " \t\r\n\a"
char **cshell_split_line(char *line)
{
  int bufsize = CSHELL_TOKEN_BUFSIZE, pos = 0;
  char **args = malloc(bufsize * sizeof(char*));
  char *arg;

  arg = strtok(line, CSHELL_TOKEN_DELIM);
  while (arg != NULL) {
    args[pos] = arg;
    pos++;

    if (pos >= bufsize) {
      bufsize *= 2;
      args = realloc(args, bufsize * sizeof(char*));
    }

    arg = strtok(NULL, CSHELL_TOKEN_DELIM);
  }
  args[pos] = NULL;
  return args;
}

/**
   @brief Run program and wait for it to terminate
   @param args from cshell_split_line
   @return Always return 1 to continue execution
*/
int cshell_run(char **args){
  pid_t pid, wpid;
  int status;
  
  pid = fork();
  
  //Child process
  if (pid == 0){
    if (execvp(args[0],args) == -1){
      perror("Failed to execute commands");
    }
    exit(EXIT_FAILURE);
  } else {
    //Parent process
    do {
      //Waits for child process to finish
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    //WIFEXITED checks if child process terminated normally
    //WIFSIGNALED checks if child process terminated by signal
  }
  return 1;
}

/**
   @brief Execute commands if is implemented
*/
int cshell_execute(char **args){
  if (args[0] == NULL){
    //empty command was entered
    return 1;
  }
  
  int i = 0;
  for (i; i < 

/**
   @brief Loop for interpreting and executing commands
*/
void cshell_loop(){
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = cshell_read_line();
    args = cshell_split_line(line);
    status = cshell_execute(args);
    
    free(line);
    free(args);
  } while (status);
}

/**
   @brief Loop for shell 
   @param argc Argument count
   @param argv Argument array of pointers
*/
int main(int argc, char **argv) {  
  cshell_loop();
  return EXIT_SUCCESS;
}
