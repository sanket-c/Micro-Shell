#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/resource.h>
#include "parse.h"

extern char **environ; // for environment
char* hostname; // for hostname
int fd[50]; // for pipe
int isPipe = 0;
int cmd_number, fdIndex;

static void executeCmd(Cmd c);

static void executeEcho(Cmd c)
{
    int i = 0;
    int old_std_out = dup(fileno(stdout));
    int old_std_err = dup(fileno(stderr));
    int file = -1;

    if(c->out != Tnil)
    {
        if(c->out == Tpipe)
        {
            if(dup2(fd[(2 * (cmd_number-1)) + 1], fileno(stdout)) == -1)
            {
                perror("Error while output redirection ");
                return;
            }
        }
        else if(c->out == TpipeErr)
        {
            if(dup2(fd[(2 * (cmd_number-1)) + 1], fileno(stdout)) == -1 && dup2(fd[(2 * (cmd_number-1)) + 1], fileno(stderr)))
            {
                perror("Error while output redirection ");
                return;
            }
        }
        else if(c->out == Tout)
        {
            file = open(c->outfile, O_CREAT | O_TRUNC | O_WRONLY, 0660);
            if(file < 0)
            {
                perror("Could not open file ");
                return;
            }
            if(dup2(file, fileno(stdout)) == -1)
            {
                perror("Error while output redirection ");
                return;
            }
        }
        else if(c->out == Tapp)
        {
            file = open(c->outfile, O_CREAT | O_APPEND | O_WRONLY, 0660);
            if(file < 0)
            {
                perror("Could not open file ");
                return;
            }
            if(dup2(file, fileno(stdout)) == -1)
            {
                perror("Error while output redirection ");
                return;
            }
        }
        else if(c->out == ToutErr)
        {
            file = open(c->outfile, O_CREAT | O_TRUNC | O_WRONLY, 0660);
            if(file < 0)
            {
                perror("Could not open file ");
                return;
            }
            if(dup2(file, fileno(stdout)) == -1)
            {
                perror("Error while output redirection ");
                return;
            }
        }
        else if(c->out == TappErr)
        {
            file = open(c->outfile, O_CREAT | O_APPEND | O_WRONLY, 0660);
            if(file < 0)
            {
                perror("Could not open file ");
                return;
            }
            if(dup2(file, fileno(stdout)) == -1)
            {
                perror("Error while output redirection ");
                return;
            }
        }
    }

    if(c->args[1] == NULL)
    {
        printf("\n");
    }
    else if(c->args[1][0] == '$')
    {
        char* path = c->args[1];
        char* p = getenv(&path[1]);
        if(p != NULL)
        {
            printf("%s\n", p);
        }
        else
        {
            printf("\n");
        }
    }
    else
    {
        char *buffer = malloc(8092);
        for(i = 1; i < c->nargs; i++)
        {
            strcat(buffer, c->args[i]);
            if(i != c->nargs-1)
            {
                strcat(buffer, " ");
            }
        }
        puts(buffer);
    }
    fflush(stdout);
    dup2(old_std_out, fileno(stdout));
    dup2(old_std_err, fileno(stderr));
    if(file != -1)
        close(file);
    return;
}

