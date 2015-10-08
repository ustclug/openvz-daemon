#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>
#include <json.h>

#define PORT 8888

#define SERVERKEYFILE "server.key"
#define SERVERCERTFILE "server.pem"
#define ROOTCACERTFILE "ca.pem"
#define CLIENTCN "control.freeshell.ustc.edu.cn"

#define HOSTNAMELEN 100
#define BUFFERSIZE 10240
#define POSTBUFFERSIZE  512

extern void process_get(const char * url, json_object * return_obj);
static char hostname[HOSTNAMELEN];

struct post_con_info {
    struct MHD_PostProcessor *postprocessor;
    char json_string[BUFFERSIZE];
    int json_string_len;
    unsigned int answercode;
};

static long
get_file_size(const char *filename) {
    FILE *fp;

    fp = fopen(filename, "rb");
    if (fp) {
        long size;

        if ((0 != fseek(fp, 0, SEEK_END)) || (-1 == (size = ftell(fp))))
            size = 0;

        fclose(fp);

        return size;
    }
    else
        return 0;
}

static char *
load_file(const char *filename) {
    FILE *fp;
    char *buffer;
    long size;

    size = get_file_size(filename);
    if (size == 0)
        return NULL;

    fp = fopen(filename, "rb");
    if (!fp)
        return NULL;

    buffer = malloc(size);
    if (!buffer) {
        fclose(fp);
        return NULL;
    }

    if (size != fread(buffer, 1, size, fp)) {
        free(buffer);
        buffer = NULL;
    }

    fclose(fp);
    return buffer;
}

/**
 * Get the client's certificate
 *
 * @param tls_session the TLS session
 * @return NULL if no valid client certificate could be found, a pointer
 *  	to the certificate if found
 */
static gnutls_x509_crt_t
get_client_certificate(gnutls_session_t tls_session) {
    unsigned int listsize;
    const gnutls_datum_t *pcert;
    unsigned int client_cert_status;
    gnutls_x509_crt_t client_cert;

    if (tls_session == NULL)
        return NULL;
    if (gnutls_certificate_verify_peers2(tls_session,
                                         &client_cert_status))
        return NULL;
    if(client_cert_status != 0)
        return NULL;
    pcert = gnutls_certificate_get_peers(tls_session,
                                         &listsize);
    if ((pcert == NULL) ||
        (listsize == 0)) {
        fprintf(stderr,
                "Failed to retrieve client certificate chain\n");
        return NULL;
    }
    if (gnutls_x509_crt_init(&client_cert)) {
        fprintf(stderr,
                "Failed to initialize client certificate\n");
        return NULL;
    }
    /* Note that by passing values between 0 and listsize here, you
       can get access to the CA's certs */
    if (gnutls_x509_crt_import(client_cert,
                               &pcert[0],
                               GNUTLS_X509_FMT_DER)) {
        fprintf(stderr,
                "Failed to import client certificate\n");
        gnutls_x509_crt_deinit(client_cert);
        return NULL;
    }
    return client_cert;
}

/**
 * Get the distinguished name from the client's certificate
 *
 * @param client_cert the client certificate
 * @return NULL if no dn or certificate could be found, a pointer
 * 			to the dn if found
 */
char *
cert_auth_get_dn(gnutls_x509_crt_t client_cert) {
    char *buf;
    size_t lbuf;

    lbuf = 0;
    gnutls_x509_crt_get_dn(client_cert, NULL, &lbuf);
    buf = malloc(lbuf);
    if (buf == NULL) {
        fprintf(stderr,
                "Failed to allocate memory for certificate dn\n");
        return NULL;
    }
    gnutls_x509_crt_get_dn(client_cert, buf, &lbuf);

    return buf;
}

/**
 * Check the client cert
 *
 * @param connection
 * @param client_name the string that should be included in the cert DN
 *
 * @return 0 if there is a validate client cert.
 */
static int
is_authenticated(struct MHD_Connection *connection, char *client_name) {
    const union MHD_ConnectionInfo * ci;
    gnutls_session_t tls_session;
    gnutls_x509_crt_t client_cert;
    char * client_dn;
    ci = MHD_get_connection_info(connection,
                                 MHD_CONNECTION_INFO_GNUTLS_SESSION);
    tls_session = ci->tls_session;
    client_cert = get_client_certificate(tls_session);
    if(client_cert == NULL)
        return 1;
    client_dn = cert_auth_get_dn(client_cert);
    gnutls_x509_crt_deinit(client_cert);
    if(client_dn == NULL)
        return 1;
    char * pos = strstr(client_dn, client_name);
    free(client_dn);
    if (pos == NULL)
        return 1;
    return 0;
}

/**
 * Construct the response with a simple string.
 *
 * @param page point to a string which is the body of response
 *
 * @return  MHD_YES or  MHD_NO
 */
static int
string_res(struct MHD_Connection *connection, const char * page, unsigned int status_code) {
    int ret;
    struct MHD_Response *response;

    response =
            MHD_create_response_from_buffer(strlen(page), (void *) page,
                                            MHD_RESPMEM_MUST_COPY);
    if (!response)
        return MHD_NO;

    ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);

    return ret;
}

/**
 * Construct the response with a JSON object.
 *
 * @param data_obj point to the JSON object and it will use json_object_put.
 *        So if it the last reference, it will be freed!
 *
 * @return MHD_YES or  MHD_NO
 */
