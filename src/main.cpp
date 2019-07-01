#include "myshell.cpp"

/**
 * A function that prevents ctrl + c
 */ 
void sigint_handler(int sig) 
{
	int olderrno = errno;
	pid_t fg_pid = getpid();
	if(fg_pid == 0) // no foreground group
 	   return;
	kill(-fg_pid, SIGINT); // send SIGINT to the whole foreground group
	errno = olderrno;
}

/**
 * main function
 */ 
int main() 
{
    signal(SIGINT,  sigint_handler);
    Shell myShell(cin, cout);
    myShell.init();
    myShell.run();
    return EXIT_SUCCESS;
}