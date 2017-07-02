#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#include "libpill.h"
#include "mssl.h"
#include "jsv8dlfn.h"

enum {
    HEADER_COMPLETE = 1,
    BODY_COMPLETE
};

struct decoder_s {
    int bytes_in_chunk;
    int state;
    int (*ondata)(char *buf, int size, void *data);
    void *data;
};

enum {
    STATE_CHUNK_SIZE,
    STATE_CHUNK_EXT,
    STATE_CHUNK_DATA,
    STATE_CHUNK_CRLF,
};

enum {
    CHUNK_NOMEM = -2,
    CHUNK_ERROR = -1,
    CHUNK_DONE = 0,
    CHUNK_INCOMPLETE,
    CHUNK_DATA,
};

struct http_s {
    struct mill_fd_s *mfd;
    char *host;
    int port;
    int64_t deadline;

    struct mill_fd_s *(*connectfunc)(ipaddr *addr, int64_t deadline);
    int (*writefunc)(struct mill_fd_s *mfd, const void *buf, int count, int64_t deadline);
    int (*readfunc)(struct mill_fd_s *mfd, void *buf, int count, int64_t deadline);
    void (*closefunc)(struct mill_fd_s *);
    void (*closefd)(struct mill_fd_s *);

    int (*ondata)(char *buf, int size, void *q, int done);
    v8_state vm;
    v8_coro cr;

    int state;

    /* read buffer */
    char *buf;
    int buf_size;
    int buf_len;

    int code;
    int version;
    char *head;
    int head_len;
    char *body;
    int body_len;
    int content_len;
#define CONTENT_CHUNKED -2
#define CONTENT_LENGTH_NONE -1
    int keep_alive;

    struct decoder_s decoder;
};


#define emalloc(sz)  http_alloc_func(NULL, sz)
#define erealloc(ptr, sz)   http_alloc_func(ptr, sz)
#define efree(ptr)  (void) http_alloc_func(ptr, 0)
void *(*http_alloc_func)(void *ptr, size_t size) = realloc;

static const char *
is_header_complete(const char *buf, const char *end, int last_len, int *ret) {
    int ret_cnt = 0;
    buf = last_len < 3 ? buf : buf + last_len - 3;
    *ret = 0;   /* incomplete */
    while (1) {
        if (buf == end)
            return NULL;
        if (*buf == '\r') {
            buf++;
            if (buf == end)
                return NULL;
            if (*buf != '\n') {
                *ret = -1;  /* error */
                return NULL;
            }
            buf++;
            ret_cnt++;
        } else if (*buf == '\n') {
            buf++;
            ret_cnt++;
        } else {
            buf++;
            ret_cnt = 0;
        }
        if (ret_cnt == 2) {
            *ret = 1;
            return buf;
        }
    }
    return NULL;
}

/*
 * return:
 *  0       -- incomplete
 *  -1      -- error
 *  > 0     -- size of header
 */

static int
http_response_header(const char *buf, int size, int last_size) {
    int ret;
    const char *start = buf;
    const char *end;
    if (size < 0)
        return -1;
    if (! (end = is_header_complete(buf, buf + size, last_size, &ret)))
        return ret;
    ret = (end - start);
#if 0
    ret--;  /* end[-1] == '\n' */
    if (end[-2] == '\r')
        ret--;
#endif
    return ret;
}

/* version = 10 * major + minor */
static int
http_response_code(const char *buf, int size, int *version) {
    if (! buf || size < 13)
        return 0;
    if (! (buf[0] == 'H'
            && buf[1] == 'T' && buf[2] == 'T' && buf[3] == 'P'
            && buf[4] == '/'
            && buf[5] >= '0' && buf[5] <= '9'
            && buf[6] == '.' && buf[7] >= '0' && buf[7] <= '9' && buf[8] == ' '
        )
    )
        return -1;
    if (version)
        *version = (buf[5] - '0') * 10 + buf[7] - '0';
    if (buf[9] < '1' || buf[9] > '9' || buf[10] < '0' || buf[10] > '9'
        || buf[11] < '0' || buf[11] > '9' || buf[12] != ' '
    )
        return -1;

    return (buf[9]-'0') * 100 + (buf[10]-'0') * 10 + buf[11] - '0';
}