static void executePwd(Cmd c)
{
    long size = pathconf(".", _PC_PATH_MAX);
    char* pw = (char *)malloc((size_t)size);
    getcwd(pw, (size_t)size);
    if(c->out == Tnil)
    {
        printf("%s\n", pw);
    }
    else if(c->out != Tnil)
    {
        int old_std_out = dup(fileno(stdout));
        int old_std_err = dup(fileno(stderr));
        int file = -1;
        if(c->out == Tpipe)
        {
            if(dup2(fd[(2 * (cmd_number-1)) + 1], fileno(stdout)) == -1)
            {
                perror("Error while output redirection ");
                return;
            }
        }
        else if(c->out == TpipeErr)
        {
            if(dup2(fd[(2 * (cmd_number-1)) + 1], fileno(stdout)) == -1 && dup2(fd[(2 * (cmd_number-1)) + 1], fileno(stderr)))
            {
                perror("Error while output redirection ");
                return;
            }
        }
        else if(c->out == Tout)
        {
            file = open(c->outfile, O_CREAT | O_TRUNC | O_WRONLY, 0660);
            if(file < 0)
            {
                perror("Could not open file ");
                return;
            }
            if(dup2(file, fileno(stdout)) == -1)
            {
                perror("Error while output redirection ");
                return;
            }
        }
        else if(c->out == Tapp)
        {
            file = open(c->outfile, O_CREAT | O_APPEND | O_WRONLY, 0660);
            if(file < 0)
            {
                perror("Could not open file ");
                return;
            }
            if(dup2(file, fileno(stdout)) == -1)
            {
                perror("Error while output redirection ");
                return;
            }
        }
        else if(c->out == ToutErr)
        {
            file = open(c->outfile, O_CREAT | O_TRUNC | O_WRONLY, 0660);
            if(file < 0)
            {
                perror("Could not open file ");
                return;
            }
            if(dup2(file, fileno(stdout)) == -1)
            {
                perror("Error while output redirection ");
                return;
            }
        }
        else if(c->out == TappErr)
        {
            file = open(c->outfile, O_CREAT | O_APPEND | O_WRONLY, 0660);
            if(file < 0)
            {
                perror("Could not open file ");
                return;
            }
            if(dup2(file, fileno(stdout)) == -1)
            {
                perror("Error while output redirection ");
                return;
            }
        }
        puts(pw);
        fflush(stdout);
        dup2(old_std_out, fileno(stdout));
        dup2(old_std_err, fileno(stderr));
        close(file);
    }
    return;
}

static void executeCd(Cmd c)
{
    if(c->args[1] == NULL || strcmp(c->args[1],"~") == 0)
    {
        char* dir = "HOME";
        char* directory = malloc(sizeof(char*) * strlen(getenv(dir)));
        strcpy(directory,getenv(dir));
        chdir(directory);
        /*if(gethostname(hostname, 1023) < 0)
        {
            hostname = "myShell";
        }*/
    }
    else
    {
        if(chdir(c->args[1]) != -1)
        {
            /*if(gethostname(hostname, 1023) < 0)
            {
                hostname = "myShell";
            }
            long size = pathconf(".", _PC_PATH_MAX);
            char* pw = (char *)malloc((size_t)size);
            getcwd(pw, (size_t)size);
            char* dir = "HOME";
            char* directory = malloc(sizeof(char*) * strlen(getenv(dir)));
            strcpy(directory,getenv(dir));
            if(strcmp(pw, directory) != 0)
            {
                strcat(hostname, ":~/");
                char* new_hostname = strstr(strstr(strstr(pw, "/")+1,"/")+1, "/")+1;
                strcat(hostname, new_hostname);
            }*/
        }
        else
        {
            printf("cd: %s: No such file or directory\n",c->args[1]);
        }
    }
    return;
}

