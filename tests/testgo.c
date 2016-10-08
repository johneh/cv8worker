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

void js_panic(js_vm *vm) {
    fprintf(stderr, "%s\n", js_errstr(vm));
    exit(1);
}
#define CHECK(rc, vm) if(!rc) js_panic(vm)

static char resp[] = "HTTP/1.1 200 OK\r\nContent-Length: 3\r\nConnection: close\r\n\r\nOk\n";

coroutine void write_response(js_vm *vm, js_handle *cr, js_handle *hin) {
//    mill_fd csock = js_topointer(hin);
    js_handle *h1 = js_get(hin, "sock");
    mill_fd csock = js_topointer(h1);
    assert(csock);
    js_reset(h1);

    char *ptr = resp;
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
    mill_close(csock, 1);   // XXX: close in JS?
    /* js_gosend(cr, js_int32(vm, 1)); */
    js_godone(cr);
}


coroutine void read_request(js_vm *vm, js_handle *cr, js_handle *hin) {
    mill_fd csock = js_topointer(hin);
    assert(csock);
    int msg_length = 0;
    while(1) {
        char buf[512] = {0};
        int num_bytes_read = mill_read(csock, buf, sizeof(buf), now()+1);
        if (num_bytes_read > 0)
            msg_length += num_bytes_read;
        else if (num_bytes_read == 0 || errno == ECONNRESET) {
            js_goerr(cr, js_string(vm, "error reading from socket", -1));
            mill_close(csock, 1);   // XXX: for now
            break;
        } else {
            assert(errno == EAGAIN || errno == ETIMEDOUT);
        }

        if(msg_length >= 52) {
            js_gosend(cr, js_string(vm, buf, msg_length));
            break;
        }
    }
    js_godone(cr);
}

static char script[] = "\
$go(listen_and_serve, null, function (err, csock) {\n\
        if (err !== null) {\n\
            $print(err);\n\
            return;\n\
        }\n\
        $go(read_request, csock, function(err, req) {\n\
                if (err !== null)\n\
                    $print(err);\n\
                else {\n\
                    var resp = {\
    sock : csock,\n\
    data : 'HTTP/1.1 200 OK\\r\\nContent-Length: 3\\r\\nConnection: close\\r\\n\\r\\nOk\\n'\n\
                    };\n\
                    $go(write_response, resp, function(err, req) {\n\
                        // close socket here.\n\
                    });\n\
                }\n\
            }\n\
        );\n\
});";


coroutine void listen_and_serve(js_vm *vm, js_handle *cr, js_handle *hin) {
    ipaddr address;
    int rc = iplocal(&address, NULL, 5555, 0);
    assert(rc == 0);
    mill_fd lsock = tcplisten(&address, 300, 0);
    assert(lsock);
    while(1) {
        mill_fd csock = tcpaccept(lsock, -1);
        if (! csock)
            js_goerr(cr, js_string(vm, strerror(errno), -1));
        else
            js_gosend(cr, js_pointer(vm, csock));
    }
}


void setup(js_vm *vm) {
    js_handle *hcr = js_go(vm, listen_and_serve);
    assert(hcr);
    js_set(JSGLOBAL(vm), "listen_and_serve", hcr);
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
