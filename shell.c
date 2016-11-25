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
   @brief Loop for interpreting and executing commands
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
*/

//Initial buffer size
#define CSHELL_BUFSIZE 2
/**
   @brief Read input from stdin
   @return Line from stdin
*/
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

/**
   @brief Loop for shell 
   @param argc Argument count
   @param argv Argument array of pointers
*/
int main(int argc, char **argv) {
  /*
  cshell_loop();
  return EXIT_SUCCESS;
  */
  char *line = cshell_read_line();
  printf("%s\n",line);
}