static void executeSetEnv(Cmd c)
{
    char **env = environ;
    if(c->args[1] == NULL)
    {
        int old_std_out = dup(fileno(stdout));
        int old_std_err = dup(fileno(stderr));
        int file = -1;

        if(c->out != Tnil)
        {
            if(c->out == Tpipe)
            {
                if(dup2(fd[(2 * (cmd_number-1)) + 1], fileno(stdout)) == -1)
                {
                    perror("Error while output redirection ");
                    return;
                }
            }
            else if(c->out == TpipeErr)
            {
                if(dup2(fd[(2 * (cmd_number-1)) + 1], fileno(stdout)) == -1 && dup2(fd[(2 * (cmd_number-1)) + 1], fileno(stderr)))
                {
                    perror("Error while output redirection ");
                    return;
                }
            }
            else if(c->out == Tout)
            {
                file = open(c->outfile, O_CREAT | O_TRUNC | O_WRONLY, 0660);
                if(file < 0)
                {
                    perror("Could not open file ");
                    return;
                }
                if(dup2(file, fileno(stdout)) == -1)
                {
                    perror("Error while output redirection ");
                    return;
                }
            }
            else if(c->out == Tapp)
            {
                file = open(c->outfile, O_CREAT | O_APPEND | O_WRONLY, 0660);
                if(file < 0)
                {
                    perror("Could not open file ");
                    return;
                }
                if(dup2(file, fileno(stdout)) == -1)
                {
                    perror("Error while output redirection ");
                    return;
                }
            }
            else if(c->out == ToutErr)
            {
                file = open(c->outfile, O_CREAT | O_TRUNC | O_WRONLY, 0660);
                if(file < 0)
                {
                    perror("Could not open file ");
                    return;
                }
                if(dup2(file, fileno(stdout)) == -1)
                {
                    perror("Error while output redirection ");
                    return;
                }
            }
            else if(c->out == TappErr)
            {
                file = open(c->outfile, O_CREAT | O_APPEND | O_WRONLY, 0660);
                if(file < 0)
                {
                    perror("Could not open file ");
                    return;
                }
                if(dup2(file, fileno(stdout)) == -1)
                {
                    perror("Error while output redirection ");
                    return;
                }
            }
        }

        while(*env != NULL)
        {
            printf("%s\n", *env);
            env++;
        }

        fflush(stdout);
        dup2(old_std_out, fileno(stdout));
        dup2(old_std_err, fileno(stderr));
        if(file != -1)
            close(file);
    }
    else if(c->args[2] == NULL)
    {
        setenv(c->args[1], "", 1);
    }
    else
    {
        setenv(c->args[1], c->args[2], 1);
    }
}

static void executeUnsetEnv(Cmd c)
{
    if(c->args[1] == NULL)
    {
        printf("Environment Variable not specified.\n");
    }
    else
    {
        unsetenv(c->args[1]);
    }
}