static inline unsigned
upcase(unsigned c) {
    if (c >= 'a' && c <= 'z')
        c -= 0x20;
    return c;
}

static const char *
field_value(const char *f, const char *end, int *len) {
    const char *g;
    while (f < end && (f[0] == ' ' || f[0] == '\t')) f++;
    if (f == end) return NULL;
    for (g = f+1; g < end && (*g >= ' ' || *g == '\t'); g++)
		;   /* FIXME reject DEL */
    if (g == end) return NULL;
    if (! (*g == '\n' || (g[0] == '\r' && g+1 < end && g[1] == '\n')))
        return NULL;  
	while (g > f && (g[-1] == ' ' || g[-1] == '\t')) g--;
    *len = g - f;
    return f;
}

/* 
 * Returns a pointer to the value of the item or NULL.
 * Skips leading and trailing spaces.
 *  e.g:
 *      const char *p = find_header(..., ..., "Content-Length", & len);
 */

static const char *
find_header(const char *head, int head_len, const char *name, int *len) {
    const char *p, *f;
    const char *end = head + head_len;
    if (*name == '\0')
        return NULL;
    for (f = head; f < end; f++) {
        if (*f != '\n')
            continue;
        f++;
        for (p = name; *p && f < end; p++, f++)
            if (upcase(*p) != upcase(*f))
                goto cont;
        if (f >= end)
            return NULL;
        if (f[0] == ':') {
            p = field_value(f+1, end, len);
            return p;
#if 0
            if (p == NULL)
                return NULL;
            f = get_eol(p + *len, end);
            assert(f);
            while (f < end && (*f == ' ' || *f == '\t')) {
                /* multi-line value */
                int len1;
                const char *v1 = field_value(f, end, & len1);
                if (!v1)
                    return NULL;
                *len = (v + len1) - p;
                f = get_eol(v + len1, end);
            }
            return p;
#endif
        }
cont:
        f--;
    }
    return NULL;
}

#if 0
/* returned pointer is after NEWLINE */
static const char *
get_eol(const char *buf, const char *buf_end) {
    for (; buf < buf_end; buf++)
        if (*buf == '\n') return ++buf;
    return NULL;
}
#endif

static int
decode_chunked(struct decoder_s *decoder, char *buf, int *size) {
    char *sp = buf;
    char *end = buf + *size;
    *size = 0;

#define DECODE_HEX(ch) \
((ch >= '0' && ch <= '9') ? (ch - '0') \
    : (ch >= 'a' && ch <= 'f') ? (ch - 'a' + 10) \
        : (ch >= 'A' && ch <= 'F') ? (ch - 'A' + 10) : -1)

    for(;;) {
        switch (decoder->state) {
        case STATE_CHUNK_SIZE: {
            unsigned bchunk = 0;
            int nhex = 0, v;
            for (;;++sp) {
                if (sp == end)
                    return CHUNK_INCOMPLETE;
                if (sp[0] == '\r') {
                    if (nhex > 0)
                        break;
                    return CHUNK_ERROR;
                }
                v = DECODE_HEX(sp[0]);
                if (v == -1 || nhex == 8) {
                    return CHUNK_ERROR;
                }
                bchunk = bchunk * 16 + v;
                nhex++;
            }
            if (bchunk > INT_MAX) {
                fprintf(stderr, "a big chunk!\n");
                return CHUNK_ERROR;
            }
            decoder->bytes_in_chunk = bchunk;
            decoder->state = STATE_CHUNK_EXT;
            *size = sp - buf;
        }
        case STATE_CHUNK_EXT:
            for (;; ++sp) {
                if (sp == end)
                    return CHUNK_INCOMPLETE;
                if (*sp == '\r')
                    break;
                if (*sp == '\n')
                    return CHUNK_ERROR;
            }
            if (++sp == end)
                return CHUNK_INCOMPLETE;
            if (*sp != '\n')
                return CHUNK_ERROR;
            sp++;

            /* trailer:
             *      0\r\nHeader_Name: Header_Value \r\n\r\n
             * or none:
             *      0\r\n\r\n
             */

            if (decoder->bytes_in_chunk == 0) {
                sp -= 2;   /* \r\n; use http_get_header() for trailer-part */
                if (! decoder->ondata(sp, 0, decoder->data))
                    return CHUNK_NOMEM;
                *size = sp - buf;
                return CHUNK_DONE;
            }
            decoder->state = STATE_CHUNK_DATA;
            *size = sp - buf;
        case STATE_CHUNK_DATA: {
            int bytes_available = end - sp;
            assert(bytes_available >= 0);
            if (bytes_available <= 0)
                return CHUNK_INCOMPLETE;
            if (bytes_available < decoder->bytes_in_chunk) {
                if (!decoder->ondata(sp, bytes_available, decoder->data))
                    return CHUNK_NOMEM;
                sp += bytes_available;
                decoder->bytes_in_chunk -= bytes_available;
                *size = sp - buf;
                return CHUNK_DATA;
            }
            if (!decoder->ondata(sp,
                        decoder->bytes_in_chunk, decoder->data))
                return CHUNK_NOMEM;
            sp += decoder->bytes_in_chunk;
            decoder->bytes_in_chunk = 0;
            decoder->state = STATE_CHUNK_CRLF;
            *size = sp - buf;
            return CHUNK_DATA;
        }
        case STATE_CHUNK_CRLF:
            if (sp == end)
                return CHUNK_INCOMPLETE;
            if (*sp != '\r')
                return CHUNK_ERROR;
            if (++sp == end)
                return CHUNK_INCOMPLETE;
            if (*sp != '\n')
                return CHUNK_ERROR;
            sp++;
            decoder->state = STATE_CHUNK_SIZE;
            *size = sp - buf;
            break;
        default:
            break;
        }
    }
    return CHUNK_ERROR;
#undef DECODE_HEX
}


