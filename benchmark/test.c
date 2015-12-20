/************************************************
 * This si probably useless now as we use clock
 * which seems to report time spent on processor
 * by this process, not total time.
 ************************************************/

#define _POSIX_C_SOURCE 2

#include <stdio.h> //Standard headers
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h> //POSIX threads

#include <fcntl.h>
#include <sys/wait.h>

inline int test() {
    pid_t cpid, w;
    int status;
    char *const empty_env[] = { NULL };
    // int devnull;
    cpid = fork();
    if (cpid == 0) {
        // devnull = open("/dev/null", O_WRONLY);
        // dup2(devnull, 1);
        // dup2(devnull, 2);
        fclose(stdin);
        execve("/bin/true", (char * []) { NULL }, empty_env);
        perror("execve");
        exit(EXIT_FAILURE);
    }
    do {
        w = waitpid(cpid, &status, WUNTRACED | WCONTINUED);
        if (w == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    return WEXITSTATUS(status) == EXIT_SUCCESS;
}

#define ITERATIONS (100000)

int main() {
    clock_t t1, t2;

    t1 = clock();

    for (int i = 0; i < ITERATIONS; i++) {
        test();
    }

    t2 = clock();

    double delta = (double) (t2 - t1);
    double iteration_clocks = delta / (double) ITERATIONS;
    double iteration_seconds = iteration_clocks / (double) CLOCKS_PER_SEC;

    printf("%.10g sec/iteration\n", iteration_seconds);

    return 0;
}
