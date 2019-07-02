#include "myshell.h"


/**
 * Contruct function
 * Initialize the background, count, name (default by the program)
 * Get the work path of the program
 */
Shell::Shell(istream& _in, ostream& _out, string _shell_name, string _joiner):in(_in), out(_out), shell_name(_shell_name), joiner(_joiner)
{
    vcmd.clear();
    cmdstring.clear();
    background = false;
    count = 0;
    name = "user";
    workPath = getcwd(NULL, 0); // get the workPath 
    fpath = workPath + "/../bin/output";
    ofstream fout (fpath); // clear the file
}

/**
 * Init function
 * Init the shell program with an UI
 * Initialize the work path as well as the default path
 * Switch the work path to the default path
 * Update `name`
 */ 
void Shell::init() 
{
    system("clear");
    out << string(5, '\n');
    out << "                                 My Shell                        \n";
    out << string(3, '\n');
    out << "                            by Siping Wang @ THU                        \n";
    out << string(5, '\n');
    split(workPath, '/');
    // get the user's name
    if (vcmd.size() == 2) name = vcmd[1];
    else name = vcmd[2];
    // cd to the user's default path
    if (chdir(("/" + vcmd[1] + "/" + name).c_str()) != 0) 
    {
        if (chdir(("/" + name).c_str()) != 0) 
        {
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
void Shell::bye() 
{
    out << shell_name << ": Thanks for using : )\n\n\n[进程已完成]\n\n";
}

/**
 * Built-in cd function
 * If the command is just "cd", switch the work path to the defaultPath
 * If the command is "cd + [path]", switch the work path to [path]
 * Then, consider if the command is to be executed in the background
 * If not, change the work path of this program
 */ 
int Shell::cd(vector<string>& tempcmd) 
{
    if (tempcmd.size() == 1) // to the default path
    { 
        if (chdir((defaultPath).c_str()) != 0) 
        {
            perror(shell_name.data());
            return -1;
        }
    }
    else 
    {
        if (chdir(tempcmd[1].c_str()) != 0) // cd with an argument, to the argument path
        { 
            perror(shell_name.data());
            return -1;
        }
    }
    if (background == false) // if the command is to be executed in the foreground, change `workPath`
    { 
        workPath = getcwd(NULL, 0);
    }
    return 0;
}

/**
 * A function that executes the cd-series commands in the program
 * First decide whether there are cd-series commands in the command
 * Then decide whether the command is with an argument
 * Then execute the command and update `vcmd` (a vecotr-based command queue)
 */ 
bool Shell::exec_cd() 
{
    int sz = vcmd.size();
    if (sz == 0) return true;
    vector<string> tempcmd;
    tempcmd.clear();
    if (vcmd[0] == "cd") 
    {
        if (sz == 1) // there is only a "cd" in the command
        {
            tempcmd.push_back(vcmd[0]);
            vcmd.clear();
            if(cd(tempcmd)) return false;
        }
        else if (vcmd[1] == joiner) // there is a "cd" with no argument and other commands
        {
            tempcmd.push_back(vcmd[0]);
            vcmd.erase(vcmd.begin());
            vcmd.erase(vcmd.begin());
            if(cd(tempcmd)) return false;
        }
        else if (sz == 2) // there is only a "cd" with argument in the command
        {
            tempcmd.push_back(vcmd[0]);
            tempcmd.push_back(vcmd[1]);
            vcmd.clear();
            if(cd(tempcmd)) return false;
        }
        else // there is a "cd" with argument and other commands
        {
            tempcmd.push_back(vcmd[0]);
            tempcmd.push_back(vcmd[1]);
            vcmd.erase(vcmd.begin());
            vcmd.erase(vcmd.begin());
            vcmd.erase(vcmd.begin()); // pop joiner from the command queue
            if(cd(tempcmd)) return false;
        }
    }
    return true;
}

/**
 * A function that executes the command string
 * Create a child process to run the command, using the execl funtion
 */ 
int Shell::exec_command(string& cmdstring) 
{
    pid_t pid;
    int status;
    // if the string is empty, return 1
    if(cmdstring.empty())
    {
        return (1);
    }
    if((pid = fork()) < 0) // error in creating the process
    {
        status = -1;
    }
    /* child process */
    else if(pid == 0)
    {
        execl("/bin/sh", shell_name.data(), "-c", cmdstring.data(), (char *)0);
    }
    /* parent process */
    else 
    {
        do
        {
            waitpid(pid, &status, WUNTRACED);
        } 
        while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return status;
}

/**
 * A function that return the name of the directory of the work parh
 * Using in displaying the prompt information
 */ 
string Shell::getDirName() 
{
    return workPath.substr(workPath.find_last_of('/') + 1); 
}

/**
 * Split the initial command string to a set of short commands
 * Also used in splitting the path with flag '/', to find `name` and `defaultPath`
 */ 
void Shell::split(string& command, char flag) 
{
    vcmd.clear();
    command.erase(0,command.find_first_not_of(" ")); // 去除首端多余空格
    command.erase(command.find_last_not_of(" ") + 1); // 去掉尾端多余空格
    std::istringstream iss(command);
    string temp;
    while (getline(iss, temp, flag)) 
    {
        vcmd.push_back(temp);
    }
}

/**
 * Combine some short commands to a string, so that to be executed
 * Jump the cd-series comamnds as well as joiner
 * Update `vcmd`
 */ 
string Shell::rebuildCommand() 
{
    string command;
    command.clear();
    if (!vcmd.empty() && vcmd[0] == joiner) //  the commands start with an joiner
    {
        vcmd.erase(vcmd.begin());
    }
    while (!vcmd.empty() && vcmd[0] != "cd" && vcmd[0] != joiner) // jump the cd-series commands
    {
        command += vcmd[0];
        command += " ";
        vcmd.erase(vcmd.begin());
    }
    return command;
}

/**
 * Parse `vcmd` on the first place
 * Find if the command starts with a "bye" or "exit", then exit the program
 * Find if the command ends with a "*", then switch `background` to true
 * Update `vcmd`
 */ 
bool Shell::parseCommand() 
{
    if (vcmd.empty()) 
    {
        return true;
    }
    if (vcmd[0] == "bye" || vcmd[0] == "exit") 
    {
        bye();
        return false;
    }
    if (vcmd[0] == joiner) 
    {
        out << shell_name.data() << ": syntax error near unexpected token `" << joiner <<"'\n";
        return true;
    }
    if (vcmd[0] == "*") 
    {
        out << shell_name.data() << ": syntax error near unexpected token `*'\n";
        return true;
    }

    /* switch `background` and then pass the command to be executed */
    if (vcmd.back() == "*") 
    {
        background = true;
        vcmd.pop_back();
        count++;
    }
    else 
    {
        background = false;
    }
    execute();
    return true;
}

/**
 * A function that execute the command
 * Two situations, `background` == false and `background` == true
 * If `background` == true, create another child process to execute the command
 * Excute the cd-series commands at the first place each time find a cd-series command
 */ 
void Shell::execute() 
{
    /* foreground execution */
    if (background == false) 
    {
        while (!vcmd.empty()) 
        {
            if (!exec_cd()) return;
            string command = rebuildCommand();
            if (!command.empty()) exec_command(command);
        } 
    }

    /* background execution */
    else 
    {
        pid_t pid;
        int status;
        pid = fork();

        /* child process */
        if (pid == 0) 
        {
            cmdstring.erase(cmdstring.find_last_not_of("*") + 1); // initial command string
            while (!vcmd.empty()) 
            {
                if (!exec_cd()) return;
                string command = rebuildCommand();
                if (!command.empty()) exec_command(command);
            }
            ofstream fout (fpath, std::ios::app); 
            fout << "[";
            fout << count;
            fout << "]";
            fout <<"+  Done\t\t";
            fout << cmdstring << endl;
            // fout << cmdstring; 
            exit(EXIT_FAILURE);
        }
        /* fork error */
        else if (pid < 0) 
        {
            perror(shell_name.data());
        }
        /* parent process */
        else 
        {
            out << "[" << count << "]" << "(" << pid << ")\n"; 
        }
    }
}

/**
 * Running the shell
 */ 
void Shell::run() 
{
    string dirName;
    int status;
    do 
    {
        dirName = getDirName();
        if (dirName == name) dirName = "~";
        ifstream fread (fpath);
        string line;
        while (getline(fread, line)) {
            out << line << endl;
        }
        ofstream fout (fpath);
        // output the prompt
        out << shell_name.data() << ":" << dirName << " " << name << "$ ";
        // read the command from the keyboard
        getline(cin, cmdstring);
        // split the command with ' '
        split(cmdstring, ' ');
        status = parseCommand();
    } 
    while (status);
}