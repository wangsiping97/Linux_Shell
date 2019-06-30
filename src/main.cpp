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

enum TYPE {
    foreground,
    background
};

class Jobs {
private: 
    pid_t pid;
    TYPE type;
};

class Shell {
private: 
    istream& in;
    ostream& out;
    vector<string> vcmd;
    string workPath; // 工作目录
    string rootPath; // 根目录
    string name;
    bool background;
    int count;
private: 
    void split(string& command, char flag);
    bool parseCommand();
    bool execute();
    string getPath();
    void cd(vector<string>& tempcmd);
    void help();
    void bye();
    int myexecvp(vector<string> tempcmd);
public: 
    Shell(istream& _in, ostream& _out);
    string getDirName();
    void init();
    void run();
};


Shell::Shell(istream& _in, ostream& _out):in(_in), out(_out) {
    vcmd.clear();
    background = false;
    count = 0;
}

void Shell::init() {
    system("clear");
    out << string(5, '\n');
    out << "                                 My Shell                        \n";
    out << string(3, '\n');
    out << "                            by Siping Wang @ THU                        \n";
    out << string(5, '\n');
    workPath = getcwd(NULL, 0);
    split(workPath, '/');
    if (vcmd.size() == 2) name = vcmd[1];
    else name = vcmd[2];
    if (chdir(("/" + vcmd[1] + "/" + name).c_str()) != 0) {
        if (chdir(("/" + name).c_str()) != 0) {
            perror("ERROR");
            EXIT_FAILURE;
        }
    }
    rootPath = getcwd(NULL, 0);
    workPath = rootPath;
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

bool Shell::execute() {
    pid_t pid;
    int status;
    pid = fork();
    if (pid == 0) {
        // check if there are multiple commands in the line, then execute all the commands.
        vector<string> tempcmd;
        int res;
        int i = 0;
        int sz = vcmd.size();
        do {
            tempcmd.clear();
            while (i < sz && vcmd[i] != "&&") {
                tempcmd.push_back(vcmd[i]);
                i++;
            }
            res = myexecvp(tempcmd);
            if (res == -1) {
                perror("MyShell");
                return false;
            }
        } while (i <= sz && !res);
    }
    else if (pid < 0) {
        perror("MyShell");
        return false;
    }
    else {
        do {
            if (background == false)
                waitpid(pid, &status, WUNTRACED);
            else {
                out << "[" << count << "]" << "(" << pid << ")\n"; 
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return true;
}

void Shell::cd(vector<string>& tempcmd) {
    if (tempcmd.size() == 1) { // 默认去根目录
        if (chdir((rootPath).c_str()) != 0) {
            perror("MyShell");
        }
    }
    else {
        if (chdir(tempcmd[1].c_str()) != 0) {
            perror("MyShell");
        }
    }
    if (background == false)
        workPath = getcwd(NULL, 0);
}

void Shell::help() {
    out << "These shell commands are defined internally.\n";
    out << "Type program names and arguments, then hit enter.\n";
    out << "Use \'man -k\' or \'info\' to find out more about commands not in this list.\n";
}

void Shell::bye() {
    out << "MyShell: Thanks for using ~\n";
}

int Shell::myexecvp(vector<string> tempcmd) {
    int argc = tempcmd.size();
    if (argc == 0) // command starts with "&&"
    {
        return 0;
    }
    if (tempcmd[0] == "cd" && tempcmd.size() <= 2) {
        cd(tempcmd);
        return 0;
    }
    if (tempcmd[0] == "help") {
        help();
        return 0;
    }
    if (tempcmd[0] == "bye" && tempcmd.size() == 1) {
        bye();
        return 0;
    }
    char ** args = new char* [argc + 1];
    for (int i = 0; i < argc; ++i) {
        args[i] = (char*)tempcmd[i].data();
    }
    args[argc] = NULL;
    return execvp(args[0], args);
}

bool Shell::parseCommand() {
    if (vcmd.empty()) return true;
    if (vcmd.back() == "*") {
        background = true;
        vcmd.pop_back();
        count++;
    }
    else {
        background = false;
    }
    if (vcmd[0] == "bye" && vcmd.size() == 1 && background == false) {
        bye();
        return false;
    }
    if (vcmd.empty()) // An empty command was entered.
    {
        if (background == false)
            return true;
        if (background == true) {
            out << "MyShell: syntax error near unexpected token `*'\n";
            return true;
        }
    }
    if (vcmd[0] == "&&") {
        out << "MyShell: syntax error near unexpected token `&&'\n";
        return true;
    }
    return execute();
}

void Shell::run() {
    string line;
    string dirName;
    int status;
    do {
        dirName = getDirName();
        if (dirName == name) dirName = "~";
        out << "MyShell:" << dirName << " " << name << "$ ";
        getline(cin, line);
        split(line, ' ');
        status = parseCommand();
    } while (status);
}

void sigint_handler(int sig) 
{
	int olderrno = errno;
	pid_t fg_pid = getpid();
	if(fg_pid == 0) //no foreground group
 	   return;
	kill(-fg_pid, SIGINT); //send SIGINT to the whole foreground group
	errno = olderrno;
}



int main() {
    signal(SIGINT,  sigint_handler); // prevent ctrl + c
    Shell myShell(cin, cout);
    myShell.init();
    myShell.run();
    return EXIT_SUCCESS;
}