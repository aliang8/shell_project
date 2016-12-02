/*****************************************************************************
CShell Project
Systems 
Anthony Liang, Sam Xu, Shaeq Ahmed								   
*******************************************************************************/

#define TRUE 1
#define FALSE !TRUE

void signalHandler_child(int p);
//SIGINT
void signalHandler_int(int p);

char *cmds[] = {
  "cd",
  "exit"
};

char in [] = "<";
char out [] = ">";
char amp [] = "&";

static pid_t SH_PID;
static pid_t SH_PGID;
static int SH_IS_INTERACTIVE;
static struct termios SH_TMODES;

static char* currentDir;
extern char** environ;

struct sigaction act_child;
struct sigaction act_int;

int no_reprint; 

pid_t pid;

int changeDirectory(char * args[]);
