#include "myshell.cpp"

/**
 * main function
 */ 
int main() 
{
    signal(SIGINT, SIG_IGN); // prevent ctrl-c
    system ("clear");

    /* settinges */
    string shell_name;
    string joiner;
    cout << "Settings:\n";
    cout << "Shell Name (default by MyShell): ";
    getline(cin, shell_name);
    cout << "Joiner (default by &&, cannot be *): ";
    getline(cin, joiner);

    /* default settings */
    if (shell_name.empty()) shell_name = "MyShell";
    if (joiner.empty()) joiner = "&&";
    
    /* create the Shell object and run the shell */
    Shell myShell(cin, cout, shell_name, joiner);
    myShell.init();
    myShell.run();
    return EXIT_SUCCESS;
}