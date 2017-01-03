#include <sqlite3.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "jsv8dlfn.h"
#include "libpill.h"

coroutine static void
do_sqlite3_open(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    char *filename = V8_TOSTR(args[0]);
    sqlite3 *db = NULL;
    if (sqlite3_open(filename, & db) != SQLITE_OK) {
        const char *emsg = sqlite3_errmsg(db);
        sqlite3_close_v2(db);
        jsv8->goreject(vm, cr, emsg);
    } else
        jsv8->goresolve(vm, cr, V8_PTR(db), 1);
}

coroutine static void
do_sqlite3_close(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    sqlite3 *db = V8_TOPTR(args[0]);
    int ret = sqlite3_close_v2(db);
    if (ret != SQLITE_OK)
        jsv8->goreject(vm, cr, sqlite3_errstr(ret));
    else
        jsv8->goresolve(vm, cr, V8_VOID, 1);
}

/* XXX: unlike prepare(), can have multiple sql statements seperated by semi */
coroutine static void
do_sqlite3_exec(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    sqlite3 *db = V8_TOPTR(args[0]);
    char *query = V8_TOSTR(args[1]);
    char *errmsg;
    if (sqlite3_exec(db, query, 0, 0, & errmsg) != SQLITE_OK) {
        jsv8->goreject(vm, cr, errmsg);
        sqlite3_free(errmsg);
    } else
        jsv8->goresolve(vm, cr, V8_VOID, 1);
}

coroutine static void
do_sqlite3_prepare(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    sqlite3 *db = V8_TOPTR(args[0]);
    char *query = V8_TOSTR(args[1]);
    int qlen = V8_TOINT32(args[2]);
    sqlite3_stmt *stmt;
    int ret = sqlite3_prepare_v2(db, query, qlen, & stmt, 0);
    if (ret != SQLITE_OK) {
        const char *emsg = sqlite3_errstr(ret);
        jsv8->goreject(vm, cr, emsg);
    } else
        jsv8->goresolve(vm, cr, V8_PTR(stmt), 1);
}

coroutine static void
do_sqlite3_finalize(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    sqlite3_stmt *stmt = V8_TOPTR(args[0]);
    int ret = sqlite3_finalize(stmt);
    if (ret != SQLITE_OK)
        jsv8->goreject(vm, cr, sqlite3_errstr(ret));
    else
        jsv8->goresolve(vm, cr, V8_VOID, 1);
}

coroutine static void
do_sqlite3_reset(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    sqlite3_stmt *stmt = V8_TOPTR(args[0]);
    int ret = sqlite3_reset(stmt);
    if (ret != SQLITE_OK)
        jsv8->goreject(vm, cr, sqlite3_errstr(ret));
    else
        jsv8->goresolve(vm, cr, V8_VOID, 1);
}

coroutine static void
do_sqlite3_bind(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    sqlite3_stmt *stmt = V8_TOPTR(args[0]);
    char *fmt = V8_TOSTR(args[1]);
    int nitems = strlen(fmt);
    char *bp = V8_TOPTR(args[2]);
    int i, ret = SQLITE_OK;
    char *sp;
    for (i = 1, sp = bp; i <= nitems; i++, fmt++) {
        switch (*fmt) {
        case 'i':
            ret = sqlite3_bind_int(stmt, i, *((int *) sp));
            sp += sizeof (int);
            break;
        case 'j':
            ret = sqlite3_bind_int64(stmt, i, *((int64_t *) sp));
            sp += 8;
            break;
        case 'd':
            ret = sqlite3_bind_double(stmt, i, *((double *) sp));
            sp += sizeof (double);
            break;
        case 's': {
            // FIXME don't use strlen, pass len + data AND pass length as
            // 4th parameter to bind_text
            int stlen = strlen(sp);
            ret = sqlite3_bind_text(stmt, i, sp, stlen, SQLITE_TRANSIENT);
            sp += (stlen + 1);
        }
            break;
        case 'a': {
            int stlen = *((int *) sp);
            sp += 4;
            ret = sqlite3_bind_blob(stmt, i, sp, stlen, SQLITE_TRANSIENT);
            sp += stlen;
        }
            break;
        case '_':
            ret = sqlite3_bind_null(stmt, i);
            break;
        default:
            ret = SQLITE_ERROR;
            break;
        }
        if (ret != SQLITE_OK) {
            jsv8->goreject(vm, cr, sqlite3_errstr(ret));
            return;
        }
    }
    jsv8->goresolve(vm, cr, V8_VOID, 1);
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
            set_column_type(b, i, 's'); // FIXME use sized string
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

/* step and fetch one result row */
coroutine static void
do_sqlite3_next(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    struct sqlite3_stmt *stmt = V8_TOPTR(args[0]);
    int ret = sqlite3_step(stmt);
    if (ret == SQLITE_DONE)
        jsv8->goresolve(vm, cr, V8_NULL, 1);
    else if (ret != SQLITE_ROW)
        jsv8->goreject(vm, cr, sqlite3_errstr(ret));
    else {
        int i, ncols = sqlite3_column_count(stmt);
        struct fb_buffer fb = {0};
        assert(ncols >= 0); /* ncols == 0 ->  empty result set (UNLIKELY??) */
        init_buffer(&fb, ncols);
        for (i = 0; i < ncols; i++) {
            write_column(stmt, i, sqlite3_column_type(stmt, i), &fb);
        }
        jsv8->goresolve(vm, cr, V8_BUFFER(fb.buf, fb.len), 1);
    }
}


/* step and fetch N result rows */
coroutine static void
do_sqlite3_each(v8_state vm, v8_coro cr, int argc, v8_val args[]) {
    struct sqlite3_stmt *stmt = V8_TOPTR(args[0]);
    unsigned int max_rows = V8_TOUINT32(args[1]);
    unsigned int batch_rows = V8_TOUINT32(args[2]);
    int ret = SQLITE_DONE;
    unsigned int n = max_rows, count = 0;
    if (batch_rows == 0)
        batch_rows = max_rows;
    while (n > 0 && (ret = sqlite3_step(stmt)) == SQLITE_ROW) {
        int i, ncols = sqlite3_column_count(stmt);
        struct fb_buffer fb = {0};
        assert(ncols > 0);
        init_buffer(&fb, ncols);
        for (i = 0; i < ncols; i++) {
            write_column(stmt, i, sqlite3_column_type(stmt, i), &fb);
        }
        jsv8->goresolve(vm, cr, V8_BUFFER(fb.buf, fb.len), 0);
        n--;
        if (++count == batch_rows) {
            count = 0;
            mill_yield();
        }
    }
    if (n == 0 || ret == SQLITE_DONE)
        jsv8->goresolve(vm, cr, V8_NULL, 1);
    else {
        assert(ret != SQLITE_ROW);
        jsv8->goreject(vm, cr, sqlite3_errstr(ret));
    }
}

static v8_ffn ff_table[] = {
    {1, do_sqlite3_open, "open", FN_CORO},
    {1, do_sqlite3_close, "close", FN_CORO},
    {2, do_sqlite3_exec, "exec", FN_CORO},
    {3, do_sqlite3_prepare, "prepare", FN_CORO},
    {1, do_sqlite3_finalize, "finalize", FN_CORO},
    {1, do_sqlite3_reset, "reset", FN_CORO},
    {1, do_sqlite3_next, "next", FN_CORO},
    {3, do_sqlite3_each, "each", FN_COROPUSH},
    {3, do_sqlite3_bind, "bind", FN_CORO},
    {0},
};

int JS_LOAD(v8_state vm, v8_val hlib) {
    JS_EXPORT(ff_table);
}
