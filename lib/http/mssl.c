#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <assert.h>
#include "libpill.h"
#include "mssl.h"

/*
 * ABOUT openssl + thread 
 * https://www.openssl.org/docs/manmaster/crypto/threads.html
 *
 * N.B.: Handle all ssl connections in a single thread.
 * If not running in the main thread, call ERR_remove_state(0) and ??
 * in thread cleanup handler (unless detached).
 */

static SSL_CTX *ssl_cli_ctx;
static SSL_CTX *ssl_serv_ctx;

typedef struct {
    unsigned long sslerr;
//    SSL_CTX *ctx;
    BIO *bio;
} sslconn;

/* man BIO_should_retry:
 ..          For example if a call to BIO_read() on a socket BIO returns
       0 and BIO_should_retry() is false then the cause will be that the
       connection closed.
*/

static int ssl_wait(mill_fd mfd, int64_t deadline) {
    sslconn *sconn = mill_getdata(mfd);
    if (! BIO_should_retry(sconn->bio)) {
        sconn->sslerr = ERR_get_error();
        /* XXX: Ref. openssl/source/ssl/ssl_lib.c .. */
        if (ERR_GET_LIB(sconn->sslerr) != ERR_LIB_SYS)
            errno = EIO;
        return -1;
    }

    if (BIO_should_read(sconn->bio)) {
        int rc = mill_fdwait(mfd, FDW_IN, deadline);
        if (rc == 0) {
            errno = ETIMEDOUT;
            return -1;
        }
        return rc;
    }
    if (BIO_should_write(sconn->bio)) {
        int rc = mill_fdwait(mfd, FDW_OUT, deadline);
        if (rc == 0) {
            errno = ETIMEDOUT;
            return -1;
        }
        return rc;
    }
    /* assert(!BIO_should_io_special(sconn->bio)); */
    errno = EIO;
    return -1;  /* should not happen ? */
}

#if 0
http://www.opensource.apple.com/source/OpenSSL/OpenSSL-16/openssl/crypto/x509/x509_vfy.c

Use int X509_cmp_current_time(s); -- google for man page 
    also X509_cmp_time ... X509.h

Validity Period
OpenSSL represents the not-valid-after (expiration) and
not-valid-before as ASN1_TIME objects, which can be extracted as follows:

ASN1_TIME *not_before = X509_get_notBefore(cert);
	ASN1_TIME *not_after = X509_get_notAfter(cert);

ASN1_TIME *cert_time;
char *pstring;
cert_time = X509_get_notBefore(cert));
pstring = (char*)cert_time->data;
printf("Time unformatted is: %s",pstring);
The format should be like "YYMMDDHHMMSSZ"


    -- See openssl/objects.h for NID_XXX 
/* Check that the common name matches the host name*/ 
void check_cert_chain(ssl,host) 
  SSL *ssl; 
  char *host; 
  {

    X509 *peer; 
    char peer_CN[256];
    if(SSL_get_verify_result(ssl)!=X509_V_OK) 
              berr_exit("Certificate doesn't verify");

    /*Check the common name*/ 
   peer=SSL_get_peer_certificate(ssl); 
   X509_NAME_get_text_by_NID ( 
            X509_get_subject_name (peer),  NID_commonName,  peer_CN, 256); 
   if(strcasecmp(peer_CN, host)) 
            err_exit("Common name doesn't match host name");

  } 
#endif

/* optional: call after ssl_connect()/ssl_accept() */
int ssl_handshake(mill_fd mfd, int64_t deadline) {
    sslconn *sconn = mill_getdata(mfd);
    if (mill_getfd(mfd) == -1) {
        errno = EBADF;
        return -1;
    }
    while (BIO_do_handshake(sconn->bio) <= 0) {
        if (ssl_wait(mfd, deadline) < 0)
            return -1;
    }

#if 0
    SSL *ssl;
    BIO_get_ssl(conn->bio, &ssl);
    X509 *cert = SSL_get_peer_certificate(ssl);
    if (cert) {
        X509_NAME *certname = X509_get_subject_name(cert);
        X509_NAME_print_ex_fp(stderr, certname, 0, 0);
        X509_free(cert);
#if 0
        /* verify cert, requires loading known certs.
         * See SSL_CTX_load_verify_locations() call in ssl_init() */
        if (SSL_get_verify_result(ssl) != X509_V_OK) {
            warn ...
        }
#endif
    }
#endif
    return 0;
}

void ssl_closefd(mill_fd mfd) {
    if (mill_getfd(mfd) != -1) { 
        sslconn *sconn = mill_getdata(mfd);
        BIO_ssl_shutdown(sconn->bio);
        mill_fdclose(mfd);
    }
}

void ssl_close(mill_fd mfd) {
    sslconn *sconn = mill_getdata(mfd);
    ssl_closefd(mfd);
    BIO_free_all(sconn->bio);
    (void) mill_close(mfd, 0);
}

const char *ssl_errstr(mill_fd mfd) {
    static const char unknown_err[] = "Unknown error";
    sslconn *sconn = mill_getdata(mfd);
    if (sconn->sslerr)
        return ERR_error_string(sconn->sslerr, NULL);
    if (errno)
        return strerror(errno);
    return unknown_err;
}