#define AMOUNT_TO_READ 1500

static int
check_room(struct http_s *s) {
    if (! s->buf)
        s->buf_size = AMOUNT_TO_READ;
    else if (s->buf_size - s->buf_len >= AMOUNT_TO_READ)
        return 0;
    else {
        while (s->buf_size - s->buf_len < AMOUNT_TO_READ)
            s->buf_size *= 2;
    }
    s->buf = erealloc(s->buf, s->buf_size);
    if (! s->buf) {
        errno = ENOMEM;
        s->state = -errno;
        return -1;
    }
    return 0;
}

static int chunked_data(char *buf, int size, void *data) {
    struct http_s *s = data;
    if (size > 0) {
        char *ptr = emalloc(size);
        if (!ptr)
            return 0;
        memcpy(ptr, buf, size);
        if (-1 == s->ondata(ptr, size, s, 0))
            return 0;
    } else
        s->ondata(NULL, 0, s, 1);
    return 1;
}

static int
fetch_chunked_body(struct http_s *s) {
    int rc, last_len;
    for(;;) {
        last_len = s->buf_len;
        rc = decode_chunked(&s->decoder, s->buf, &last_len);
        if (rc == CHUNK_ERROR) {
            errno = EPROTO;
            return -1;
        } else if (rc == CHUNK_NOMEM) {
            errno = ENOMEM;
            return -1;
        }

        s->buf_len -= last_len;
        assert(s->buf_len >= 0);

        /* XXX: s->buf_len is usually 0 */
        memmove(s->buf, s->buf + last_len, s->buf_len);
        if (rc == CHUNK_DONE) {
            /* fprintf(stderr, "CHUNK_DONE\n"); */
            break;
        }
        if (rc == CHUNK_DATA)
            return rc;

        if (! s->mfd) { /* XXX: fatal ? */
            errno = EIO;
            return -1;
        }
        if (-1 == check_room(s))
            return -1;
        rc = s->readfunc(s->mfd, s->buf + s->buf_len,
                                    AMOUNT_TO_READ, s->deadline);
        if (rc == 0) {
            errno = ECONNRESET;
            return -1;
        }
        if (rc < 0)
            return -1;
        s->buf_len += rc;
    }


    /* trailer -- currently not used ? */
    last_len = 0; 
    for(;;) {
        rc = http_response_header(s->buf, s->buf_len, last_len); 
        if (rc < 0) {
            errno = EPROTO;
            return -1;
        }
        if (rc > 0) {
            // fprintf(stderr, "trailer bytes = %d\n", rc);
            s->buf_len -= rc;
            if (s->buf_len > 0)
                memmove(s->buf, s->buf + rc, s->buf_len);
            break;
        }
        if (-1 == check_room(s))
            return -1;
        rc = s->readfunc(s->mfd, s->buf + s->buf_len,
                                    AMOUNT_TO_READ, s->deadline);
        if (rc == 0) {
            errno = ECONNRESET;
            return -1;
        }
        if (rc < 0)
            return -1;
        last_len = s->buf_len;
        s->buf_len += rc;
    }
    return 0;
}

