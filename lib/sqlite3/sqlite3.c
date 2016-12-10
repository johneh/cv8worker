#include <sqlite3.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "jsv8dlfn.h"

static coroutine void
do_sqlite3_open(v8_state vm, v8_handle hcr, void *ptr) {
    char *filename = ptr;
    sqlite3 *db = NULL;
    if (sqlite3_open(filename, & db) != SQLITE_OK) {
        const char *emsg = sqlite3_errmsg(db);
        sqlite3_close_v2(db);
        v8->goreject(vm, hcr, emsg);
    } else
        v8->goresolve(vm, hcr, db, -1, 1);
}

static coroutine void
do_sqlite3_close(v8_state vm, v8_handle hcr, void *ptr) {
    sqlite3 *db = ptr;
    int ret = sqlite3_close_v2(db);
    if (ret != SQLITE_OK)
        v8->goreject(vm, hcr, sqlite3_errstr(ret));
    else
        v8->goresolve(vm, hcr, NULL, 0, 1);
}


/* XXX: unlike prepare(), can have multiple sql statements seperated by semi */
static coroutine void
do_sqlite3_exec(v8_state vm, v8_handle hcr, void *ptr) {
    struct exec_s {
        sqlite3 *db;
        char *query;
    };
    struct exec_s *exs = ptr;
    char *errmsg;
    if (sqlite3_exec(exs->db, exs->query, 0, 0, & errmsg) != SQLITE_OK) {
        v8->goreject(vm, hcr, errmsg);
        sqlite3_free(errmsg);
    } else
        v8->goresolve(vm, hcr, NULL, 0, 1);
}

static coroutine void
do_sqlite3_prepare(v8_state vm, v8_handle hcr, void *ptr) {
    struct prepare_s {
        sqlite3 *db;
        int sqlen;
        char sql[1];
    } __attribute__((packed));
    struct prepare_s *ps = ptr;
    sqlite3_stmt *stmt;
    int ret = sqlite3_prepare_v2(ps->db, ps->sql, ps->sqlen, & stmt, 0);
    if (ret != SQLITE_OK) {
        const char *emsg = sqlite3_errstr(ret);
        v8->goreject(vm, hcr, emsg);
    } else
        v8->goresolve(vm, hcr, stmt, -1, 1);
}

static coroutine void
do_sqlite3_finalize(v8_state vm, v8_handle hcr, void *ptr) {
    sqlite3_stmt *stmt = ptr;
    int ret = sqlite3_finalize(stmt);
    if (ret != SQLITE_OK)
        v8->goreject(vm, hcr, sqlite3_errstr(ret));
    else
        v8->goresolve(vm, hcr, NULL, 0, 1);
}

static coroutine void
do_sqlite3_reset(v8_state vm, v8_handle hcr, void *ptr) {
    sqlite3_stmt *stmt = ptr;
    int ret = sqlite3_reset(stmt);
    if (ret != SQLITE_OK)
        v8->goreject(vm, hcr, sqlite3_errstr(ret));
    else
        v8->goresolve(vm, hcr, NULL, 0, 1);
}

static coroutine void
do_sqlite3_bind(v8_state vm, v8_handle hcr, void *ptr) {
    struct bind_s {
        sqlite3_stmt *stmt;
        char buf[1]; /* format string + data */
    };
    struct bind_s *bp = ptr;
    char *fmt = bp->buf, *sp;
    int i, ret = SQLITE_OK;
    int nitems = strlen(bp->buf);
    for (i = 1, sp = bp->buf + nitems + 1; i <= nitems; i++, fmt++) {
        switch (*fmt) {
        case 'i':
            ret = sqlite3_bind_int(bp->stmt, i, *((int *) sp));
            sp += sizeof (int);
            break;
        case 'j':
            ret = sqlite3_bind_int64(bp->stmt, i, *((int64_t *) sp));
            sp += 8;
            break;
        case 'd':
            ret = sqlite3_bind_double(bp->stmt, i, *((double *) sp));
            sp += sizeof (double);
            break;
        case 's': {
            int stlen = strlen(sp);
            ret = sqlite3_bind_text(bp->stmt, i, sp, stlen, SQLITE_TRANSIENT);
            sp += (stlen + 1);
        }
            break;
        case 'a': {
            int stlen = *((int *) sp);
            sp += 4;
            ret = sqlite3_bind_blob(bp->stmt, i, sp, stlen, SQLITE_TRANSIENT);
            sp += stlen;
        }
            break;
        case '_':
            ret = sqlite3_bind_null(bp->stmt, i);
            break;
        default:
            ret = SQLITE_ERROR;
            break;
        }
        if (ret != SQLITE_OK) {
            v8->goreject(vm, hcr, sqlite3_errstr(ret));
            return;
        }
    }
    v8->goresolve(vm, hcr, NULL, 0, 1);
}


struct fb_buffer {
    unsigned size;
    unsigned len;
    char *buf;
};

#define SLOP 256

