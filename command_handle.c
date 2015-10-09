#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <json.h>
#define BUFFERSIZE 1024
int run_command_block(const char * command, json_object * data,
                      void (*cb)(const char * line, int line_no, json_object * cb_data)){
    FILE * fp;
    fp = popen(command,"r");
    char line[BUFFERSIZE];
    if(fp == NULL)
        return 1;
    int line_no = 0;
    if (NULL != cb) {
        while(fgets(line,sizeof(line),fp)) {
            cb(line,line_no,data);
            line_no++;
        }
    }
    return pclose(fp);
}

/**
 * @param execute
 * @param args normally args[0] is same to execute and the last element should be NULL.
 */
int run_command_nonblock(const char * execute, char * const args[]) {
    printf("We are in run_command_nonblock\n");
    int status = 0;
    int error = 0;

    pid_t pid1 = fork();

    if (pid1 > 0) {
        /* parent process A */
        waitpid(pid1, &status, NULL);
    } else if (0 == pid1) {
        /* child process B */
        pid_t pid2 = fork();
        printf("pid2: %d\n",pid2);
        if (pid2 > 0) {
            exit(0);
        } else if (0 == pid2) {
            /* child process C */
            execvp(execute, args);
        } else {
            exit(1);
        }
    } else {
        error = 1;
    }
    printf("pid1: %d\n",pid1);
    return error || status;
}