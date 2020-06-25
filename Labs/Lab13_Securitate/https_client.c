#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>

#include "stdio.h"
#include "string.h"

#define BUFFLEN 1024

int main() {
    BIO* bio;
    SSL* ssl;
    SSL_CTX* ctx;
    FILE* fp;

    int p;
    char buff[BUFFLEN];
    char r[BUFFLEN];
    char* request = "GET / HTTP/1.0\r\nHost: www.verisign.com\r\n\r\n";

    /* initializare librarie */
    SSL_library_init();
    ERR_load_BIO_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    /* initializare context */
    ctx = SSL_CTX_new(SSLv23_client_method());   

    /* incarca trust store */
    if(!SSL_CTX_load_verify_locations(ctx, "TrustStore.pem", NULL)) {
        fprintf(stderr, "Error loading trust store\n");
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return 0;
    }

    /* stabileste conexiune */
    bio = BIO_new_ssl_connect(ctx);
   
    /* Seteaza flag SSL_MODE_AUTO_RETRY  */
    BIO_get_ssl(bio, &ssl);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    /* setup conexiune */
    BIO_set_conn_hostname(bio, "verisign.com:443");

    /* verifica conexiunea deschisa si efectueaza handshake */
    if (BIO_do_connect(bio) <= 0) {
        /* trateaza esec conexiune */
        fprintf(stderr, "Connection failed\n");
        ERR_print_errors_fp(stderr);
    }

    /* verifica certificat */
    if (SSL_get_verify_result(ssl) != X509_V_OK) {
        /* trateaza esec verificare */
        fprintf(stderr, "Invalid certificate\n");
        ERR_print_errors_fp(stderr);
    }

    /* Trimite request */
    BIO_write(bio, request, strlen(request));

    /* Deschide fisier de scriere */
    fp = fopen("reply.txt", "w");
    /* Citeste raspuns si pregateste  output*/
    while (1) {
        p = BIO_read(bio, buff, BUFFLEN);
        if (p < 0) {
            if (!BIO_should_retry(bio)) {
                /* esec conexiune */
                fprintf(stderr, "Connection error\n");
                ERR_print_errors_fp(stderr);
                break;
            } else {
                /* retry conexiune */
                fprintf(stderr, "Reestablished connection\n");
                ERR_print_errors_fp(stderr);
            }
        } else if (p == 0) {
            /* conexiune inchisa*/
            fprintf(stderr, "Succesfully received\n");
            ERR_print_errors_fp(stderr);
            break;
        }

        fwrite(buff, 1, strlen(buff), fp);
    }
    fclose(fp);

    /* Inchide conexiune si elibereaza context */
    BIO_free_all(bio);
    SSL_CTX_free(ctx);
    return 0;
}
