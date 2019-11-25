#include "myshell.h"

vector<string> Shell::done_commands;

void* Shell::receive_info(void* __this) {
    Shell * _this =(Shell *)__this;
    int new_s;
    int len;
    char buf[MAX_LINE];
    while (1)
    {
        if ((new_s=accept(_this->s, (struct sockaddr *)&_this->sin, (socklen_t *)&len)) < 0)
        {
            perror("accept error");
            return 0;
        }
        while ((len=recv(new_s, buf, sizeof(buf),0))>0)
        {
            _this->done_commands.push_back(buf);
        }
        close(new_s);
    }
    return 0;
}

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
    done_commands.clear();
	bzero((void *)&sin, sizeof(sin));
	sin.sin_family=AF_INET;
	sin.sin_addr.s_addr=INADDR_ANY;
	sin.sin_port=htons(SERVER_PORT);

	if ((s=socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(-1);
	}
	if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		perror("bind");
		exit(-1);
	}
	listen(s, MAX_PENDING);
    pthread_t tid;
    pthread_create(&tid, NULL, receive_info, (void*)this);
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
    name = vcmd[2];
    // cd to the user's default path
    if (chdir(("/" + vcmd[1] + "/" + name).c_str()) != 0) 
    {
        perror("ERROR");
        EXIT_FAILURE;
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
            struct hostent *hp;
            struct sockaddr_in sin;
            char buf[MAX_LINE];
            int len;
            int s;

            hp=gethostbyname("localhost");
            bzero((void *)&sin, sizeof(sin));
            sin.sin_family=AF_INET;
            bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
            sin.sin_port=htons(SERVER_PORT);

            if ((s=socket(PF_INET, SOCK_STREAM, 0)) < 0) {
                perror("socket");
                return;
            }
            if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
                perror("connect");
                close(s);
                return;
            }
            sprintf(buf, "[%d]+  Done\t\t%s", count, cmdstring.c_str());
            buf[MAX_LINE-1]='\0';
            len=strlen(buf)+1;
            send(s, buf, len, 0);
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
        for (int i = 0; i < done_commands.size(); ++i)
        {
            out << done_commands[i] << endl;
        }
        done_commands.clear();
        out << shell_name.data() << ":" << dirName << " " << name << "$ "; // output the prompt
        getline(cin, cmdstring); // read the command from the keyboard
        split(cmdstring, ' '); // split the command with ' '
        status = parseCommand();
    } 
    while (status);
}