static void
size_buffer(struct fb_buffer *fb, int len) {
    if (!fb->buf) {
        fb->buf = malloc(fb->size = len+SLOP);
        fb->len = 0;
    } else if (len > fb->size - fb->len)
        fb->buf = realloc(fb->buf, fb->size = fb->len + len + SLOP);
    if (!fb->buf) {
        fprintf(stderr, "Out of memory");
        exit(1);
    }
}

static void
init_buffer(struct fb_buffer *fb, int ncols) {
    size_buffer(fb, ncols + 1);
    fb->len = ncols + 1;
    fb->buf[ncols] = '\0';
}

static inline void
set_column_type(struct fb_buffer *fb, int idx, char type) {
    /* 0 <= idx < ncols */ 
    fb->buf[idx] = type;
}

static void
write_column(sqlite3_stmt *stmt, int i, int coltype, struct fb_buffer *b) {
    switch (coltype) {
        case SQLITE_INTEGER:
            /* sqlite3_int64 i64 = sqlite3_column_int64(stmt, i); */
            set_column_type(b, i, 'j');
            size_buffer(b, 8);
            *((int64_t *) &b->buf[b->len]) = sqlite3_column_int64(stmt, i);
            b->len += 8;
            break;
        case SQLITE_FLOAT:
            set_column_type(b, i, 'd');
            size_buffer(b, 8);
            *((double *) &b->buf[b->len]) = sqlite3_column_double(stmt, i);
            b->len += 8;
            break;
        case SQLITE_NULL:
            set_column_type(b, i, '_');
            break;
        case SQLITE3_TEXT: {
            const char *txt = (const char *) sqlite3_column_text(stmt, i);
            int len = sqlite3_column_bytes(stmt, i);
            set_column_type(b, i, 's');
            size_buffer(b, len + 1);
            memcpy(&b->buf[b->len], txt, len);
            b->buf[b->len+=len] = '\0';
            b->len++;
        }
            break;
        default: {
            /* SQLITE_BLOB */
            const void *blob = sqlite3_column_blob(stmt, i);
            int len = sqlite3_column_bytes(stmt, i);
            set_column_type(b, i, 'a');
            size_buffer(b, len+4);
            *((int *) &b->buf[b->len]) = len;
            memcpy(&b->buf[b->len+4], blob, len);
            b->len += (len+4);
        }
    }
}

/* step and a fetch result row */
static coroutine void
do_sqlite3_next(v8_state vm, v8_handle hcr, void *ptr) {
    struct sqlite3_stmt *stmt = ptr;
    int ret = sqlite3_step(stmt);
    if (ret == SQLITE_DONE)
        v8->goresolve(vm, hcr, NULL, 0, 1);
    else if (ret != SQLITE_ROW)
        v8->goreject(vm, hcr, sqlite3_errstr(ret));
    else {
        int i, ncols = sqlite3_column_count(stmt);
        struct fb_buffer fb = {0};
        assert(ncols >= 0); /* ncols == 0 ->  empty result set (UNLIKELY??) */
        init_buffer(&fb, ncols);
        for (i = 0; i < ncols; i++) {
            write_column(stmt, i, sqlite3_column_type(stmt, i), &fb);
        }
        v8->goresolve(vm, hcr, fb.buf, fb.len, 1);
    }
}


/* step and fetch N result rows */
static coroutine void
do_sqlite3_each(v8_state vm, v8_handle hcr, void *ptr) {
    struct each_s {
        struct sqlite3_stmt *stmt;
        unsigned int count;
    };
    struct each_s *rr = ptr;
    int ret = SQLITE_DONE;
    unsigned int n = rr->count;
    while (n > 0 && (ret = sqlite3_step(rr->stmt)) == SQLITE_ROW) {
        int i, ncols = sqlite3_column_count(rr->stmt);
        struct fb_buffer fb = {0};
        assert(ncols > 0);
        init_buffer(&fb, ncols);
        for (i = 0; i < ncols; i++) {
            write_column(rr->stmt, i, sqlite3_column_type(rr->stmt, i), &fb);
        }
        v8->goresolve(vm, hcr, fb.buf, fb.len, 0);
        n--;
    }
    if (n == 0 || ret == SQLITE_DONE)
        v8->goresolve(vm, hcr, NULL, 0, 1);
    else {
        assert(ret != SQLITE_ROW);
        v8->goreject(vm, hcr, sqlite3_errstr(ret));
    }
}

static v8_ffn ff_table[] = {
    {0, do_sqlite3_open, "open", V8_DLCORO},
    {0, do_sqlite3_close, "close", V8_DLCORO},
    {0, do_sqlite3_exec, "exec", V8_DLCORO},
    {0, do_sqlite3_prepare, "prepare", V8_DLCORO},
    {0, do_sqlite3_finalize, "finalize", V8_DLCORO},
    {0, do_sqlite3_reset, "reset", V8_DLCORO},
    {0, do_sqlite3_next, "next", V8_DLCORO},
    {0, do_sqlite3_each, "each", V8_DLCORO},
    {0, do_sqlite3_bind, "bind", V8_DLCORO},
    {0},
};

int JS_LOAD(v8_state vm, v8_handle hlib) {
    JS_EXPORT(ff_table);
}
