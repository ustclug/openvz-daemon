#include <stdio.h>
#include <string.h>
#include <json.h>

extern int run_command_block(const char * command, json_object * data,
                             void (*cb)(const char * line, json_object * cb_data));

void list_process(const char * line, json_object * cb_data);
void status_process(const char * line, json_object * cb_data);

json_object *
process_get(const char * url) {
    //printf("url:%s\n",url);
    /**
     * GET /vz
     * return a json_list of all containers
     */
    if(0 == strncmp(url, "/vz", strlen("/vz"))) {
        const char * url_end = url + strlen("/vz");
        if('\0' == *url_end || ('/' == *url_end && '\0' == *(url_end+1))) {
            json_object *data;
            data = json_object_new_array();
            run_command_block("vzlist -o ctid -a -H;", data, list_process);
            return data;
        }
    }
    /**
     * GET /vz/id
     * return the vz's info
     */
    if(0 == strncmp(url, "/vz/", strlen("/vz/"))) {
        int id;
        const char * url_id = url + strlen("/vz/");
        if(NULL == url_id || *url_id < '0' || *url_id > '9')
            return NULL;
        sscanf(url + strlen("/vz/"),"%d", &id);
        return NULL;

    }
    return NULL;
}


/**
 * data process
 */

void
list_process(const char * line, json_object * cb_data) {
    int ctid;
    sscanf(line,"%d",&ctid);
    if(json_object_is_type(cb_data,json_type_array)) {
        json_object * ctid_json = json_object_new_int(ctid);
        json_object_array_add(cb_data,ctid_json);
    }
}

void
status_process(const char * line, json_object * cb_data) {

}