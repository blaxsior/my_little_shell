#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[])
{
    char *arguments1[] = {"ls", NULL};
    char *arguments2[] = {"grep", "test*", NULL};

    int my_pipe[2];
    if (pipe(my_pipe) == -1)
    {
        fprintf(stderr, "Error creating pipe\n");
    }

    pid_t child_id;
    child_id = fork();
    if (child_id == -1)
    {
        fprintf(stderr, "Fork error\n");
    }
    if (child_id == 0) // child process
    {
        close(my_pipe[0]);   // child doesn't read
        dup2(my_pipe[1], 1); // redirect stdout
        close(my_pipe[1]);

        execvp(arguments1[0], arguments1);

        fprintf(stderr, "Exec failed\n");
    }
    else
    {
        close(my_pipe[1]); // parent doesn't write
        dup2(my_pipe[0], 0);
        close(my_pipe[0]);

        char reading_buf[1];
        printf("onServer\n");
        wait(NULL);

        execvp(arguments2[0], arguments2);
        close(my_pipe[0]);
    }
}