static int
fetch_body(struct http_s *s) {
    int content_len = s->content_len;
    int rc;

    if ((s->code / 100) == 1 || s->code == 204 || s->code == 304    /* implied zero length. */
            || s->content_len == 0
    ) {
        s->ondata(NULL, 0, s, 1);
        return 0;
    }

    if (s->content_len == CONTENT_CHUNKED)
        return fetch_chunked_body(s);

    if (s->buf_len > 0) {
        assert(s->buf);
        if (content_len > 0 && s->buf_len >= content_len) {
            char *sb = s->buf;
            s->buf = NULL;
            s->buf_len -= content_len;
            if (s->buf_len > 0) {
                if (-1 == check_room(s))
                    return -1;
                assert(s->buf_size >= s->buf_len);
                memcpy(s->buf, sb + content_len, s->buf_len);
            }
            if (-1 == s->ondata(sb, content_len, s, 1))
                return -1;
            return content_len;
        }
        if (-1 == s->ondata(s->buf, (rc = s->buf_len), s, 0))
            return -1;
        s->buf = NULL;
        s->buf_len = 0;
        return rc;
    }

    if (! s->mfd) { // XXX: fatal?
        errno = EIO;
        return -1;
    }

    assert(s->buf_len == 0);

    if (s->buf == NULL && -1 == check_room(s))
        return -1;
    assert(s->buf_size == AMOUNT_TO_READ);
    rc = s->readfunc(s->mfd, s->buf,
                                AMOUNT_TO_READ, s->deadline);
    if (rc < 0)
        return -1;

    if (content_len > 0 && s->body_len + rc >= content_len) {
        char *sb = s->buf;
        s->buf_len = s->body_len + rc - content_len; /* extra bytes */
        rc -= s->buf_len;
        assert(rc > 0);
        s->buf = NULL;
        if (s->buf_len > 0) {
            if (-1 == check_room(s))
                return -1;
            assert(s->buf_size >= s->buf_len);
            memcpy(s->buf, sb + rc, s->buf_len);
        }
        if (-1 == s->ondata(sb, rc, s, 1))
            return -1;
        return rc;
    }

    if (rc == 0) {
        if (content_len < 0) {
            s->ondata(NULL, 0, s, 1);
            return 0;
        }
        errno = ECONNRESET;
        return -1;
    }

    /* rc > 0 */
    if (-1 == s->ondata(s->buf, rc, s, 0))
        return -1;
    s->buf = NULL;
    s->buf_len = 0;
    return rc;
}

static int
header_contains(const char *hp, int hlen, const char *name) {
    const char *p, *f, *g;
    const char *end = hp + hlen;
    int namelen = strlen(name);
    for (f = hp; f < end; f++) {
        if (namelen > (end - f))
            return 0;
        g = f;
        for (p = name; *p; p++, g++)
            if (upcase(*p) != upcase(*g))
                goto cont;
        return 1;
cont:;
    }
    return 0;
}

