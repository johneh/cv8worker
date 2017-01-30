#include <signal.h>
#include <sys/signalfd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "libpill.h"
#include "jsv8dlfn.h"

/* XXX: possible alternative to using signalfd() is sigwaitinfo()
 * in a separate thread. */

struct signalfd_s {
    mill_fd mfd;
    sigset_t mask;  /* set of signals handled by the fd */
    int sfd;
};

#define SHUTDOWN_SIG SIGUSR1

// N.B.: all signals blocked in the main thread.
static v8_val
do_signalfd_add(v8_state vm, int argc, v8_val args[]) {
    struct signalfd_s *s = V8_TOPTR(args[0]);
    int signo = V8_TOINT32(args[1]);
    int sfd, save_errno = 0;

    /* Block signal (in V8 thread) so that it isn't handled
     * according to the default disposition */
    sigset_t set;
    sigemptyset(&set);
    if (sigaddset(&set, signo) == -1 ||
            pthread_sigmask(SIG_BLOCK, &set, NULL) == -1
    ) {
        return V8_INT32(-1);
    }

    sigaddset(&s->mask, signo);
    sfd = signalfd(s->sfd, &s->mask, SFD_CLOEXEC);
    if (sfd == -1) {
        save_errno = errno;
        goto er1;
    }

    if (s->sfd == -1) {
        if (!(s->mfd = mill_open(sfd))) {
            save_errno = errno;
            close(sfd);
            goto er1;
        }
        s->sfd = sfd;
    }

    return V8_INT32(0);
er1:
    assert(save_errno > 0);
    sigdelset(&s->mask, signo);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL); // XXX: warn on failure?
    errno = save_errno;
    return V8_INT32(-1);
}

// N.B.: all signals blocked in the main thread.
static v8_val
do_signalfd_del(v8_state vm, int argc, v8_val args[]) {
    struct signalfd_s *s = V8_TOPTR(args[0]);
    int signo = V8_TOINT32(args[1]);
    int sfd;
    if (s->sfd == -1 || sigismember(&s->mask, signo) <= 0
        || signo == SHUTDOWN_SIG
    ) {
        return V8_INT32(0);
    }

    sigdelset(&s->mask, signo);
    sfd = signalfd(s->sfd, &s->mask, SFD_CLOEXEC);
    if (sfd == -1) {
        int save_errno = errno;
        sigaddset(&s->mask, signo);
        errno = save_errno;
        return V8_INT32(-1);
    } else {
        /* unblock the signal in the V8 thread */
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, signo);
        pthread_sigmask(SIG_UNBLOCK, &set, NULL); // XXX: warn on failure
        return V8_INT32(0);
    }
}

// man signalfd;
//  struct signalfd_siginfo {
//      uint32_t ssi_signo;   /* Signal number */
//      ....
//


static coroutine void
signalfd_await(v8_state vm, struct signalfd_s *s, v8_val fn) {

    assert(s->sfd >= 0);

    /* Block shutdown signal (in V8 thread) so that it isn't handled
     * according to the default disposition */
    sigset_t set;
    sigemptyset(&set);
    if (sigaddset(&set, SHUTDOWN_SIG) == -1 ||
            pthread_sigmask(SIG_BLOCK, &set, NULL) == -1
    ) {
        fprintf(stderr, "%s\n", strerror(errno));
    }
    sigaddset(&s->mask, SHUTDOWN_SIG);
    (void) signalfd(s->sfd, &s->mask, SFD_CLOEXEC);

    int fdsi_size = sizeof (struct signalfd_siginfo);
    struct signalfd_siginfo sinfo;

    for (;;) {
        // struct signalfd_siginfo *fdsi = malloc(fdsi_size);
        struct signalfd_siginfo *fdsi = &sinfo;
        int rc, n = 0;
again:
        rc = mill_read(s->mfd, (char *)fdsi + n, fdsi_size - n, -1);
        if (rc <= 0)
            break;
        n += rc;
        if (n < fdsi_size)
            goto again;
        if (fdsi->ssi_signo == SHUTDOWN_SIG
                    && fdsi->ssi_ptr == (uint64_t)(uintptr_t) s)
            break;  // exiting ...

        jsv8->task_(vm, fn, V8_INT32(fdsi->ssi_signo));
    }
}

static v8_val
do_signalfd_await(v8_state vm, int argc, v8_val args[]) {
    struct signalfd_s *s = V8_TOPTR(args[0]);
    int hfn = V8_TOINT32(args[1]);
    go(signalfd_await(vm, s, V8_HANDLE(hfn)));
    return V8_VOID;
}

static v8_val
do_signalfd_close(v8_state vm, int argc, v8_val args[]) {
    struct signalfd_s *s = V8_TOPTR(args[0]);
    union sigval v;
    v.sival_ptr = s;
    (void) sigqueue(getpid(), SHUTDOWN_SIG, v);
    return V8_VOID;
}

static v8_val
do_signalfd_create(v8_state vm, int argc, v8_val argv[]) {
    struct signalfd_s *s = malloc(sizeof(struct signalfd_s));
    if (s) {
        s->mfd = NULL;
        s->sfd = -1;
        sigemptyset(&s->mask);
    }
    return V8_PTR(s);
}

#if 0
static v8_val
do_signalfd_free(v8_state vm, int argc, v8_val argv[]) {
    struct signalfd_s *s = V8_TOPTR(argv[0]);
    if(s->mfd)  // XXX: should have called mill_fdclose() in the main thread!
        mill_close(s->mfd, 1);
    free(s);
    return V8_VOID;
}
#endif

static v8_ffn ff_table[] = {
    { 0, do_signalfd_create, "sfd_create", FN_CTYPE },
    { 2, do_signalfd_add, "sfd_add", FN_CTYPE },
    { 2, do_signalfd_del, "sfd_del", FN_CTYPE },
    { 1, do_signalfd_close, "sfd_close", FN_CTYPE },
    { 2, do_signalfd_await, "sfd_await", FN_CTYPE },
};

int JS_LOAD(v8_state vm, v8_val hlib) {
    JS_EXPORT(ff_table);
}

