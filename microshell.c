#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

void put_str(char *str);

#define FATAL put_str("error: fatal\n")
#define BADCD put_str("error: cd: bad arguments\n")
#define CDFAIL put_str("error: cd: cannot change directory to ")
#define EXECFAIL put_str("error: cannot execute ")

void put_str(char *str)
{
	size_t	i = 0;

	while (str[i])
		write (2, &(str)[i++], 1);
}

int	cd(char **av, size_t i)
{
	if (i != 2)
		return (BADCD, 1);
	if (chdir(av[1]) == -1)
		return (CDFAIL, put_str(av[1]), 1);
	return (0);
}
void ft_pipe(int has_pipe, int *fd, int end) {
	if (has_pipe &&
		(dup2(fd[end], end) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
		FATAL, exit(1);
}

int execute(char **av, char **envp, size_t i) 
{
	int has_pipe = !strcmp(av[i], "|") ? 0 : i;
	int fd[2], status, pid;

	if (!has_pipe && !strcmp(*av, "cd"))
		return (cd(av, i));
	if (has_pipe && pipe(fd) == -1)
		FATAL, exit(1);
	if ((pid = fork()) == -1)
		FATAL, exit(1);
	if (!pid) 
    { 
		av[i] = 0;
		ft_pipe(has_pipe, fd, 1);
		if (execve(*av, av, envp) == -1)
            EXECFAIL, put_str(*av), write(2, "\n", 1), exit(1);
	}
	waitpid(pid, &status, 0);
	ft_pipe(has_pipe, fd, 0);
	return (WIFEXITED(status) && WEXITSTATUS(status));
}

int main(int argc, char **av, char **envp) {
	int status;
    size_t i = 0;

	if (argc == 1)
        return (1);
	while (av[i]) 
    {
		av += (i + 1);
		i = 0;
		while (av[i] && strcmp(av[i], ";") && strcmp(av[i], "|"))
			i++;
		status = execute(av, envp, i);
	}
	return (status);
}