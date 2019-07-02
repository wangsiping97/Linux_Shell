#include <iostream> 
#include <string>
#include <vector> 
#include <sstream> 
#include <unistd.h>
#include <signal.h>
#include <fstream>

using std::istream;
using std::ostream;
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::ifstream;
using std::ofstream; 

class Shell 
{
private: 

    /* features */
    istream& in; // input
    ostream& out; // output
    string name; // name of the user
    string shell_name; // name of shell
    string joiner; // joiner of multiple commands

    /* about command */
    string cmdstring; // the command read from the keyboard
    vector<string> vcmd; // vector-based short commands queue that are to be executed

    /* about path */
    string workPath; // work path of the program
    string defaultPath; // default path

    /* about background-running */
    bool background; // is the command executed in background
    int count; // number of background commands
    string fpath; // a file that store the information of done-jobs that executed in the background

private: 

    /* parsing the command */
    void split(string& command, char flag); // split the command/path
    bool parseCommand(); // parse the command
    string rebuildCommand(); // combine short commands to a string

    /* built-in commands */
    void bye(); // built-in bye
    int cd(vector<string>& tempcmd); // built-in cd

    /* executing the command */
    bool exec_cd(); // execute the built-in cd function
    int exec_command(string& cmdstring); // execute the command (not including "cd")
    void execute(); // execute the command

public: 

    Shell(istream& _in, ostream& _out, string _shell_name, string _joiner); // construct function
    string getDirName(); // get the name of the directory in the workpath
    void init(); // init the shell
    void run(); // run the shell
};