static void executeWhere(Cmd c)
{
    if(c->args[1] != NULL)
    {
        char* p = getenv("PATH");
        char paths[100];
        strcpy(paths, p);
        char* path_tokens = malloc(500);
        char delims[] = ":";
        char *path = malloc(100);
        path_tokens = strtok(paths, delims);
        struct stat st;
        char* cmd = malloc(100);
        strcpy(cmd, "/");
        strcpy(cmd, c->args[1]);
        int pathFound = 0;
        while(path_tokens != NULL)
        {
            strcpy(path, path_tokens);
            strcat(path, "/");
            strcat(path, cmd);
            if(stat(path, &st) == 0)
            {
                pathFound = 1;
                break;
            }
            path_tokens = strtok( NULL, delims );
        }

        int old_std_out = dup(fileno(stdout));
        int old_std_err = dup(fileno(stderr));
        int file = -1;

        if(c->out != Tnil)
        {
            if(c->out == Tpipe)
            {
                if(dup2(fd[(2 * (cmd_number-1)) + 1], fileno(stdout)) == -1)
                {
                    perror("Error while output redirection ");
                    return;
                }
            }
            else if(c->out == TpipeErr)
            {
                if(dup2(fd[(2 * (cmd_number-1)) + 1], fileno(stdout)) == -1 && dup2(fd[(2 * (cmd_number-1)) + 1], fileno(stderr)))
                {
                    perror("Error while output redirection ");
                    return;
                }
            }
            else if(c->out == Tout)
            {
                file = open(c->outfile, O_CREAT | O_TRUNC | O_WRONLY, 0660);
                if(file < 0)
                {
                    perror("Could not open file ");
                    return;
                }
                if(dup2(file, fileno(stdout)) == -1)
                {
                    perror("Error while output redirection ");
                    return;
                }
            }
            else if(c->out == Tapp)
            {
                file = open(c->outfile, O_CREAT | O_APPEND | O_WRONLY, 0660);
                if(file < 0)
                {
                    perror("Could not open file ");
                    return;
                }
                if(dup2(file, fileno(stdout)) == -1)
                {
                    perror("Error while output redirection ");
                    return;
                }
            }
            else if(c->out == ToutErr)
            {
                file = open(c->outfile, O_CREAT | O_TRUNC | O_WRONLY, 0660);
                if(file < 0)
                {
                    perror("Could not open file ");
                    return;
                }
                if(dup2(file, fileno(stdout)) == -1)
                {
                    perror("Error while output redirection ");
                    return;
                }
            }
            else if(c->out == TappErr)
            {
                file = open(c->outfile, O_CREAT | O_APPEND | O_WRONLY, 0660);
                if(file < 0)
                {
                    perror("Could not open file ");
                    return;
                }
                if(dup2(file, fileno(stdout)) == -1)
                {
                    perror("Error while output redirection ");
                    return;
                }
            }
        }

        if(pathFound)
            printf("%s : %s : Executable \n", c->args[1], path);
        if(strcmp(c->args[1],"cd")==0)
            printf("%s : Built-in Command\n", c->args[1]);
        else if(strcmp(c->args[1],"echo")==0)
            printf("%s : Built-in Command\n", c->args[1]);
        else if(strcmp(c->args[1],"setenv")==0)
            printf("%s : Built-in Command\n", c->args[1]);
        else if(strcmp(c->args[1],"unsetenv")==0)
            printf("%s : Built-in Command\n", c->args[1]);
        else if(strcmp(c->args[1],"pwd")==0)
            printf("%s : Built-in Command\n", c->args[1]);
        else if(strcmp(c->args[1],"where")==0)
            printf("%s : Built-in Command\n", c->args[1]);
        else if(strcmp(c->args[1],"logout")==0)
            printf("%s : Built-in Command\n", c->args[1]);
        else if(strcmp(c->args[1],"nice")==0)
            printf("%s : Built-in Command\n", c->args[1]);
        else if(!pathFound)
            printf("%s : Command not found\n", c->args[1]);

        fflush(stdout);
        dup2(old_std_out, fileno(stdout));
        dup2(old_std_err, fileno(stderr));
        if(file != -1)
            close(file);
    }
    else
    {
        printf("where [command]\n");
    }
}

static void executeNice(Cmd c)
{
    int which = PRIO_PROCESS;
    int who = getpid();
    int ret_value;
    pid_t pid;
    int priority;
    if(c->nargs == 1)
    {
        printf("syntax: nice [-|+ number] [command]\n");
        return;
    }
    else if(c->nargs == 2)
    {
        priority = atoi(c->args[1]);
        if(priority == 0 && strcmp(c->args[1],"0") != 0 && strcmp(c->args[1],"+0") != 0 && strcmp(c->args[1],"-0") != 0)
        {
            priority = 4;
            int i = 1;
            for(i ; i < c->nargs; i++)
            {
                c->args[i-1] = c->args[i];
            }
            c->args[i-1] =  NULL;
            c->nargs = c->nargs - 1;
        }
        else
        {
            printf("syntax: nice [-|+ number] [command]\n");
            return;
        }
    }
    else if(c->nargs > 2)
    {
        priority = atoi(c->args[1]);
        if(priority == 0 && strcmp(c->args[1],"0") != 0 && strcmp(c->args[1],"+0") != 0 && strcmp(c->args[1],"-0") != 0)
        {
            priority = 4;
            int i = 1;
            for(i ; i < c->nargs; i++)
            {
                c->args[i-1] = c->args[i];
            }
            c->args[i-1] =  NULL;
            c->nargs = c->nargs - 1;
        }
        else
        {
            if(priority < -20 || priority > 19)
            {
                priority = 4;
            }
            int i = 2;
            for(i ; i < c->nargs; i++)
            {
                c->args[i-2] = c->args[i];
            }
            c->args[i-2] =  NULL;
            c->args[i-1] =  NULL;
            c->nargs = c->nargs - 2;
        }
    }
    getpriority(which, who);
    ret_value = setpriority(which, who, priority);
    if(ret_value >= 0)
    {
        pid = fork();
        if(pid < 0)
        {
            perror("Fork Error ");
        }
        else if(pid == 0)
        {
            executeCmd(c);
            exit(0);
        }
        else
        {
            wait();
        }
    }
}

