#include "myshell.cpp"

/**
 * main function
 */ 
int main() 
{
    signal(SIGINT, SIG_IGN); // prevent ctrl-c
    Shell myShell(cin, cout);
    myShell.init();
    myShell.run();
    return EXIT_SUCCESS;
}