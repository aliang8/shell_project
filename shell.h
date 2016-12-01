/*****************************************************************************
CShell Project
Systems 
Anthony Liang, Sam Xu, Shaeq Ahmed								   
*******************************************************************************/

/* SIGNAL HANDLER IMLPEMENTATION */
//SIGCHILD

void signalHandler_child(int p);
//SIGINT
void signalHandler_int(int p);

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
