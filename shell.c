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
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <pwd.h>
#include "shell.h"
#include <termios.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>

//Color codes
#define GRN   "\033[01;32m"
#define BLU   "\033[01;34m"
#define RESET "\x1B[0m"

/**
 *@brief Prints a few pretty lines when u start the program
*/
void introScreen(){
  printf( "\n\t _____   _____  _            _  _ \n");
printf("\t/  __ \\ /  ___|| |          | || |\n");
printf("\t| /  \\/ \\ `--. | |__    ___ | || |\n");
printf("\t| |      `--. \\| '_ \\  / _ \\| || |\n");
printf("\t| \\__/\\ /\\__/ /| | | ||  __/| || |\n");
printf(" \t \\____/ \\____/ |_| |_| \\___||_||_|\n");

  printf("\n\t===============================\n\n");
  printf("\t            C Shell \n");
  printf("\t By Sam Xu, Anthony Liang, & Shaeq Ahmed \n");
  printf("\n\t===============================\n\n\n");
}


/**
 *@brief Prints out the user prompt in linux format, added custom COLORS to make it more aesthetic!
 */

/**
 *@brief Prints out the user prompt in linux format, added custom COLORS to make it more aesthetic!
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
  char* ret = malloc(strlen(return_cwd)+99);
  sprintf(ret,GRN"%s@%s"RESET":"BLU"%s"RESET"$ ", username, hostname, return_cwd); //ayy colorful prompt

  free(hostname);
  free(username);
  free(cwd);
  free(return_cwd);
  return ret;
}

/**
 *@brief cd method
 *@param args -  the array  of arguments, directories in this case
 *@return each value reports the sucess rate of the arguments
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
 *@brief function responsible for excuting processes
 *@param args - the array of strings as arguments
 *@param background - checks whether a background process is occuring
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
 
/**
 *@brief this is a helper function that helps control the writing and reading of files
 *@param args - the array of strings as arguments
 *@param inputfile - name of the input file
 *@param outputfile - name of the output file
 *@param option - controls whether we are writing or reading, > or <
 */ 

void cshell_io(char * args[], char* inputFile, char* outputFile, int option){
  
  int err = -1;
  
  int fileDescriptor; // between 0 and 19, describing the output or input file
  
  if((pid=fork())==-1){
    printf("Child process could not be created\n");
    return;
  }
  if(pid==0){
    // Option 0: output redirection
    if (option == 0){
      // We open (create) the file truncating it at 0, for write only
      fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600); 
      // We replace de standard output with the appropriate file
      dup2(fileDescriptor, STDOUT_FILENO); 
      close(fileDescriptor);
      // Option 1: input and output redirection
    }else if (option == 1){
      // We open file for read only (it's STDIN)
      fileDescriptor = open(inputFile, O_RDONLY, 0600);  
      // We replace de standard input with the appropriate file
      dup2(fileDescriptor, STDIN_FILENO);
      close(fileDescriptor);
      // Same as before for the output file
      fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
      dup2(fileDescriptor, STDOUT_FILENO);
      close(fileDescriptor);		 
    } else if (option == 2) {
      fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
      dup2(fileDescriptor, STDERR_FILENO);
      close(fileDescriptor);
    } else if (option == 3){
      fileDescriptor = open(outputFile, O_RDWR | O_CREAT | O_APPEND, 0600);
      dup2(fileDescriptor, STDOUT_FILENO);
      close(fileDescriptor);
    } else if (option == 4){
      fileDescriptor = open(outputFile, O_RDWR | O_CREAT | O_EXCL, 0600);
      dup2(STDOUT_FILENO, STDERR_FILENO);

    setenv("parent",getcwd(currentDir, 1024),1);
   
    if (execvp(args[0],args)==err){
      printf("err");
      kill(getpid(),SIGTERM);
    }   
  }
  waitpid(pid,NULL,0);
}

/**
 *@brief helper function responsible for the helper 
 *@param args - a list of arguments
 */ 
