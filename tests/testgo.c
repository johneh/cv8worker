#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "libpill.h"

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

void js_panic(v8_state vm) {
    fprintf(stderr, "%s\n", v8_errstr(vm));
    exit(1);
}
#define CHECK(rc, vm) if(!rc) js_panic(vm)

coroutine void write_response(v8_state vm, v8_handle hcr, void *p1) {
    /* 'ps' in JS pack() */
    struct foo_s {
        void *p;
        char data[1];
    } __attribute__((packed));
    struct foo_s *foo = p1;

    mill_fd csock = foo->p;
    char *ptr = foo->data;
    int total = strlen(ptr);

    int rc;
    while (1) {
        rc = mill_write(csock, ptr, total, -1);
        if (rc > 0) {
            ptr += rc;
            total -= rc;
            if (total > 0)
                continue;
            mill_close(csock, 1);
            v8_goresolve(vm, hcr, NULL, 0, 1);
            break;
        }
        // assert(rc < 0);
        if (rc < 0 && errno != EAGAIN) {
            fprintf(stderr, "fd: %d\n", mill_getfd(csock));
            mill_close(csock, 1);
            v8_goreject(vm, hcr, strerror(errno));
            break;
        }
        yield();
    }
}


coroutine void read_request(v8_state vm, v8_handle hcr, void *ptr) {
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
            mill_close(csock, 1);
            v8_goreject(vm, hcr, "error reading from socket");
            break;
        } else {
            assert(errno == EAGAIN || errno == ETIMEDOUT);
        }

        if (msg_length >= 52) {
            buf[msg_length] = '\0';
            v8_goresolve(vm, hcr, strdup(buf), msg_length, 1);
            break;
        }
    }
}

// csock freed in C code unlike req and resp
static char script[] = "\
$go(listen_and_accept, $nullptr, function (csock, done) {\n\
        $go(read_request, csock)\n\
        .then(function(req) {\n\
var data = 'HTTP/1.1 200 OK\\r\\nContent-Length: 3\\r\\nConnection: close\\r\\n\\r\\nOk\\n';\n\
var resp = $malloc($nullptr.packSize('ps', csock, data)).gc();\n\
resp.pack(0, 'ps', csock, data);\n\
            return $go(write_response, resp);\n\
        })\n\
        .then(function(data) {\n\
        })\n\
        .catch(function(err) {\n\
            $print(err);\n\
        });\n\
});";


coroutine void listen_and_accept(v8_state vm, v8_handle hcr, void *ptr) {
    ipaddr address;
    int rc = iplocal(&address, NULL, 5555, 0);
    assert(rc == 0);
    mill_fd lsock = tcplisten(&address, 300, 0);
    assert(lsock);
    while(1) {
        mill_fd csock = tcpaccept(lsock, -1);
        if (csock)
            v8_goresolve(vm, hcr, csock, -1, 0);
    }
}

void setup(js_vm *vm) {
    v8_handle hcr = v8_go(vm, listen_and_accept);
    assert(hcr);
    v8_set(vm, v8_global(vm), "listen_and_accept", hcr);
    v8_reset(vm, hcr);

    hcr = v8_go(vm, read_request);
    assert(hcr);
    v8_set(vm, v8_global(vm), "read_request", hcr);
    v8_reset(vm, hcr);

    hcr = v8_go(vm, write_response);
    assert(hcr);
    v8_set(vm, v8_global(vm), "write_response", hcr);
    v8_reset(vm, hcr);
}


int main(int argc, char *argv[]) {
    mill_init(-1, 0);
    mill_worker w = mill_worker_create();
    v8_state vm = js_vmopen(w);

    setup(vm);
    int rc = v8_run(vm, script);
    CHECK(rc, vm);

    js_vmclose(vm);
    mill_worker_delete(w);
    mill_fini();
    return 0;
}
