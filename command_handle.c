#include <stdlib.h>
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
    while(fgets(line,sizeof(line),fp)) {
        cb(line,line_no,data);
        line_no++;
    }
    return pclose(fp);
}
