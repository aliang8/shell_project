/*****************************************************************************
CShell Project
Systems 
Anthony Liang, Sam Xu, Shaeq Ahmed								   
*******************************************************************************/
//colors, parsing ;, fix ls, add ~

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <pwd.h>
#include "shell.h"
#include <termios.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>

#define GRN   "\033[01;32m"
#define BLU   "\033[01;34m"
#define RESET "\x1B[0m"

/**
 *@brief Prints a few pretty lines when u start the program
*/
void introScreen(){
  printf("\n\t===============================\n");
  printf("\t            C Shell \n");
  printf("\t By Sam Xu, Anthony Liang, & Shaeq Ahmed \n");
  printf("\n\t===============================\n\n\n");
}


/**
 *Print out the user prompt each line. 
 */
void shellPrompt(){
  struct passwd* userdata = getpwuid( geteuid());
  char prompt[1204] = "";
  gethostname(prompt, sizeof(prompt));
  char* cwd;
  char buff[1024];
  cwd = getcwd( buff, 1024);
  char *checkhome = strstr(cwd, userdata->pw_dir);
  char *tail;
  if (!checkhome)
    tail = 0;
  else
    tail = checkhome + strlen(userdata->pw_dir);
   char return_cwd [] = "";
  
  if (!tail) {
    strcpy(return_cwd, cwd);
  }
  else {
   strcpy(return_cwd, "~");
   strcat(return_cwd, tail);
}

  printf(GRN"%s@%s"RESET":"BLU"%s"RESET"$ ", getenv("LOGNAME"), prompt, return_cwd);
}

/**
 * cd method
 */
int cshell_cd(char* args[]){
  if (args[1] == NULL) {
    chdir(getenv("HOME")); 
    return 1;
  }
  else{ 
    if (chdir(args[1]) == -1) {
      printf(" %s: no such directory\n", args[1]);
      return -1;
    }
  }
  return 0;
}

/**
 * function responsible for excuting processes, can be in the background
 * 
 */ 
void cshell_exec(char **args, int background){	 
  int err = -1;
	 
  if((pid=fork())==-1){
    printf("Child process could not be created\n");
    return;
  }
  if(pid==0){
    signal(SIGINT, SIG_IGN);
    setenv("parent",getcwd(currentDir, 1024),1);        
    if (execvp(args[0],args)==err){
      printf("Command not found");
      kill(getpid(),SIGTERM);
    }
  }
	 
  if (background == 0){
    waitpid(pid,NULL,0);
  }else{
    printf("Process created with PID: %d\n",pid);
  }	 
}
 

void cshell_io(char * args[], char* inputFile, char* outputFile, int option){
	 
  int err = -1;
	
  int fileDescriptor; // between 0 and 19, describing the output or input file
	
  if((pid=fork())==-1){
    printf("Child process could not be created\n");
    return;
  }
  if(pid==0){
    if (option == 0){
      fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600); 
      dup2(fileDescriptor, STDOUT_FILENO); 
      close(fileDescriptor);
    }else if (option == 1){
      fileDescriptor = open(inputFile, O_RDONLY, 0600);  
      dup2(fileDescriptor, STDIN_FILENO);
      close(fileDescriptor);
      fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
      dup2(fileDescriptor, STDOUT_FILENO);
      close(fileDescriptor);		 
    }
		 
    setenv("parent",getcwd(currentDir, 1024),1);
		
    if (execvp(args[0],args)==err){
      printf("err");
      kill(getpid(),SIGTERM);
    }		 
  }
  waitpid(pid,NULL,0);
}


void cshell_pipeHandle(char * args[]){
  int filedes[2]; 
  int filedes2[2];
	
  int num_cmds = 0;
	
  char *command[256];
	
  pid_t pid;
	
  int err = -1;
  int end = 0;
        
  int i = 0;
  int j = 0;
  int k = 0;
  int l = 0;
        
  while (args[l] != NULL){
    if (strcmp(args[l],"|") == 0){
      num_cmds++;
    }
    l++;
  }
  num_cmds++;
        
  while (args[j] != NULL && end != 1){
    k = 0;
    while (strcmp(args[j],"|") != 0){
      command[k] = args[j];
      j++;	
      if (args[j] == NULL){
	end = 1;
	k++;
	break;
      }
      k++;
    }
    command[k] = NULL;
    j++;		
	        
    if (i % 2 != 0){
      pipe(filedes);
    }else{
      pipe(filedes2); 
    }
		
    pid=fork();
		
    if(pid==-1){			
      if (i != num_cmds - 1){
	if (i % 2 != 0){
	  close(filedes[1]);
	}else{
	  close(filedes2[1]); 
	} 
      }			
      printf("Child process could not be created\n");
      return;
    }
    if(pid==0){
      if (i == 0){
	dup2(filedes2[1], STDOUT_FILENO);
      }
      else if (i == num_cmds - 1){
	if (num_cmds % 2 != 0){ 
	  dup2(filedes[0],STDIN_FILENO);
	}else{ 
	  dup2(filedes2[0],STDIN_FILENO);
	}
      }else{
	if (i % 2 != 0){
	  dup2(filedes2[0],STDIN_FILENO); 
	  dup2(filedes[1],STDOUT_FILENO);
	}else{
	  dup2(filedes[0],STDIN_FILENO); 
	  dup2(filedes2[1],STDOUT_FILENO);					
	} 
      }
			
      if (execvp(command[0],command)==err){
	kill(getpid(),SIGTERM);
      }		
    }
			        
    if (i == 0){
      close(filedes2[1]);
    }
    else if (i == num_cmds - 1){
      if (num_cmds % 2 != 0){					
	close(filedes[0]);
      }else{					
	close(filedes2[0]);
      }
    }else{
      if (i % 2 != 0){					
	close(filedes2[0]);
	close(filedes[1]);
      }else{					
	close(filedes[0]);
	close(filedes2[1]);
      }
    }
				
    waitpid(pid,NULL,0);
				
    i++;	
  }
}
			