static int
fetch_header(struct http_s *s) {
    int rc, last_len;
    char *hp;
    assert(s->state == 0);
    if (! s->mfd) {
        errno = EINVAL;
        s->state = -errno;
        return -1;
    }
    s->content_len = CONTENT_LENGTH_NONE;

    for(;;) {
        if (-1 == check_room(s))
            return -1;
        rc = s->readfunc(s->mfd, s->buf + s->buf_len,
                                    AMOUNT_TO_READ, s->deadline);
        if (rc == 0) {
            errno = ECONNRESET;
            s->state = -errno;
            return -1;
        }
        if (rc < 0) {
            s->state = -errno;
            return -1;
        }
        last_len = s->buf_len;
        s->buf_len += rc;
        if (!s->code && s->host) {  /* response header */
            s->code = http_response_code(s->buf, s->buf_len, &s->version);
            if (s->code < 0) {
                errno = EPROTO;
                s->state = -errno;
                return -1;
            }
            if (s->code == 0)
                continue;
        }
        rc = http_response_header(s->buf, s->buf_len, last_len); 
        if (rc < 0) {
            errno = EPROTO;
            s->state = -errno;
            return -1;
        }
        if (rc == 0)
            continue;
        last_len = s->buf_len;
        s->head_len = rc;
        s->head = s->buf;
        s->buf = NULL;
        s->buf_len = 0; 
        if (-1 == check_room(s))
            return -1;
        if (last_len > s->head_len) {
            s->buf_len = last_len - s->head_len;
            assert(s->buf_size >= s->buf_len);   /* ? */
            memcpy(s->buf, s->head + s->head_len, s->buf_len);
        }
        break;
    }

    /* remove excess CRLF */
    s->head_len--;
    if (s->head_len > 0 && s->head[s->head_len - 1] == '\r')
        s->head_len--;
    /* "... If a message is received with both a
     * Transfer-Encoding header field and a Content-Length header field,
     * the latter MUST be ignored."
     */

    /* Content-Length >= 0 */
    hp = (char *) find_header(s->head, s->head_len, "Content-Length", &rc);
    if (hp) {
        char save = hp[rc];
        long l; 
        hp[rc] = '\0';
        errno = 0;
        l = strtol(hp, NULL, 10);
        hp[rc] = save;
        if (l < 0 || l > INT_MAX)
            errno = ERANGE;
        if (errno != 0) {
            s->state = -errno;
            return -1;
        }
        s->content_len = l;
    }

    hp = (char *) find_header(s->head, s->head_len, "Transfer-Encoding", &rc);
    if (hp) {
        if (header_contains(hp, rc, "chunked")) {
            s->content_len = CONTENT_CHUNKED;
            s->decoder.ondata = chunked_data;
            s->decoder.data = s;
        } else if (header_contains(hp, rc, "identity"))
            ;
        else {
            // deflate, gzip, or compress ? Also, several values can be listed,
            // separated by a comma e.g.:
            //  Transfer-Encoding: gzip, chunked

            errno = EPROTO;
            s->state = -errno;
            return -1;
        }
    }

    s->keep_alive = 0;
    if (! (s->content_len > 0 || s->content_len == CONTENT_CHUNKED))
        ;
    else {
        hp = (char *) find_header(s->head, s->head_len, "Connection", &rc);
        if (hp) {
            if (header_contains(hp, rc, "close"))
                ;
            else if (header_contains(hp, rc, "keep-alive"))
                s->keep_alive = 1;
        }
    }

    s->state = HEADER_COMPLETE;
    return s->head_len;
}

static void
http_free(void *ptr) {
    if (ptr) {
        struct http_s *s = ptr;
        if (s->mfd) {
            s->closefunc(s->mfd);
            s->mfd = NULL;
        }
        if (s->head)
            efree(s->head);
        if (s->body)
            efree(s->body);
        if (s->buf)
            efree(s->buf);
        if (s->host)
            free(s->host);  /* from strdup() */
        efree(ptr);
    }
}

static void
tcpclose(struct mill_fd_s *mfd) {
    (void) mill_close(mfd, 1);
}

static struct http_s *
http_create(const char *host, int port, int useTLS) {
    struct http_s *s = emalloc(sizeof (struct http_s));
    if (! s) {
        errno = ENOMEM;
        return NULL;
    }
    memset(s, '\0', sizeof (struct http_s));
    s->host = strdup(host);
    if (!s->host) {
        efree(s);
        errno = ENOMEM;
        return NULL;
    }
    s->port = port;
    s->deadline = -1;
    if (useTLS) {
        s->connectfunc = ssl_connect;
        s->writefunc = ssl_write;
        s->readfunc = ssl_read;
        s->closefunc = ssl_close;
        s->closefd = ssl_closefd;
    } else {
        s->connectfunc = tcpconnect;
        s->writefunc = mill_write;
        s->readfunc = mill_read;
        s->closefunc = tcpclose;
        s->closefd = mill_fdclose;
    }
    return s;
}

static coroutine void
do_http_connect(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    struct http_s *s = V8_TOPTR(args[0]);
    ipaddr addr;
    int rc = ipremote(&addr, s->host, s->port, 0, s->deadline);
    if (rc != 0) {
        jsv8->goreject(vm, cr, strerror(errno));
    } else {
        s->mfd = s->connectfunc(&addr, s->deadline);
        if (!s->mfd)
            jsv8->goreject(vm, cr, strerror(errno));
        else
            jsv8->goresolve(vm, cr, V8_VOID, 1);
    }
}

