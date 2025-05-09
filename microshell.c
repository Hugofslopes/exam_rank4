#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define ERRCD "error: cd: cannot change directory to "
#define ARGCD "error: cd: bad arguments\n"
#define FATAL "error: fatal\n"
#define ERREXE "error: cannot execute "

void put_str(char *str)
{
    for (size_t i = 0; str[i]; i++)
        write(2, &(str)[i], 1);
}

int cd(char **av, size_t i)
{
    if (i != 2)
        return (put_str(ARGCD), 1);
    if (chdir(av[1]) == -1)
        return (put_str(ERRCD), put_str(av[1]), put_str("\n"), 1);
    return (0);
}

void dups(int h_p, int *fd, int v)
{
    if (h_p && (dup2(fd[v], v) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
        put_str(FATAL), exit (1);
}

int execute(char **av, char **envp, size_t i)
{
    int h_p = (av[i] && !strcmp(av[i],"|"));
    int fd[2], pid, status;
    if (!h_p && !strcmp(*av,"cd" ))
        return (cd(av, i));
    if (h_p && pipe(fd) == -1)
        put_str(FATAL), exit (1);
    if ((pid =fork()) == -1)
        put_str(FATAL), exit (1);
    if (!pid)
    {
        av[i] = 0;
        dups(h_p, fd, 1);
        if (execve(*av, av, envp) == -1)
        put_str(ERREXE), put_str(*av), put_str("\n"), exit (1);
    }
    waitpid(pid, &status, 0);
    dups(h_p, fd, 0);
    return(WIFEXITED(status) && WEXITSTATUS(status));
}

int main(int ac, char **av, char **envp)
{
    (void)ac;
    int status = 0;
    size_t i = 0;

    while (av[i])
    {
        av += i + 1;
        i = 0;
        while (av[i] && strcmp(av[i],";") && strcmp(av[i],"|"))
            i++;
        if (i)
            status = execute(av, envp, i);
    }
    return (status);
}