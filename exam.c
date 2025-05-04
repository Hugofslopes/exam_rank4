#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>


void fatal() {
    write(2, "error: fatal\n", 12);
    exit(1);
}

// Split argv into commands separated by '|' and execute pipeline
void run_pipeline(char **argv, char **envp) {
    int i = 1; // start after program name
    int prev_fd = -1; // input for current command
    int pipefd[2];
    pid_t pid;

    while (argv[i]) {
        // Find next pipe or end
        int j = i;
        while (argv[j] && strcmp(argv[j], "|") != 0)
            j++;
        // Prepare command args array from argv[i..j-1]
        int cmd_argc = j - i;
        char **cmd_argv = malloc(sizeof(char *) * (cmd_argc + 1));
        if (!cmd_argv) fatal();
        for (int k = 0; k < cmd_argc; k++)
            cmd_argv[k] = argv[i + k];
        cmd_argv[cmd_argc] = NULL;
        if (argv[j]) { // there is another command after pipe
            if (pipe(pipefd) == -1) fatal();
        }
        pid = fork();
        if (pid == -1) fatal();
        if (pid == 0) {
            // Child process
            if (prev_fd != -1) {
                dup2(prev_fd, 0);
                close(prev_fd);
            }
            if (argv[j]) {
                close(pipefd[0]);
                dup2(pipefd[1], 1);
                close(pipefd[1]);
            }
            execve(cmd_argv[0], cmd_argv, envp);
            perror("error: cannot execute");
            exit(1);
        } else {
            // Parent process
            if (prev_fd != -1)
                close(prev_fd);
            if (argv[j]) {
                close(pipefd[1]);
                prev_fd = pipefd[0];
            }
            free(cmd_argv);
            i = j + (argv[j] ? 1 : 0);
        }
    }
    if (prev_fd != -1)
        close(prev_fd);
    while (wait(NULL) > 0)
        ;

}

int main(int argc, char **argv, char **envp) {

    if (argc < 2) {
        write(2, "Usage: ./microshell command [| command ...]\n", 43);
        return 1;
    }
    run_pipeline(argv, envp);
    return 0;
}