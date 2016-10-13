#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "libmill.h"

#include "jsv8.h"

// Adapted from a test program written by https://github.com/benjolitz:
// Apache bench sends 52 bytes every time per request.
// Let's do a simple counter.
// I expect you will run this server and use:
// ab -c 150 -n 10000  http://localhost:5555/
// To torture it.

// Statistics
//============ 
// ab -c 5 -n 10000
// Requests per second:    17027.33 [#/sec] (mean)
// Time per request:       0.294 [ms] (mean)
// Time per request:       0.059 [ms] (mean, across all concurrent requests)
// Transfer rate:          997.7 [Kbytes/sec] received
//

void js_panic(js_vm *vm) {
    fprintf(stderr, "%s\n", js_errstr(vm));
    exit(1);
}
#define CHECK(rc, vm) if(!rc) js_panic(vm)

coroutine void write_response(js_vm *vm, handle_t hcr, void *p1) {
    /* 'ps' in JS pack() */
    struct foo_s {
        void *p;
        char data[1];
    } __attribute__((packed));
    struct foo_s *foo = p1;

#if 0
    mill_fd csock = *((void **) p1);
    assert(csock);
    char *ptr = (char *)p1 + sizeof (csock);
    assert(ptr == foo->data);
    assert(csock == foo->csock);
    int total = strlen(ptr);
#endif
    mill_fd csock = foo->p;
    char *ptr = foo->data;
    int total = strlen(ptr);

    int rc;
again:
    rc = mill_write(csock, ptr, total, -1);
    if (rc > 0) {
        ptr += rc;
        total -= rc;
        if (total > 0)
            goto again;
        goto finish;
    }
    assert(rc < 0 && errno == EAGAIN);
    yield();
    goto again;
finish:
    {
    int fd = mill_getfd(csock);
    mill_fdclean(csock);
    close(fd);
#if 0
    free(p1);   // DO NOT use the pointer var in JS !
#else
    // free in JS (See JS below). Also free csock in JS?
    js_gosend(vm, hcr, NULL);
#endif
    }

    js_godone(vm, hcr);
}


coroutine void read_request(js_vm *vm, handle_t hcr, void *ptr) {
    mill_fd csock = ptr;
    assert(csock);
    int msg_length = 0;
    while (1) {
        char buf[512] = {0};
        int num_bytes_read = mill_read(csock,
                    buf+msg_length, sizeof(buf) - 1 - msg_length, now()+1);
        if (num_bytes_read > 0)
            msg_length += num_bytes_read;
        else if (num_bytes_read == 0 || errno == ECONNRESET) {
            js_goerr(vm, hcr, "error reading from socket");
            mill_close(csock, 1);   // XXX: for now
            break;
        } else {
            assert(errno == EAGAIN || errno == ETIMEDOUT);
        }

        if (msg_length >= 52) {
            buf[msg_length] = '\0';
            js_gosend(vm, hcr, strdup(buf));
            break;
        }
    }
    js_godone(vm, hcr);
}

static char script[] = "\
$go(listen_and_accept, $nullptr, function (err, csock) {\n\
        if (err !== null) {\n\
            $print(err);\n\
            return;\n\
        }\n\
        $go(read_request, csock, function(err, req) {\n\
                if (err !== null)\n\
                    $print(err);\n\
                else {\n\
var data = 'HTTP/1.1 200 OK\\r\\nContent-Length: 3\\r\\nConnection: close\\r\\n\\r\\nOk\\n';\n\
var resp = $malloc($nullptr.packSize('ps', csock, data));\n\
resp.pack(0, 'ps', csock, data);\n\
                    $go(write_response, resp, function(err, data) {\n\
                        // close socket here.\n\
                        resp.free();\n\
                    });\n\
                }\n\
            }\n\
        );\n\
});";


coroutine void listen_and_accept(js_vm *vm, handle_t hcr, void *ptr) {
    ipaddr address;
    int rc = iplocal(&address, NULL, 5555, 0);
    assert(rc == 0);
    mill_fd lsock = tcplisten(&address, 300, 0);
    assert(lsock);
    while(1) {
        mill_fd csock = tcpaccept(lsock, -1);
        if (! csock)
            js_goerr(vm, hcr, strerror(errno));
        else
            js_gosend(vm, hcr, csock);
    }
}


void setup(js_vm *vm) {
    js_handle *hcr = js_go(vm, listen_and_accept);
    assert(hcr);
    js_set(JSGLOBAL(vm), "listen_and_accept", hcr);
    js_reset(hcr);

    hcr = js_go(vm, read_request);
    assert(hcr);
    js_set(JSGLOBAL(vm), "read_request", hcr);
    js_reset(hcr);

    hcr = js_go(vm, write_response);
    assert(hcr);
    js_set(JSGLOBAL(vm), "write_response", hcr);
    js_reset(hcr);
}


int main(int argc, char *argv[]) {
    mill_init(-1, 0);
    mill_worker w = mill_worker_create();
    js_vm *vm = js_vmopen(w);

    setup(vm);
    int rc = js_run(vm, script);
    CHECK(rc, vm);

    js_vmclose(vm);
    mill_worker_delete(w);
    mill_fini();
    return 0;
}