static int
json_res(struct MHD_Connection *connection, json_object *data_obj) {
    int ret;
    struct MHD_Response *response;
    json_object * obj = json_object_new_object();
    json_object_object_add(obj, "host", json_object_new_string(hostname));
    json_object_object_add(obj, "response", data_obj);
    const char * json_str = json_object_to_json_string(obj);
    response =
        MHD_create_response_from_buffer(strlen(json_str), (void *) json_str,
                                        MHD_RESPMEM_MUST_COPY);
    json_object_put(obj);
    if (!response)
        return MHD_NO;
    MHD_add_response_header(response, "Content-Type", "application/json");
    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;

}

/**
 * Iterate the post data. And store the data with key "json" into con_info->json_string.
 * The string length is no more than BUFFERSIZE-1
 */
static int
iterate_post (void *cls, enum MHD_ValueKind kind, const char *key,
              const char *filename, const char *content_type,
              const char *transfer_encoding, const char *data, uint64_t off, size_t size) {
    if (0 != strcmp (key, "json"))
        return MHD_NO;
    struct post_con_info * con_info = cls;
    con_info->answercode = MHD_HTTP_INTERNAL_SERVER_ERROR;
    if (size + con_info->json_string_len > BUFFERSIZE - 1) {
        return MHD_NO;
    }
    strncat(con_info->json_string, data, size);
    con_info->json_string_len += size;
    con_info->answercode = MHD_HTTP_OK;
    return MHD_YES;
}

static void
request_completed (void *cls, struct MHD_Connection *connection,
                   void **con_cls, enum MHD_RequestTerminationCode toe)
{
    struct post_con_info *con_info = *con_cls;

    if (NULL == con_info)
        return;
    if (NULL != con_info->postprocessor)
        MHD_destroy_post_processor(con_info->postprocessor);
    free (con_info);
    *con_cls = NULL;
}

static int
answer_to_connection(void *cls, struct MHD_Connection *connection,
                     const char *url, const char *method,
                     const char *version, const char *upload_data,
                     size_t *upload_data_size, void **con_cls) {

    if (0 != is_authenticated (connection,CLIENTCN))
        return string_res(connection, "Not Authenticated", MHD_HTTP_UNAUTHORIZED);

    if (NULL == *con_cls) {
        struct post_con_info * con_info;
        con_info = malloc(sizeof(struct post_con_info));
        // allocate memory failed
        if (NULL == con_info)
            return MHD_NO;
        con_info->json_string[0] = '\0';
        con_info->json_string_len = 0;
        con_info->postprocessor = NULL;
        if (0 == strcmp(method, "POST") || 0 == strcmp(method, "PUT")) {
            con_info->postprocessor = MHD_create_post_processor(connection, POSTBUFFERSIZE,
                                                                iterate_post, (void *)con_info);
            if (NULL == con_info->postprocessor)
            {
                free (con_info);
                return MHD_NO;
            }
        }
        *con_cls = (void *) con_info;
        return MHD_YES;
    }

    if (0 == strncmp(url, "/v1", strlen("/v1"))){
        const char * url2 = url + strlen("/v1");
        json_object *return_obj = json_object_new_object();
        if (NULL == return_obj) {
            return string_res(connection, "Internal Error.", MHD_HTTP_INTERNAL_SERVER_ERROR);
        }

        if (0 == strcmp(method, "GET")) {
            process_get(url2, return_obj);
        } else if (0 == strcmp(method, "POST") || 0 == strcmp(method, "PUT")) {
            struct post_con_info *con_info = *con_cls;
            if (0 != *upload_data_size)
            {
                MHD_post_process (con_info->postprocessor, upload_data,
                                  *upload_data_size);
                *upload_data_size = 0;
                return MHD_YES;
            }
            if (MHD_HTTP_OK != con_info->answercode) {
                return string_res(connection, "Internal Error.", con_info->answercode);
            }
            if (0 == strcmp(method, "POST")) {

            }else if (0 == strcmp(method, "PUT")) {

            }
        }

        if (json_object_object_length(return_obj) > 0) {
            return json_res(connection, return_obj);
        } else {
            json_object_object_add(return_obj, "error", json_object_new_string("No results get!"));
            return json_res(connection, return_obj);
        }
    }

    return string_res(connection, "It works!", MHD_HTTP_OK);
}

int
main() {
    struct MHD_Daemon *daemon;
    char *key_pem;
    char *cert_pem;
    char *root_ca_pem;

    key_pem = load_file(SERVERKEYFILE);
    cert_pem = load_file(SERVERCERTFILE);
    root_ca_pem = load_file(ROOTCACERTFILE);
    gethostname(hostname,HOSTNAMELEN);

    if ((key_pem == NULL) || (cert_pem == NULL) || root_ca_pem == NULL) {
        printf("The key/certificate files could not be read.\n");
        return 1;
    }

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY | MHD_USE_SSL,
                              PORT, NULL, NULL,
                              &answer_to_connection, NULL,
                              MHD_OPTION_THREAD_POOL_SIZE, 4,
                              MHD_OPTION_HTTPS_MEM_KEY, key_pem,
                              MHD_OPTION_HTTPS_MEM_CERT, cert_pem,
                              MHD_OPTION_HTTPS_MEM_TRUST, root_ca_pem,
                              MHD_OPTION_NOTIFY_COMPLETED, request_completed, NULL,
                              MHD_OPTION_END);
    if (NULL == daemon) {
        printf("Failed to start daemon\n");

        free(key_pem);
        free(cert_pem);
        free(root_ca_pem);

        return 1;
    }

    getchar();

    MHD_stop_daemon(daemon);
    free(key_pem);
    free(cert_pem);
    free(root_ca_pem);

    return 0;
}