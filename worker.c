#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define PATH_MAX 1024
// Thinking about creating binarys files that are executed here but idk how should i pass the other params needed in the scheduler
// Not sure if is the best way but looks funny to implement
// Yeah it was funny

int path_exists(const char *path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

int is_executable(const char *path) {
    if (!path_exists(path)) return 0;
    return (access(path, X_OK) == 0);
}

int execute_binary_exec(const char *path, char *const argv[]) {
    if (!is_executable(path)) {
        fprintf(stderr, "Error: '%s' doesn't exist or isn't executable\n", path);
        return -1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return -1;
    }

    if (pid == 0) {
        execv(path, argv);
        perror("execv failed");
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return -1;
        }
    }
}

void worker(int taskId, char taskExecution[64]){
    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %s\n", cwd);
    } else {
        perror("getcwd() error");
    }

    size_t total_len = strlen(cwd) + strlen("/dags/") + strlen(taskExecution) + 1;
    char *full_path = malloc(total_len);

    strcpy(full_path, cwd); strcat(full_path, "/dags/"); strcat(full_path, taskExecution);

    printf("Full path for binary execution %s\n", full_path);

    execute_binary_exec(full_path, NULL);
    return;
}