/**
 * Method used to handle the commands entered via the standard input
 */ 
int cshell_run(char * args[]){
  int i = 0;
  int j = 0;
	
  int fileDescriptor;
  int standardOut;
	
  int temp;
  int background = 0;
	
  char *args_temp[256];
        
  while ( args[j] != NULL){
    if ( (strcmp(args[j],">") == 0) || (strcmp(args[j],"<") == 0) || (strcmp(args[j],"&") == 0)){
      break;
    }
    args_temp[j] = args[j];
    j++;
  }
	
  // 'exit' command quits the shell
  if(strcmp(args[0],"exit") == 0) exit(0);
  else if (strcmp(args[0],"clear") == 0) system("clear");
  // 'pwd' command prints the current directory
  else if (strcmp(args[0],"pwd") == 0){
    if (args[j] != NULL){
      // If we want file output
      if ( (strcmp(args[j],">") == 0) && (args[j+1] != NULL) ){
	fileDescriptor = open(args[j+1], O_CREAT | O_TRUNC | O_WRONLY, 0600); 
	// We replace de standard output with the appropriate file
	standardOut = dup(STDOUT_FILENO); 	// first we make a copy of stdout
	// because we'll want it back
	dup2(fileDescriptor, STDOUT_FILENO); 
	close(fileDescriptor);
	printf("%s\n", getcwd(currentDir, 1024));
	dup2(standardOut, STDOUT_FILENO);
      }
    }else{
      printf("%s\n", getcwd(currentDir, 1024));
    }
  } 
  else if (strcmp(args[0],"cd") == 0) cshell_cd(args);
  else{
    while (args[i] != NULL && background == 0){
      if (strcmp(args[i],"&") == 0){
	background = 1;
      }else if (strcmp(args[i],"|") == 0){
	cshell_pipeHandle(args);
	return 1;
      }else if (strcmp(args[i],"<") == 0){
	temp = i+1;
	if (args[temp] == NULL || args[temp+1] == NULL || args[temp+2] == NULL ){
	  printf("Not enough input arguments\n");
	  return -1;
	}else{
	  if (strcmp(args[temp+1],">") != 0){
	    printf("Usage: Expected '>' and found %s\n",args[temp+1]);
	    return -2;
	  }
	}
	cshell_io(args_temp,args[i+1],args[i+3],1);
	return 1;
      }
      else if (strcmp(args[i],">") == 0){
	if (args[i+1] == NULL){
	  printf("Not enough input arguments\n");
	  return -1;
	}
	cshell_io(args_temp,NULL,args[i+1],0);
	return 1;
      }
      i++;
    }
    cshell_exec(args_temp,background);
  }
  return 1;
}

/**
 *  handler for SIGCHLD
 */
void signalHandler_child(int p){
  while (waitpid(-1, NULL, WNOHANG) > 0) {
  }
  printf("\n");
}

/**
 *  handler for SIGINT
 */
void signalHandler_int(int p){
  if (kill(pid,SIGTERM) == 0){
    printf("\nprocess %d received SIGINT\n",pid);
    no_reprint = 1;			
  }else{
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

//Easier implementation of reading input dynamically
char *cshell_read_line(){
  char *line = NULL;
  size_t bufsize = 0; // have getline allocate a buffer for us
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
  int bufsize = CSHELL_BUFSIZE;
  int pos = 0;
  char *line = malloc(sizeof(char)* bufsize);
  char c;

  while(1){
    //Reads in a character
    c = getchar();
    
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
#define SEMICOLON_DELIM ";"
char **cshell_split_line(char *line, char *delim)
{
  int bufsize = CSHELL_TOKEN_BUFSIZE, pos = 0;
  char **args = malloc(bufsize * sizeof(char*));
  char *arg;

  arg = strtok(line, delim);
  while (arg != NULL) {
    args[pos] = arg;
    pos++;

    if (pos >= bufsize) {
      bufsize *= 2;
      args = realloc(args, bufsize * sizeof(char*));
    }

    arg = strtok(NULL, delim);
  }
  args[pos] = NULL;
  return args;
}

char **parse_semicolon(char *line) {
  char **args;
  args = cshell_split_line(line, SEMICOLON_DELIM);
  return args;
}

/**
   @brief Loop for shell 
   @param argc Argument count
   @param argv Argument array of pointers
*/
int main(int argc, char **argv, char **envp) {  
  char *line;
  char **args;
  char **cmds;
  int status;
  
  no_reprint = 0;
  pid = -1337; //unusable pid for default
  initialize();
  introScreen();
 
  environ = envp;
  setenv("shell",getcwd(currentDir, 1024),1);
  
  do {
    if (no_reprint == 0) shellPrompt();
    no_reprint = 0;
    //line = cshell_read_line();
    line = readline("");
    if(!line)
      break;
    add_history(line);
    cmds = parse_semicolon(line);    
    int i;
    for(i = 0; cmds[i] != NULL; i++) {
      printf("%i\n",i);
      args = cshell_split_line(cmds[i], CSHELL_TOKEN_DELIM);
      status = cshell_run(args);
      free(line);
      free(args);
    }
    
  } while(status);
  return EXIT_SUCCESS;
}

  
  