static void executeExternalCommand(Cmd c)
{
    int old_std_in;
    int inputfile;
    pid_t pid = fork();
    if(pid < 0)
    {
        perror("Fork Error ");
    }
    else if(pid == 0)
    {
        int old_std_out = dup(fileno(stdout));
        int old_std_err = dup(fileno(stderr));
        int file = -1;
        if(c->out != Tnil)
        {
            if(c->out == Tpipe)
            {
                if(dup2(fd[(2 * (cmd_number-1)) + 1], fileno(stdout)) == -1)
                {
                    perror("Error while output redirection ");
                    return;
                }
            }
            else if(c->out == TpipeErr)
            {
                if(dup2(fd[(2 * (cmd_number-1)) + 1], fileno(stdout)) == -1 && dup2(fd[(2 * (cmd_number-1)) + 1], fileno(stderr)))
                {
                    perror("Error while output redirection ");
                    return;
                }
            }
            else if(c->out == Tout)
            {
                file = open(c->outfile, O_CREAT | O_TRUNC | O_WRONLY, 0660);
                if(file < 0)
                {
                    perror("Could not open file ");
                    return;
                }
                if(dup2(file, fileno(stdout)) == -1)
                {
                    perror("Error while output redirection ");
                    return;
                }
            }
            else if(c->out == Tapp)
            {
                file = open(c->outfile, O_CREAT | O_APPEND | O_WRONLY, 0660);
                if(file < 0)
                {
                    perror("Could not open file ");
                    return;
                }
                if(dup2(file, fileno(stdout)) == -1)
                {
                    perror("Error while output redirection ");
                    return;
                }
            }
            else if(c->out == ToutErr)
            {
                file = open(c->outfile, O_CREAT | O_TRUNC | O_WRONLY, 0660);
                if(file < 0)
                {
                    perror("Could not open file ");
                    return;
                }
                if(dup2(file, fileno(stdout)) == -1)
                {
                    perror("Error while output redirection ");
                    return;
                }
            }
            else if(c->out == TappErr)
            {
                file = open(c->outfile, O_CREAT | O_APPEND | O_WRONLY, 0660);
                if(file < 0)
                {
                    perror("Could not open file ");
                    return;
                }
                if(dup2(file, fileno(stdout)) == -1)
                {
                    perror("Error while output redirection ");
                    return;
                }
            }
        }

        if(c->in == Tin)
        {
            old_std_in = dup(STDIN_FILENO);
            inputfile = open(c->infile, O_RDONLY, 0660);
            if(inputfile < 0)
            {
                perror("Could not open file ");
                exit(0);
            }
            if(dup2(inputfile, STDIN_FILENO) < 0)
            {
                perror("Error while input redirection ");
                exit(0);
            }
            if(c->args[0] != NULL)
            {
                if(execvp(c->args[0], c->args) == -1)
                {
                    perror("Command not found ");
                }
            }
            dup2(old_std_in, STDIN_FILENO);
            close(inputfile);
        }
        else if(c->in == Tpipe)
        {
            old_std_in = dup(STDIN_FILENO);
            if(dup2(fd[(2 * (cmd_number-1))-2], STDIN_FILENO) == -1)
            {
                perror("Error while input redirection ");
                exit(0);
            }
            if(c->args[0] != NULL)
            {
                if(execvp(c->args[0], c->args) == -1)
                {
                    perror("Command not found ");
                }
            }
            close(fd[(2 * (cmd_number-1))-2]);
        }
        else if(c->in == Tnil)
        {
            if(c->args[0] != NULL)
            {
                if(execvp(c->args[0], c->args) == -1)
                {
                    perror("Command not found ");
                }
            }
        }
        dup2(old_std_out, fileno(stdout));
        dup2(old_std_err, fileno(stderr));
        if(file != -1)
            close(file);
        exit(0);
    }
    else
    {
        wait();
    }
}

