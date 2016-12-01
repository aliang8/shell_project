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
#include "shell.h"
#include <termios.h>
#include <signal.h>

#define GRN   "\033[01;32m"
#define BLU   "\033[01;34m"
#define RESET "\x1B[0m"

#define MAXTOKEN 128 //max tokens
#define MAXLINE 1024 //max characters

//function headers for builtin commands
int cshell_cd(char **args);
int cshell_help(char **args);
int cshell_exit(char **args);

/**
   @brief Prints a few pretty lines when u start the program
*/
void introScreen(){
  printf("\n\t===============================\n");
  printf("\t            C Shell \n");
  printf("\t By Sam Xu, Anthony Liang, & Shaeq Ahmed \n");
  printf("\n\t===============================\n\n\n");
}

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
   @brief Execute commands if is implemented
*/
int cshell_execute(char **args){
  int i = 0;
  int j = 0;

  int fileDesc;
  int stdOUT;

  int temp;
  int backg = 0;

  char *arg_temp[128];

  //Looking for special characters and stores it in a seperate array
  while(args[j]!=NULL){
    if((strcmp(args[j],out)== 0)||(strcmp(args[j],in)== 0)||(strcmp(args[j],"amp")== 0)){
      break;
    }
    args_temp[j] = args[j];
    j++;
  }
  
  if(strcmp(args[0],cmds[1])==0){
    exit(0);
  }else if(strcmp(args[0],cmds[0])==0){
    cshell_cd(args);
  }else{
    while(args[i]!= NULL & background == 0){
      if (strcmp(args[i],amp) == 0){
	background = 1;
      }else if(strcmp(args[i],"|")==0){
	pipeHandler(args);
	return 1;
      }
    }
  }
  
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

/* void cshell_loop(){ */
/*   char *line; */
/*   char **args; */
/*   int status; */
/*   do { */
/*     //allows for autocompletion */
/*     //rl_bind_key('\t',rl_complete); */
/*     //printf("> "); */
/*     //line = cshell_read_line(); */
/*     char* prompt = makeprompt(); */
/*     line = readline(prompt); */
/*     if(!line) */
/*       break; */
/*     add_history(line); */
/*     args = cshell_split_line(line); */
/*     status = cshell_execute(args); */
    
/*     free(line); */
/*     free(args); */
/*     free(prompt); */
/*   } while (status); */
/* } */


void signalHandler_int(int i){
  if(kill(pid,SIGTERM) == 0) {
    printf("\nprocessing %d a SIGINT signal\n", pid);
    no_reprint = 1;
  }else{
    printf("\n");
  }
}

void signalHandler_child(int i){
  while(waitpid(-1, NULL, WNOHANG) > 0){
    printf("\n");
  }
}


/**
   @brief Making sure the subshell is not running as a foreground job. 
   *Initialize the pid of the subshell so it could support job control
   *Post initialization allows the sub-shell to have its own child processes
   *We used the approach explained here to set things up
   *www.gnu.org/software/libc/manual/html_node/Initializing-the-Shell.html
*/
void initialize(){
  SH_PID = getpid();
  SH_IS_INTERACTIVE = isatty(STDIN_FILENO); //safety
  
  if (SH_IS_INTERACTIVE){
    //Loop the shell into the foreground
    while(tcgetpgrp(STDIN_FILENO) != (SH_PGID = getpgrp()))
      kill(SH_PID,SIGTTIN);

    
    act_child.sa_handler = signalHandler_child;
    act_int.sa_handler = signalHandler_int;

    sigaction(SIGCHLD, &act_child, 0);
    sigaction(SIGINT, &act_int, 0);

    setpgid(SH_PID, SH_PID);
    SH_PGID = getpgrp();
    if(SH_PID != SH_PGID){
      printf("Something went wrong, unable to set shell as process leader");
      exit(EXIT_FAILURE);
    }

    currentDir = (char*) calloc(1024, sizeof(char));

    //sets and controls the terminal with the shell
    tcsetpgrp(STDIN_FILENO, SH_PGID);
    tcgetattr(STDIN_FILENO, &SH_TMODES);

  }
  
}


/**
   @brief Loop for shell 
   @param argc Argument count
   @param argv Argument array of pointers
*/
int main(int argc, char **argv) {
  char line[MAXLINE];
  char *tokens[MAXTOKEN];
  int numTokens;

  no_reprint = 0; //prevent reprint in certain cases

  pid = -1337; //impossible pid

  initialize();
  introScreen();

  while(1){
    if(no_reprint == 0){
      memset(line, '\0', MAXLINE); //emptys and resets line
      fgets(line, MAXLINE, stdin); //gets user input
      if((tokens[0] = strtok(line," \n\t")) != NULL){
	numTokens = 1;
	//counts the num of tokens
	while((tokens[numTokens] = strtok(NULL, " \n\t") != NULL)){
	  numTokens++;
	}
	cshell_run(tokens);
      }
    }
  }
  
  
  exit(0);
}