static coroutine void
do_http_send(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    unsigned len = 0;
    struct http_s *s = V8_TOPTR(args[0]);
    unsigned wlen = V8_TOUINT32(args[2]);
    void *wbuf;
    if (V8_ISSTRING(args[1]))
        wbuf = V8_TOSTR(args[1]);
    else
        wbuf = V8_TOPTR(args[1]);

    while (1) {
        int rc = s->writefunc(s->mfd,
                        wbuf + len, wlen - len, s->deadline);
        if (rc >= 0) {  /* sending 0 bytes shouldn't be an error */
            len += rc;
            if (len == wlen) {
                jsv8->goresolve(vm, cr, V8_VOID, 1);
                break;
            }
        } else {
            jsv8->goreject(vm, cr, strerror(errno));
            break;
        }
    }
}

static coroutine void
do_http_header(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    struct http_s *s = V8_TOPTR(args[0]);
    int rc = fetch_header(s);
    if (rc < 0) {
        assert(s->state < 0);
        jsv8->goreject(vm, cr, strerror(-s->state));
    } else {
        jsv8->goresolve(vm, cr, V8_STR(s->head, s->head_len), 1);
        free(s->head);
        s->head = NULL;
    }
}

static int
v8_reada(char *buf, int size, void *p1, int done) {
    struct http_s *s = p1;
    if (done) {
        s->state = BODY_COMPLETE;
    }
    if (size > 0) {
        s->body = erealloc(s->body, s->body_len+size);
        if (!s->body) {
            errno = ENOMEM;
            return -1;
        }
        memcpy(s->body+s->body_len, buf, size);
        s->body_len += size;
    }
    efree(buf);
    return 0;
}

/* read complete body */
static coroutine void
do_http_reada(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    struct http_s *s = V8_TOPTR(args[0]);
    int rc;

    if (s->state != HEADER_COMPLETE) {
        int err = s->state < 0 ? -s->state : EPROTO;
        jsv8->goreject(vm, cr, strerror(err));
        return;
    }

    assert(s->body == NULL);
    s->vm = vm;
    s->cr = cr;
    s->ondata = v8_reada;
    do {
        rc = fetch_body(s);
    } while (rc >= 0 && s->state != BODY_COMPLETE);

    if (rc < 0) {
        s->state = -errno;
        jsv8->goreject(vm, cr, strerror(errno));
    } else {
        jsv8->goresolve(vm, cr, V8_BUFFER(s->body, s->body_len), 1);
        s->body = NULL;
    }
}

static int v8_readp(char *buf, int size, void *p1, int done) {
    struct http_s *s = p1;
    s->body_len += size;
    if (done) {
        /* fprintf(stderr, "body_len = %d, content_len = %d\n",
            s->body_len, s->content_len); */
        s->state = BODY_COMPLETE;
    }
    /* buf memory disowned to V8 */
    jsv8->goresolve(s->vm, s->cr, V8_BUFFER(buf, size), done);
    return 0;
}

/* push data */
static coroutine void
do_http_readp(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    struct http_s *s = V8_TOPTR(args[0]);
    int rc;
    if (s->state != HEADER_COMPLETE) {
        int err = s->state < 0 ? -s->state : EPROTO;
        jsv8->goreject(vm, cr, strerror(err));
        return;
    }
    s->vm = vm;
    s->cr = cr;
    s->ondata = v8_readp;
    do {
        rc = fetch_body(s);
    } while (rc >= 0 && s->state != BODY_COMPLETE);

    if (rc < 0) {
        s->state = -errno;
        jsv8->goreject(vm, cr, strerror(errno));
    }
}

static int
v8_readb(char *buf, int size, void *p1, int done) {
    struct http_s *s = p1;
    s->body_len += size;
    if (done) {
        /* fprintf(stderr, "body_len = %d, content_len = %d\n",
            s->body_len, s->content_len); */
        s->state = BODY_COMPLETE;
    }

    /* buf memory disowned to V8 */
    jsv8->goresolve(s->vm, s->cr, V8_BUFFER(buf, size), done);
    return 0;
}

