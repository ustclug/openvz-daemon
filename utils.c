#include <string.h>

char *
str_multi_cat(char * dest, const char ** src, size_t len, size_t n, char split) {
    size_t dest_len = strlen(dest);
    size_t src_i, src_j, dest_i = 0;
    for(src_i = 0; src_i < len; src_i++){
        for(src_j = 0; src[src_i][src_j]!='\0' && dest_i < n; src_j++) {
            dest[dest_len+dest_i] = src[src_i][src_j];
            dest_i ++;
        }
        if(src_i != len-1 && dest_i < n) {
            dest[dest_len + dest_i] = split;
            dest_i++;
        }
    }
    dest[dest_len+dest_i] = '\0';
    return dest;
}
