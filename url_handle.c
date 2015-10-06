#include <stdio.h>
#include <string.h>
#include <json.h>

extern int run_command_block(const char * command, json_object * data,
                             void (*cb)(const char * line, json_object * cb_data));

void status_process(const char * line, json_object * data);

int
process_get(const char * url) {
    //printf("url:%s\n",url);
    /**
     * /list
     */
    if(0 == strncmp(url, "/list", strlen("/list"))) {
        json_object * data;
        data = json_object_new_object();
        run_command_block("vzlist -o ctid -a -H;", data, status_process);
    }
    /**
     * /status/id
     */
    if(0 == strncmp(url, "/status", strlen("/status"))) {

    }
    return 0;
}


/**
 * data process
 */

void
status_process(const char * line, json_object * cb_data) {

}