void cshell_pipeHandle(char * args[]){
  // File descriptors
  int filedes[2]; // pos. 0 output, pos. 1 input of the pipe
  int filedes2[2];
  
  int num_cmds = 0;
  
  char *command[256];
  
  pid_t pid;
  
  int err = -1;
  int end = 0;
  
  // Variables used for the different loops
  int i = 0;
  int j = 0;
  int k = 0;
  int l = 0;
  
  // First we calculate the number of commands (they are separated
	// by '|')
  while (args[l] != NULL){
    if (strcmp(args[l],"|") == 0){
      num_cmds++;
    }
    l++;
  }
  num_cmds++;
  
  // Main loop of this method. For each command between '|', the
  // pipes will be configured and standard input and/or output will
  // be replaced. Then it will be executed
  while (args[j] != NULL && end != 1){
    k = 0;
    // We use an auxiliary array of pointers to store the command
    // that will be executed on each iteration
    while (strcmp(args[j],"|") != 0){
      command[k] = args[j];
      j++;	
      if (args[j] == NULL){
	// 'end' variable used to keep the program from entering
	// again in the loop when no more arguments are found
	end = 1;
	k++;
	break;
      }
      k++;
    }
    // Last position of the command will be NULL to indicate that
    // it is its end when we pass it to the exec function
    command[k] = NULL;
    j++;		
    
    // Depending on whether we are in an iteration or another, we
    // will set different descriptors for the pipes inputs and
    // output. This way, a pipe will be shared between each two
    // iterations, enabling us to connect the inputs and outputs of
    // the two different commands.
    if (i % 2 != 0){
      pipe(filedes); // for odd i
    }else{
      pipe(filedes2); // for even i
    }
    
    pid=fork();
    
    if(pid==-1){			
      if (i != num_cmds - 1){
	if (i % 2 != 0){
	  close(filedes[1]); // for odd i
	}else{
	  close(filedes2[1]); // for even i
	} 
      }			
      printf("Child process could not be created\n");
      return;
    }
    if(pid==0){
      // If we are in the first command
      if (i == 0){
	dup2(filedes2[1], STDOUT_FILENO);
      }
      // If we are in the last command, depending on whether it
      // is placed in an odd or even position, we will replace
      // the standard input for one pipe or another. The standard
      // output will be untouched because we want to see the 
      // output in the terminal
      else if (i == num_cmds - 1){
	if (num_cmds % 2 != 0){ // for odd number of commands
	  dup2(filedes[0],STDIN_FILENO);
	}else{ // for even number of commands
	  dup2(filedes2[0],STDIN_FILENO);
	}
	// If we are in a command that is in the middle, we will
	// have to use two pipes, one for input and another for
	// output. The position is also important in order to choose
	// which file descriptor corresponds to each input/output
      }else{ // for odd i
	if (i % 2 != 0){
	  dup2(filedes2[0],STDIN_FILENO); 
	  dup2(filedes[1],STDOUT_FILENO);
	}else{ // for even i
	  dup2(filedes[0],STDIN_FILENO); 
	  dup2(filedes2[1],STDOUT_FILENO);					
	} 
      }
      
      if (execvp(command[0],command)==err){
	kill(getpid(),SIGTERM);
      }		
    }
    
    // CLOSING DESCRIPTORS ON PARENT
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
 *@brief Method used to handle the commands entered via the standard input
 *@param args - a list of arguments taken directly from the main loop
 */ 
int cshell_run(char * args[]){


  int i = 0;
  int j = 0;
	
  int fileDescriptor;
  int standardOut;
	
  int temp;
  int background = 0;
	
  char *args_temp[256];
        
  while (args[j] != NULL){
    if ((strcmp(args[j],"2>") == 0)
	|| (strcmp(args[j],">") == 0)
	|| (strcmp(args[j],">>") == 0)
	|| (strcmp(args[j],"<") == 0) 
	|| (strcmp(args[j],"&>") == 0)){
      break;
    }
    args_temp[j] = args[j];
    j++;
  }
  args_temp[j] = NULL;
	
  // 'exit' command quits the shell
  if(strcmp(args[0],"exit") == 0) exit(0);
  else if (strcmp(args[0],"cd") == 0) cshell_cd(args);
  else{
    while (args[i] != NULL && background == 0){
      if (strcmp(args[i],"&") == 0){
	background = 1;
      }else if (strcmp(args[i],"|") == 0){
	cshell_pipeHandle(args);
	cshell_run(args_temp);
	return 1;
      }else if (strcmp(args[i],"<") == 0){
	temp = i+1;
	if (args[temp] == NULL/* || args[temp+1] == NULL || args[temp+2] == NULL */){
	  printf("Not enough input arguments\n");
	  return -1;
	}else{
	  if (args[temp+1] == NULL){
	    cshell_io(args_temp,args[i+1],NULL,1);
	    return 1;
	  }
	  if (strcmp(args[temp+1],">") != 0){
	    printf("Usage: Expected '>' and found %s\n",args[temp+1]);
	    return -2;
	  }
	}
	cshell_io(args_temp,args[i+1],args[i+3],1);
	return 1;
      }else if (strcmp(args[i],"2>") == 0){
	if (args[i+1] == NULL){
	  printf("Not enough input arguments\n");
	  return -1;
	}
	cshell_io(args_temp,NULL,args[i+1],2);
	return 1;
      }else if (strcmp(args[i],">>") == 0){
	if (args[i+1] == NULL){
	  printf("Not enough input arguments\n");
	  return -1;
	}
	cshell_io(args_temp,NULL,args[i+1],3);
	return 1;
      }else if (strcmp(args[i],">") == 0){
	if (args[i+1] == NULL){
	  printf("Not enough input arguments\n");
	  return -1;
	}
	cshell_io(args_temp,NULL,args[i+1],0);
	cshell_run(args_temp);
	return 1;
      }else if (strcmp(args[i],"&>") == 0){
	if (args[i+1] == NULL){
	  printf("Not enough input arguments\n");
	  return -1;
	}
	cshell_io(args_temp,NULL,args[i+1],4);
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


/**
   @brief Easier implementation of reading input dynamically
   @return a line of string imported from user input / stdin
*/
char *cshell_read_line(){
  char *line = NULL;
  size_t bufsize = 0; // have getline allocate a buffer for us
  getline(&line, &bufsize, stdin);
  return line;
}

//Initial buffer size
#define CSHELL_BUFSIZE 256


/**
 @brief Split line 
 @param Line read from cshell_read_line
 @return Array of args
*/
#define CSHELL_TOKEN_BUFSIZE 64
#define CSHELL_TOKEN_DELIM " \t\r\n\a"
#define SEMICOLON_DELIM ";"
char **cshell_split_line(char *line, char *delim){
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

/**
   @brief Parses multiple commands with ';'
   @param Line read from cshell_read_line
   @return Array of args, with ; as the delimiter
*/
char **parse_semicolon(char *line) {
  char **args;
  args = cshell_split_line(line, SEMICOLON_DELIM);
  return args;
}

/**
 @brief Checks whether or not the input line contains only whitespace.
 @param Line read from cshell_read_line
 @return 1 if the line contains only white space, 0 if it contains arguments
*/
int is_empty(const char *s) {
  while (*s != '\0') {
    if (!isspace(*s))
      return 0;
    s++;
  }
  return 1;
}

/**
   @brief Loop for shell 
   @param argc Argument count
   @param argv Argument array of pointers
*/
int main(int argc, char **argv) {  
  char *line;
  char **args;
  char **cmds;
  int status;
  
  no_reprint = 0;
  pid = -1337; //unusable pid for default
  initialize();
  introScreen();
 
  setenv("shell",getcwd(currentDir, 1024),1);
  
  do {
    if (no_reprint == 0)// shellPrompt();
      no_reprint = 0;
    line = readline(makeprompt());
    if(!is_empty(line)){
      add_history(line);
      cmds = parse_semicolon(line);
      int i;
      for(i = 0; cmds[i] != NULL; i++) {
	args = cshell_split_line(cmds[i], CSHELL_TOKEN_DELIM);
	status = cshell_run(args);
      }
      free(line);
      free(args);
    }
  } while(status);
  return EXIT_SUCCESS;
}