static void executeCmd(Cmd c)
{
    int i;

    if ( c )
    {
        if ( !strcmp(c->args[0], "exit") || !strcmp(c->args[0], "end") || !strcmp(c->args[0], "logout"))
        {
            exit(0);
        }
        else
        {
            if(strcmp(c->args[0], "echo") == 0)
                executeEcho(c);
            else if(strcmp(c->args[0], "pwd") == 0)
                executePwd(c);
            else if(strcmp(c->args[0], "cd") == 0)
                executeCd(c);
            else if(strcmp(c->args[0], "setenv") == 0)
                executeSetEnv(c);
            else if(strcmp(c->args[0], "unsetenv") == 0)
                executeUnsetEnv(c);
            else if(strcmp(c->args[0], "where") == 0)
                executeWhere(c);
            else if(strcmp(c->args[0], "nice") == 0)
                executeNice(c);
            else
                executeExternalCommand(c);
        }
    }
}

static void prPipe(Pipe p)
{
    int i = 0;
    Cmd c;

    cmd_number = 1;

    if ( p == NULL )
        return;

    for ( c = p->head; c != NULL; c = c->next )
    {
        fdIndex = 2 * (cmd_number - 1);
        if(c->out == Tpipe)
        {
            pipe(fd+fdIndex);
            isPipe = 1;
        }
        executeCmd(c);
        cmd_number++;
        if(isPipe == 1)
        {
            close(fd[fdIndex + 1]);
            isPipe = 0;
        }
    }

    prPipe(p->next);
}

void readUshrc(Pipe p)
{
    char* dir = "HOME";
    char* directory = malloc(sizeof(char*) * strlen(getenv(dir)));
    strcpy(directory, getenv(dir));
    strcat(directory, "/.ushrc");
    int rcFile = open(directory, O_RDONLY, 0600);

    if(rcFile < 0)
    {
        perror("Could not open ushrc ");
    }
    else
    {
        int old_std_in = dup(STDIN_FILENO);

        if(dup2(rcFile, STDIN_FILENO) < 0)
        {
            perror("Could not read ushrc\n");
        }
        else
        {
            p = parse();
            if(p != NULL)
            {
                while(strcmp(p->head->args[0], "end") && strcmp(p->head->args[0], "exit"))
                {
                    prPipe(p);
                    freePipe(p);
                    p = parse(); //parse next command
                    if(p == NULL) // no more commands
                        break;
                }
                if(dup2(old_std_in, STDIN_FILENO) < 0)
                {
                    perror("Failed to restore STD_IN ");
                    exit(0);
                }
                close(rcFile);
            }
        }
    }
}

void SIGQUITHandler(int sig)
{
    printf("\n");
    exit(0);
}

void SIGINTHandler(int sig)
{
    printf("\n");
}

int main()
{
    signal(SIGQUIT, SIGQUITHandler);
    signal(SIGINT,  SIGINTHandler);

    Pipe p;
    hostname = malloc(sizeof(char*) * 500);
    if(gethostname(hostname, 1023) < 0)
    {
        hostname = "myShell";
    }

    readUshrc(p);

    /*char* dir = "HOME";
    char* directory = malloc(sizeof(char*) * strlen(getenv(dir)));
    strcpy(directory,getenv(dir));
    chdir(directory);*/

    while ( 1 )
    {
        if(isatty(STDIN_FILENO))
            printf("%s%% ", hostname);
        fflush(stdout);
        p = parse();
        prPipe(p);
        freePipe(p);
    }
    return 0;
}

