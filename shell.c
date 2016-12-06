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
#include <termios.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>

//Color codes
#define GRN   "\033[01;32m"
#define BLU   "\033[01;34m"
#define RESET "\x1B[0m"

/**
   @brief Prints a few pretty lines when u start the program
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
  printf("\t Anthony Liang, Shaeq Ahmed, Sam Xu \n");
  printf("\n\t===============================\n\n\n");
}

/**
   @brief Prints out the user prompt in linux format, added custom COLORS to make it more aesthetic!
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
   @brief CD method
   @param Args-the array of arguments
   @return Value whether the function ran successfully
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
   @brief Function responsible for executing processes
   @param Args-array of char pointers that represent the commands
*/ 
void cshell_exec(char **args){	 
  int err = -1;
  pid_t pid;
  if((pid=fork())==-1){
    printf("Child process could not be created\n");
    return;
  }
  if(pid==0){
    signal(SIGINT, SIG_IGN);        
    if (execvp(args[0],args)==err){
      printf("Command not found");
      kill(getpid(),SIGTERM);
    }
  } else {
    int status = 0;
    waitpid(pid, &status, 0);
  }
}
 
/**
   @brief Helper function to control the input/output of files
   @param Args-array of commands, i-input, o-output, option-reading/writing
*/ 
void cshell_io(char * args[], char* i, char* o, int option){
  
  int err = -1;
  pid_t pid;
  int fd;
  
  if((pid=fork())==-1){
    printf("Child process could not be created\n");
    return;
  }
  if(pid==0){
    // Option 0: >
    if (option == 0){
      // we open (create) the file truncating it at 0, for write only
      fd = open(o, O_CREAT | O_TRUNC | O_WRONLY, 0600); 
      // replace STDOUT with appropriate file
      dup2(fd, STDOUT_FILENO); 
      close(fd);
    }else if (option == 1){ // <
      fd = open(i, O_RDONLY, 0600);  
      dup2(fd, STDIN_FILENO);
      close(fd);
      fd = open(o, O_CREAT | O_TRUNC | O_WRONLY, 0644);
      dup2(fd, STDOUT_FILENO);
      close(fd);		 
    } else if (option == 2) { // 
      fd = open(o, O_CREAT | O_TRUNC | O_WRONLY, 0644);
      dup2(fd, STDERR_FILENO);
      close(fd);
    } else if (option == 3){
      fd = open(o, O_RDWR | O_CREAT | O_APPEND, 0644);
      dup2(fd, STDOUT_FILENO);
      close(fd);
    } else if (option == 4){
      fd = open(o, O_RDWR | O_CREAT | O_EXCL, 0644);
      dup2(fd, STDERR_FILENO);
      dup2(fd, STDOUT_FILENO); 
      close(fd);
    } else if (option == 5){
      fd = open(o, O_RDWR | O_CREAT | O_APPEND, 0644);
      dup2(fd, STDERR_FILENO);
      close(fd);
    }
   
    if (execvp(args[0],args)==err){
      printf("Command failed to execute");
      kill(getpid(),SIGTERM);
    }   
  }
  waitpid(pid,NULL,0);
}

/**
 @brief Helper function that handles piping, allows for multiple pipes
 @param Args - input commands
 */ 
void cshell_pipeHandle(char * args[]){
  // File descriptors
  int fd[2]; // pos. 0 output, pos. 1 input of the pipe
  int fd2[2];
  
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
      pipe(fd); // odd
    }else{
      pipe(fd2); // even
    }
    
    pid=fork();
    
    if(pid==-1){			
      if (i != num_cmds - 1){
	if (i % 2 != 0){
	  close(fd[1]); // for odd i
	}else{
	  close(fd2[1]); // for even i
	} 
      }			
      printf("Child process could not be created\n");
      return;
    }
    if(pid==0){
      if (i == 0){
	dup2(fd2[1], STDOUT_FILENO);
      }
      else if (i == num_cmds - 1){
	if (num_cmds % 2 != 0){ // for odd number of commands
	  dup2(fd[0],STDIN_FILENO);
	}else{ // for even number of commands
	  dup2(fd2[0],STDIN_FILENO);
	}
      }else{ // for odd i
	if (i % 2 != 0){
	  dup2(fd2[0],STDIN_FILENO); 
	  dup2(fd[1],STDOUT_FILENO);
	}else{ // for even i
	  dup2(fd[0],STDIN_FILENO); 
	  dup2(fd2[1],STDOUT_FILENO);					
	} 
      }
      
      if (execvp(command[0],command)==err){
	kill(getpid(),SIGTERM);
      }		
    }
    
    if (i == 0){
      close(fd2[1]);
    }
    else if (i == num_cmds - 1){
      if (num_cmds % 2 != 0){					
	close(fd[0]);
      }else{					
	close(fd2[0]);
      }
    }else{
      if (i % 2 != 0){					
	close(fd2[0]);
	close(fd[1]);
      }else{					
	close(fd[0]);
	close(fd2[1]);
      }
    }
    
    waitpid(pid,NULL,0);
    
    i++;	
  }
}

/**
 @brief Method used to handle the commands entered via the standard input
 @param Args - a list of arguments taken directly from the main loop
 @return Returns an integer to indicate success of the function
 */ 
int cshell_run(char * args[]){
  int i = 0;
  int j = 0;
	
  int temp;
  char *args_temp[256];
        
  while (args[j] != NULL){
    if ((strcmp(args[j],"2>") == 0)
	|| (strcmp(args[j],">") == 0)
	|| (strcmp(args[j],">>") == 0)
	|| (strcmp(args[j],"<") == 0) 
	|| (strcmp(args[j],"&>") == 0)
	|| (strcmp(args[j],"2>>") == 0)){
      break;
    }
    args_temp[j] = args[j];
    j++;
  }
  args_temp[j] = NULL;
       
  //handling the events of each command separately
  if(strcmp(args[0],"exit") == 0) exit(0);
  else if (strcmp(args[0],"cd") == 0) cshell_cd(args);
  else{
    while (args[i] != NULL){
      if (strcmp(args[i],"|") == 0){ //pipes
	cshell_pipeHandle(args);
	return 1;
      }else if (strcmp(args[i],">") == 0){ 
       	check(i,args,args_temp,NULL,args[i+1],0);
      }else if (strcmp(args[i],"<") == 0){
	temp = i+1;
	if (args[temp] == NULL){
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
	check(i,args,args_temp,NULL,args[i+1],2);
      }else if (strcmp(args[i],">>") == 0){
	check(i,args,args_temp,NULL,args[i+1],3);
      }else if (strcmp(args[i],"&>") == 0){
	check(i,args,args_temp,NULL,args[i+1],4);
      }else if (strcmp(args[i],"2>>") == 0){
	check(i,args,args_temp,NULL,args[i+1],5);
      }
      i++;
    }

    cshell_exec(args_temp);
  }
  return 1;
}


//helper function for cshell_io
int check(int i, char **args, char **args_temp, char *input, char *output, int option){
  if(args[i+1] == NULL){
    printf("Not enough input arguments\n");
    return -1;
  }
  cshell_io(args_temp,input,output,option);
  return 1;
}

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
   @brief Main loop for the shell
   @param argc - Argument count
   @param argv - Argument array of pointers
*/
int main(int argc, char **argv) {  
  char *line;
  char **args;
  char **cmds;
  int status;

  introScreen();
  
  do {
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
