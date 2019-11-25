#include <iostream> 
#include <string>
#include <vector> 
#include <sstream> 
#include <unistd.h>
#include <signal.h>
#include <fstream>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define SERVER_PORT	5432
#define MAX_PENDING	5
#define MAX_LINE	256

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

    /* about communication between processes */
    static vector<string> done_commands;
    struct sockaddr_in sin;
	int s;
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

    /* receive information from child process */
    static void* receive_info(void* __this);
public: 

    Shell(istream& _in, ostream& _out, string _shell_name, string _joiner); // construct function
    ~Shell() {pthread_exit(NULL);}
    string getDirName(); // get the name of the directory in the workpath
    void init(); // init the shell
    void run(); // run the shell
};