static sslconn *ssl_conn_new(int fd, SSL_CTX *ctx, int client) {
    assert(ctx);
    SSL *ssl = NULL;
    BIO *sbio = BIO_new_ssl(ctx, client);
    if (! sbio)
        return NULL;
    BIO_get_ssl(sbio, & ssl);
    if (!ssl) {
        BIO_free(sbio);
        return NULL;
    }

    /* set .._PARTIAL_WRITE for non-blocking operation */
    SSL_set_mode(ssl, SSL_MODE_ENABLE_PARTIAL_WRITE);
    BIO *cbio = BIO_new_socket(fd, BIO_NOCLOSE);
    if (! cbio) {
        BIO_free(sbio);
        return NULL;
    } 
    BIO_push(sbio, cbio);
    sslconn *sconn = malloc(sizeof(sslconn));
    if (! sconn) {
        BIO_free_all(sbio);
        return NULL;
    }

    /*  assert(BIO_get_fd(sbio, NULL) == fd); */

    sconn->bio = sbio;
    sconn->sslerr = 0;

    /* OPTIONAL: call ssl_handshake() to check/verify peer certificate */
    return sconn;
}

int ssl_read(mill_fd mfd, void *buf, int len, int64_t deadline) {
    int rc;
    if(mill_getfd(mfd) == -1) {
        errno = EBADF;
        return -1;
    }
    if (len < 0) {
        errno = EINVAL;
        return -1;
    }
    sslconn *sconn = mill_getdata(mfd);
    do {
        rc = BIO_read(sconn->bio, buf, len);
        if (rc > 0)
            break;
        if (ssl_wait(mfd, deadline) < 0) {
            if (rc == 0) {
                errno = 0;
                return 0;
            }
            return -1;
        }
    } while (1);
    return rc;
}

int ssl_write(mill_fd mfd, const void *buf, int len, int64_t deadline) {
    int rc;
    if (mill_getfd(mfd) == -1) {
        errno = EBADF;
        return -1;
    }
    if (len < 0) {
        errno = EINVAL;
        return -1;
    }
    sslconn *sconn = mill_getdata(mfd);
    do {
        rc = BIO_write(sconn->bio, buf, len);
        if (rc > 0)
            break;
        if (ssl_wait(mfd, deadline) < 0)
            return -1;
    } while (1);
    return rc;
}

static int load_certificates(SSL_CTX *ctx,
                const char *cert_file, const char *key_file) {

    if (SSL_CTX_use_certificate_file(ctx, cert_file, SSL_FILETYPE_PEM) <= 0
        || SSL_CTX_use_PrivateKey_file(ctx, key_file, SSL_FILETYPE_PEM) <= 0
    )
        return 0;
    if (SSL_CTX_check_private_key(ctx) <= 0) /* inconsistent private key */
        return 0;
    return 1;
}

/* use ERR_print_errors(_fp) for SSL error */
static int ssl_init(void) {
    ERR_load_crypto_strings();
    ERR_load_SSL_strings();
    OpenSSL_add_all_algorithms();

    SSL_library_init();

    /* seed the PRNG .. */

    ssl_cli_ctx = SSL_CTX_new(SSLv23_client_method());
    if (ssl_cli_ctx == NULL)
        return 0;
#if 0
    /* XXX: if verifying cert using SSL_get_verify_result(), see ssl_handshake.
     *
     */
    if (!SSL_CTX_load_verify_locations(ssl_cli_ctx, NULL, "/etc/ssl/certs")) {

    }
#endif
    return 1;
}

/* use ERR_print_errors(_fp) for SSL error */
int ssl_serv_init(const char *cert_file, const char *key_file) {
    if (! ssl_cli_ctx) {
        if (! ssl_init())
            return 0;
    }
    ssl_serv_ctx = SSL_CTX_new(SSLv23_server_method());
    if (! ssl_serv_ctx)
        return 0;
    return load_certificates(ssl_serv_ctx, cert_file, key_file);
}

#if 0
/* use ERR_print_errors(_fp) for SSL error */
int ssl_serv_init(const char *cert_file, const char *key_file) {
    if (! ssl_cli_ctx) {
        if (! ssl_init())
            return 0;
    }
    ssl_serv_ctx = SSL_CTX_new(SSLv23_server_method());
    if (! ssl_serv_ctx)
        return 0;
    /* return load_certificates(ssl_serv_ctx, cert_file, key_file); */
    if (SSL_CTX_use_certificate_file(ctx, cert_file, SSL_FILETYPE_PEM) <= 0
        || SSL_CTX_use_PrivateKey_file(ctx, key_file, SSL_FILETYPE_PEM) <= 0
    )
        return 0;
    if (SSL_CTX_check_private_key(ctx) <= 0) /* inconsistent private key */
        return 0;
    return 1;
}
#endif

mill_fd ssl_connect(ipaddr *addr, int64_t deadline) {
    if (! ssl_cli_ctx) {
        if (! ssl_init()) {
            errno = EPROTO;
            return NULL;
        }
    }
    mill_fd mfd = tcpconnect(addr, deadline);
    if (!mfd)
        return NULL;
    sslconn *sconn = ssl_conn_new(mill_getfd(mfd), ssl_cli_ctx, 1);
    if (!sconn) {
        mill_close(mfd, 1);
        errno = ENOMEM;
        return NULL;
    }
    mill_setdata(mfd, sconn);
    return mfd;
}

mill_fd ssl_accept(mill_fd lsock, int64_t deadline) {
    if (! ssl_serv_ctx) {
        errno = EPROTO;
        return NULL;
    }
    mill_fd mfd = tcpaccept(lsock, deadline);
    if (!mfd)
        return NULL;
    sslconn *sconn = ssl_conn_new(mill_getfd(mfd), ssl_serv_ctx, 0);
    if (!sconn) {
        mill_close(mfd, 1);
        errno = ENOMEM;
        return NULL;
    }
    mill_setdata(mfd, sconn);
    return mfd;
}
