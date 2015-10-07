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

extern json_object * process_get(const char * url);


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

static int
secret_page(struct MHD_Connection *connection) {
    int ret;
    struct MHD_Response *response;
    const char *page = "<html><body>A secret.</body></html>";

    response =
            MHD_create_response_from_buffer(strlen(page), (void *) page,
                                            MHD_RESPMEM_PERSISTENT);
    if (!response)
        return MHD_NO;

    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}

static int
unauth_page(struct MHD_Connection *connection) {
    int ret;
    struct MHD_Response *response;
    const char *page = "<html><body>Not Authenticated.</body></html>";

    response =
            MHD_create_response_from_buffer(strlen(page), (void *) page,
                                            MHD_RESPMEM_PERSISTENT);
    if (!response)
        return MHD_NO;

    ret = MHD_queue_response(connection, MHD_HTTP_UNAUTHORIZED, response);
    MHD_destroy_response(response);

    return ret;
}

static int
json_res(struct MHD_Connection *connection, json_object * obj) {
    int ret;
    struct MHD_Response *response;
    const char * json_str = json_object_to_json_string(obj);
    response =
        MHD_create_response_from_buffer(strlen(json_str), (void *) json_str,
                                        MHD_RESPMEM_MUST_FREE);
    if (!response)
        return MHD_NO;
    MHD_add_response_header(response, "Content-Type", "application/json");
    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;

}

static int
answer_to_connection(void *cls, struct MHD_Connection *connection,
                     const char *url, const char *method,
                     const char *version, const char *upload_data,
                     size_t *upload_data_size, void **con_cls) {


    if (0 != strcmp(method, "GET"))
        return MHD_NO;
    if (NULL == *con_cls) {
        *con_cls = connection;
        return MHD_YES;
    }

    if (0 != is_authenticated (connection,CLIENTCN))
        return unauth_page(connection);
    json_object * json_obj;
    if (0 == strcmp(method, "GET"))
        json_obj = process_get(url);
    if (NULL != json_obj) {
        return json_res(connection, json_obj);
    }
    return secret_page(connection);
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
                              MHD_OPTION_END);
    if (NULL == daemon) {
        printf("%s\n", cert_pem);

        free(key_pem);
        free(cert_pem);

        return 1;
    }

    getchar();

    MHD_stop_daemon(daemon);
    free(key_pem);
    free(cert_pem);

    return 0;
}