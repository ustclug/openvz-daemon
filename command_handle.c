#include <stdlib.h>
#include <stdio.h>
#include <json.h>
#define BUFFERSIZE 1024
int run_command_block(const char * command, json_object * data,
                      void (*cb)(const char * line, json_object * cb_data)){
    FILE * fp;
    fp = popen(command,"r");
    char line[BUFFERSIZE];
    if(fp == NULL)
        return 1;
    while(fgets(line,sizeof(line),fp)) {
        cb(line,data);
    }
    pclose(fp);
    return 0;
}
