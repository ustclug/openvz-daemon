#include <stdio.h>
#include <string.h>
#include <json.h>

#define BUFFERSIZE 1024

extern int run_command_block(const char * command, json_object * data,
                             void (*cb)(const char * line, int line_no, json_object * cb_data));
extern int run_command_nonblock(const char * execute, char * const args[], pid_t * pid_task);
extern char * str_multi_cat(char * dest, const char ** src, size_t len, size_t n, char split);


void list_process(const char * line, int line_no, json_object * cb_data);
void info_process(const char *line, int line_no, json_object *cb_data);

static const char * config_attrs[] = {
        "ctid",
        "hostname",
        "diskspace.s",
        "diskspace.h",
        "cpulimit",
        "cpuunits",
        "onboot",
        "bootorder"
};
static const char * status_attrs[] = {
        "status",
        "kmemsize",
        "lockedpages",
        "privvmpages",
        "shmpages",
        "numproc",
        "physpages",
        "numtcpsock",
        "numflock",
        "numpty",
        "numsiginfo",
        "tcpsndbuf",
        "tcprcvbuf",
        "othersockbuf",
        "dgramrcvbuf",
        "dcachesize",
        "numfile",
        "swappages",
        "diskspace",
        "diskinodes",
};

/**
 * All GET method is handle here.
 *
 * @param return_obj all result is stored to return_obj.
 */
void
process_get(const char * url, json_object * return_obj) {
    //printf("url:%s\n",url);
    /**
     * GET /vz
     * return a json_list of all containers
     */
    if(0 == strncmp(url, "/vz", strlen("/vz"))) {
        const char * url_end = url + strlen("/vz");
        if('\0' == *url_end || ('/' == *url_end && '\0' == *(url_end+1))) {
            json_object * list_obj = json_object_new_array();
            if(NULL == list_obj)
                return;
            run_command_block("vzlist -o ctid -a -H;", list_obj, list_process);
            json_object_object_add(return_obj, "list", list_obj);
            return;
        }
    }
    /**
     * GET /vz/id
     * return the vz's info
     */
    if(0 == strncmp(url, "/vz/", strlen("/vz/"))) {
        int id;
        const char * url_id = url + strlen("/vz/");
        if(NULL == url_id || *url_id < '0' || *url_id > '9') {
            json_object_object_add(return_obj,"error", json_object_new_string("Can't get vz id!"));
        }
        sscanf(url + strlen("/vz/"),"%d", &id);

        char config_cmd_buffer[BUFFERSIZE] = {'\0'};
        char status_cmd_buffer[BUFFERSIZE] = {'\0'};
        char cmd_buffer[BUFFERSIZE] = {'\0'};
        str_multi_cat(config_cmd_buffer, config_attrs,
                      sizeof(config_attrs)/sizeof(const char *), BUFFERSIZE-1, ',');
        str_multi_cat(status_cmd_buffer, status_attrs,
                      sizeof(status_attrs)/sizeof(const char *), BUFFERSIZE-1, ',');

        snprintf(cmd_buffer, BUFFERSIZE, "vzlist -o %s,%s %d -H 2>&1;",
                 config_cmd_buffer,status_cmd_buffer, id);
        json_object * info_obj = json_object_new_object();
        if (NULL == info_obj)
            return;
        run_command_block(cmd_buffer, info_obj, info_process);
        json_object_object_add(return_obj, "info", info_obj);
        return;
    }
    json_object_object_add(return_obj,"error", json_object_new_string("URL not validated!"));
}


/**
 * data process
 */

void
list_process(const char * line, int line_no, json_object * cb_data) {
    int ctid;
    sscanf(line,"%d",&ctid);
    if(json_object_is_type(cb_data,json_type_array)) {
        json_object * ctid_json = json_object_new_int(ctid);
        json_object_array_add(cb_data,ctid_json);
    }
}

void
info_process(const char *line, int line_no, json_object *cb_data) {
    int n = 0;
    char * pch;
    char buffer[BUFFERSIZE];
    strncpy(buffer, line, BUFFERSIZE);
    pch = strtok(buffer, " \n");
    while(pch != NULL) {
        if(n < sizeof(config_attrs)/sizeof(const char *)) {
            json_object_object_add(cb_data, config_attrs[n], json_object_new_string(pch));
        } else if (n -sizeof(config_attrs)/sizeof(const char *) <
                sizeof(status_attrs)/sizeof(const char *)){
            json_object_object_add(cb_data, status_attrs[n-sizeof(config_attrs)/sizeof(const char *)],
                                   json_object_new_string(pch));
        } else {
            // Something Wrong
        }
        pch = strtok(NULL, " \n");
        n++;
    }
}