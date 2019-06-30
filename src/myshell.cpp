#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <iostream> 
#include <stdlib.h>
#include <string>
#include <vector> 
#include <sstream> 
#include <termios.h>

using std::istream;
using std::ostream;
using std::cin;
using std::cout;
using std::string;
using std::vector;

class Shell {
private: 
    istream& in; // input
    ostream& out; // output
    vector<string> vcmd; // vector of command
    string workPath; // work path of the program
    string defaultPath; // default path
    string name; // name of the user
    bool background; // is the command executed in background
    int count; // number of background commands
private: 
    void split(string& command, char flag); // split the command/path
    bool parseCommand(); // parse the command
    void execute(); // execute the command
    string getPath(); // get 
    int cd(vector<string>& tempcmd); // built-in cd
    void bye(); // built-in bye
    string rebuildCommand();
    bool exec_cd();
    int exec_command(string& cmdstring);
public: 
    Shell(istream& _in, ostream& _out); // construct function
    string getDirName(); // get the name of the directory in the workpath
    void init(); // init the shell
    void run(); // run the shell
};

/**
 * Contruct function
 * Initialize the background and count
 */
Shell::Shell(istream& _in, ostream& _out):in(_in), out(_out) {
    vcmd.clear();
    background = false;
    count = 0;
}

/**
 * Init function
 * Init the shell program with an UI
 * Initialize the work path as well as the default path
 * Switch the work path to the default path
 */ 
void Shell::init() {
    system("clear");
    out << string(5, '\n');
    out << "                                 My Shell                        \n";
    out << string(3, '\n');
    out << "                            by Siping Wang @ THU                        \n";
    out << string(5, '\n');
    // get the workPath 
    workPath = getcwd(NULL, 0);
    split(workPath, '/');
    // get the user's name
    if (vcmd.size() == 2) name = vcmd[1];
    else name = vcmd[2];
    // cd to the user's default path
    if (chdir(("/" + vcmd[1] + "/" + name).c_str()) != 0) {
        if (chdir(("/" + name).c_str()) != 0) {
            perror("ERROR");
            EXIT_FAILURE;
        }
    }
    // get the root(default) path
    defaultPath = getcwd(NULL, 0);
    // change the work path
    workPath = defaultPath;
}

/**
 * Built-in "bye" function
 * Exit the program with a bye-string
 */ 
void Shell::bye() {
    out << "MyShell: Thanks for using : )\n";
}

/**
 * Built-in cd function
 * If the command is just "cd", switch the work path to the defaultPath
 * If the command is "cd + [path]", switch the work path to [path]
 * Then, consider if the command is to be executed in the background
 * If not, change the work path of this program
 */ 
int Shell::cd(vector<string>& tempcmd) {
    if (tempcmd.size() == 1) { // 默认目录
        if (chdir((defaultPath).c_str()) != 0) {
            perror("MyShell");
            return -1;
        }
    }
    else {
        if (chdir(tempcmd[1].c_str()) != 0) {
            perror("MyShell");
            return -1;
        }
    }
    if (background == false) {
        workPath = getcwd(NULL, 0);
    }
    return 0;
}

/**
 * A function that executes the cd-series commands in the program
 * First decide whether there are cd-series commands in the command
 * Then decide whether the command is with an argument
 * Then execute the command and update the command-queue
 */ 
bool Shell::exec_cd() {
    int sz = vcmd.size();
    if (sz == 0) return true;
    vector<string> tempcmd;
    tempcmd.clear();
    if (vcmd[0] == "cd") {
        if (sz == 1) {
            tempcmd.push_back(vcmd[0]);
            if(cd(tempcmd)) return false;
            vcmd.clear();
        }
        else if (vcmd[1] == "&&") {
            tempcmd.push_back(vcmd[0]);
            if(cd(tempcmd)) return false;
            vcmd.erase(vcmd.begin());
            vcmd.erase(vcmd.begin());
        }
        else if (sz == 2) {
            tempcmd.push_back(vcmd[0]);
            tempcmd.push_back(vcmd[1]);
            if(cd(tempcmd)) return false;
            vcmd.clear();
        }
        else {
            tempcmd.push_back(vcmd[0]);
            tempcmd.push_back(vcmd[1]);
            if(cd(tempcmd)) return false;
            vcmd.erase(vcmd.begin());
            vcmd.erase(vcmd.begin());
            vcmd.erase(vcmd.begin()); // &&
        }
    }
    return true;
}

int Shell::exec_command(string& cmdstring) {
    pid_t pid;
    int status;
    if(cmdstring.empty()){
        return (1);
    }

    if((pid = fork())<0){
        status = -1;
    }
    
    else if(pid == 0){
        execl("/bin/sh", "MyShell", "-c", cmdstring.data(), (char *)0);
    }
    else{
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return status;
}

string Shell::getDirName() {
    return workPath.substr(workPath.find_last_of('/') + 1); 
}

void Shell::split(string& command, char flag) {
    vcmd.clear();
    command.erase(0,command.find_first_not_of(" ")); // 去除首端多余空格
    command.erase(command.find_last_not_of(" ") + 1); // 去掉尾端多余空格
    std::istringstream iss(command);
    string temp;
    while (getline(iss, temp, flag)) 
        vcmd.push_back(temp);
}

string Shell::rebuildCommand() {
    string command;
    command.clear();
    int sz = vcmd.size();
    for (int i = 0; i < sz; ++i) {
        command += vcmd[i];
        command += " ";
    }
    return command;
}

bool Shell::parseCommand() {
    if (vcmd.empty()) {
        return true;
    }
    if (vcmd[0] == "bye" || vcmd[0] == "exit") {
        bye();
        return false;
    }
    if (vcmd.back() == "*") {
        background = true;
        vcmd.pop_back();
        count++;
    }
    else {
        background = false;
    }
    execute();
    return true;
}

void Shell::execute() {
    if (background == false) {
        if (!exec_cd()) return;
        string command = rebuildCommand();
        exec_command(command);
    }
    else {
        pid_t pid;
        int status;
        pid = fork();
        if (pid == 0) {
            string initcommand = rebuildCommand();
            if (!exec_cd()) return;
            string command = rebuildCommand();
            exec_command(command);
            out << "\n[" << count << "]" <<"+  Done\t" << initcommand << "\n"; 
            exit(EXIT_FAILURE);
        }
        else if (pid < 0) {
            perror("MyShell");
        }
        else {
            out << "[" << count << "]" << "(" << pid << ")\n"; 
        }
    }
}

void Shell::run() {
    string dirName;
    string line;
    int status;
    do {
        dirName = getDirName();
        if (dirName == name) dirName = "~";
        // output the prompt
        out << "MyShell:" << dirName << " " << name << "$ ";
        // read the command from the keyboard
        getline(cin, line);
        // split the command with ' '
        split(line, ' ');
        status = parseCommand();
    } while (status);
}

void sigint_handler(int sig) 
{
	int olderrno = errno;
	pid_t fg_pid = getpid();
	if(fg_pid == 0) // no foreground group
 	   return;
	kill(-fg_pid, SIGINT); // send SIGINT to the whole foreground group
	errno = olderrno;
}

int main() {
    signal(SIGINT,  sigint_handler); // prevent ctrl + c
    Shell myShell(cin, cout);
    myShell.init();
    myShell.run();
    return EXIT_SUCCESS;
}