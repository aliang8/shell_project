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
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <pwd.h>

#define GRN   "\033[01;32m"
#define BLU   "\033[01;34m"
#define RESET "\x1B[0m"

//function headers for builtin commands
int cshell_cd(char **args);
int cshell_help(char **args);
int cshell_exit(char **args);

char *cmds[] = {
  "cd",
  "help",
  "exit"
};

char in [] = "<";
char out [] = ">";

int infound(char** args){
  int i = -1;
  while(args[++i] != NULL){
    if(strcmp(args[i],in) == 0){
      return i;
    }
  }
  return -1;
}

int outfound(char** args){
  int i = -1;
  while(args[++i] != NULL){
    if(strcmp(args[i],out) == 0){
      return i;
    }
  }
  return -1;
}

//array of function pointers
int (*func[])(char**) = {
  &cshell_cd,
  &cshell_exit
};

//returns the number of commands
int num_cmds(){
  return sizeof(cmds) / sizeof(char *);
}

/**
   @brief Builtin function implementation
*/
//cd 
//implement when cd shell is not a directory

int cshell_cd(char **args){
  if (args[1] == NULL) {
    fprintf(stderr, "cd expects argument");
  } else {
    chdir(args[1]);
    return 1;
  }
}

//exit
int cshell_exit(char **args){
  //cshell_loop is terminated when status is 0
  return 0;
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



/**
   @brief Split line 
   @param Line read from cshell_read_line
   @return Array of args
*/
//Token size
#define CSHELL_TOKEN_BUFSIZE 64
#define CSHELL_TOKEN_DELIM " \t\r\n\a"
char **cshell_split_line(char *line){
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

  int in = infound(args);
  int out = outfound(args);

  if(in != -1){
    
  }
  if(out != -1){
    int fd1 = creat(args[out+1], 0644);
    dup2(fd1, STDOUT_FILENO);
    close(fd1);
    args[out] = NULL;
    out = -1;
  }
  
  int i = 0;
  for (i; i < num_cmds(); i++){
    if (strcmp(args[0], cmds[i]) == 0){
      //if command is valid
      return (*func[i])(args);
    }
  }
  //if not builtin command
  return cshell_run(args);
}

/**
   @brief Loop for interpreting and executing commands
*/

char* makeprompt(){

  //make the struct
  struct passwd *userdata = getpwuid( geteuid());
  //get the username
  char *username = (char *) malloc( 33 );
  strcpy(username, userdata->pw_name);
  //get the hostname
  char *hostname = (char *) malloc( 65 );
  gethostname(hostname, 64);
  //get and format cwd (replace with ~ for $HOME)
  int size = 1;
  char *cwd = (char *) malloc(size * sizeof(char));
  while (! getcwd(cwd, size) )
  {
    size++;
    cwd = (char *) realloc( cwd, size * sizeof(char) );
  }
  char *check_home_dir = strstr(cwd, userdata->pw_dir);
  char *end_part;

  if (! check_home_dir)
    end_part = 0;
  else
    end_part = check_home_dir + strlen(userdata->pw_dir);

  char *return_cwd = 0;

  if (! end_part)
    return_cwd = cwd;
  else
  {
    return_cwd = malloc( size + strlen(end_part)+1);
    strcpy(return_cwd, "~");
    strcat(return_cwd, end_part);
  }


  char* ret = malloc(strlen(return_cwd)+100);
  sprintf(ret,GRN"%s@%s"RESET":"BLU"%s"RESET"$ ", username, hostname, return_cwd); //ayy colorful prompt

  free(hostname);
  free(username);
  free(cwd);
  free(return_cwd);
  return ret;
}

void cshell_loop(){
  char *line;
  char **args;
  int status;
  do {
    //allows for autocompletion
    //rl_bind_key('\t',rl_complete);
    //printf("> ");
    //line = cshell_read_line();
    char* prompt = makeprompt();
    line = readline(prompt);
    if(!line)
      break;
    add_history(line);
    args = cshell_split_line(line);
    status = cshell_execute(args);
    
    free(line);
    free(args);
    free(prompt);
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