/* pull data */
static coroutine void
do_http_readb(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    struct http_s *s = V8_TOPTR(args[0]);
    int rc;
    if (s->state != HEADER_COMPLETE) {
        int err = s->state < 0 ? -s->state : EPROTO;
        jsv8->goreject(vm, cr, strerror(err));
        return;
    }
    s->vm = vm;
    s->cr = cr;
    s->ondata = v8_readb;
    rc = fetch_body(s);
    if (rc < 0) {
        s->state = -errno;
        jsv8->goreject(vm, cr, strerror(errno));
    }
}

static v8_val
do_http_create(v8_state vm, int argc, v8_val argv[]) {
    char *host = V8_TOSTR(argv[0]);
    int port = V8_TOINT32(argv[1]);
    int useTLS = V8_TOINT32(argv[3]);
    struct http_s *s = http_create(host, port, useTLS);
    if (s)
        s->deadline = V8_TODOUBLE(argv[2]);
    return V8_PTR(s);
}

/* Close the file descriptor.
 * WARNING: Safe only if fd is not in the pollset.
 *      Should not to be used to cancel a connection.
 *  TODO: execute in a main thread goroutine (fire_and_forget). 
 */
static v8_val
do_http_close(v8_state vm, int argc, v8_val argv[]) {
    struct http_s *s = V8_TOPTR(argv[0]);
    if (s->mfd) /* maybe NULL if connect failed/timed-out */
        s->closefd(s->mfd);
    return V8_VOID;
}

/*
 * Should be safe if there are no reader/writer coroutines.
 */
static v8_val
do_http_free(v8_state vm, int argc, v8_val argv[]) {
    void *ptr = V8_TOPTR(argv[0]);
    http_free(ptr);
    return V8_VOID;
}

static v8_val
do_http_set_deadline(v8_state vm, int argc, v8_val argv[]) {
    struct http_s *s = V8_TOPTR(argv[0]);
    s->deadline = V8_TODOUBLE(argv[1]);
    return V8_VOID;
}

static coroutine void
do_http_listen_and_accept(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    int port = V8_TOINT32(args[0]);
    int mode = V8_TOINT32(args[1]);
    int backlog = V8_TOINT32(args[2]);
    int reuseport = V8_TOINT32(args[3]);
    char *name = V8_TOSTR(args[4]);
    ipaddr address;
    int rc = iplocal(&address, name[0] == '\0' ? NULL : name, port, mode);
    if (rc != 0) {
        jsv8->goreject(vm, cr, strerror(errno));
        return;
    }
    mill_fd lsock = tcplisten(&address,
                backlog <= 0 ? 128 : backlog, reuseport);
    if (!lsock) {
        jsv8->goreject(vm, cr, strerror(errno));
        return;
    }

    while (1) {
        mill_fd csock = tcpaccept(lsock, -1 /* deadline */);
        if (csock) {
            struct http_s *s = emalloc(sizeof (struct http_s));
            if (!s) {
                errno = ENOMEM;
                jsv8->goreject(vm, cr, strerror(errno));
                break;
            }
            memset(s, '\0', sizeof (struct http_s));
            s->mfd = csock;
            s->deadline = -1;
            s->writefunc = mill_write;
            s->readfunc = mill_read;
            s->closefunc = tcpclose;
            s->closefd = mill_fdclose;
            jsv8->goresolve(vm, cr, V8_PTR(s), 0);
        } /* else 
            ignore error .. */
    }
}

static v8_ffn ff_table[] = {
    {4, do_http_create, "create", FN_CTYPE },
    {1, do_http_free, "free", FN_CTYPE },
    {1, do_http_close, "closefd", FN_CTYPE },
    {2, do_http_set_deadline, "set_deadline", FN_CTYPE },
    {1, do_http_connect, "connect", FN_CORO },
    {3, do_http_send, "send", FN_CORO },
    {1, do_http_header, "header", FN_CORO },
    {1, do_http_reada, "reada", FN_CORO },
    {1, do_http_readp, "readp", FN_COROPUSH },
    {1, do_http_readb, "readb", FN_COROPULL },
    {5, do_http_listen_and_accept, "listen_and_accept", FN_COROPUSH },
    {0},
};

int JS_LOAD(v8_state vm, v8_val hlib) {
    JS_EXPORT(ff_table);
}
