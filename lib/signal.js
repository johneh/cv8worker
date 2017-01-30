const lib = $loadlib('./os/libsignal.so');

let s = {
    SIGHUP: 1,
    SIGINT: 2,
    SIGQUIT: 3,
    SIGILL: 4,
    SIGTRAP: 5,
    SIGABRT: 6,
    SIGIOT: 6,
    SIGBUS: 7,
    SIGFPE: 8,
    SIGKILL: 9,
    SIGUSR1: 10,
    SIGSEGV: 11,
    SIGUSR2: 12,
    SIGPIPE: 13,
    SIGALRM: 14,
    SIGTERM: 15,
    SIGSTKFLT: 16,
    SIGCHLD: 17,
    SIGCONT: 18,
    SIGSTOP: 19,
    SIGTSTP: 20,
    SIGTTIN: 21,
    SIGTTOU: 22,
    SIGURG: 23,
    SIGXCPU: 24,
    SIGXFSZ: 25,
    SIGVTALRM: 26,
    SIGPROF: 27,
    SIGWINCH: 28,
    SIGIO: 29,
    SIGLOST: 29,
    SIGPWR: 30,
    SIGSYS: 31,
    SIGUNUSED: 31,
    SIGRTMIN:32,
};

const SIGRTMAX = 64;

function SignalFD() {
    this._sptr = lib.sfd_create().notNull();
    this._task = null;
    this._sigs = new Array(SIGRTMAX);
}

SignalFD.prototype.add = function (signo, fn) {
    if ((signo|0) !== signo || signo <= 0 || signo > SIGRTMAX
            || signo === s.SIGKILL || signo === s.SIGSTOP)
        throw new Error('not a valid signal number');
    if (typeof fn !== 'function')
        throw new TypeError('not a function');

    if (typeof this._sigs[signo] !== 'undefined') {
        const sa = this._sigs[signo];
        sa.push(fn);
        return;
    }

    lib.sfd_add(this._sptr, signo);
    const sa = this._sigs[signo] = [];
    sa.push(fn);

    const self = this;
    if (!self._task) {
        self._task = function (err, signo) {
            // console.log(`Got signal ${signo}`);
            const sa = self._sigs[signo];
            for (let i = 0; i < sa.length; i++) {
                sa[i](signo);
            }
        };

        lib.sfd_await(self._sptr, $$persist.set(self._task));
    }
};

//SignalFD.prototype.remove = function (signo) {
//    if ((signo|0) !== signo || signo <= 0 || signo > SIGRTMAX)
//        throw new Error('not a valid signal number');
//    lib.sfd_del(this._sptr, signo);
//};

SignalFD.prototype.shutdown = function () {
    lib.sfd_close(this._sptr);
};

const SFD = new SignalFD();

module.exports = s;
module.exports.on = function (signo, fn) {
    SFD.add(signo, fn);
};

//exports.remove = function (signo) {
//    SFD.remove(signo);
//};

$atExit(function() {
    SFD.shutdown();
});

/* Test

const sig = require('./signal.js');
sig.on(sig.SIGINT, (signo) => {
        console.log('Got signal', signo);
});
setTimeout(function(){}, 10000);

*/
