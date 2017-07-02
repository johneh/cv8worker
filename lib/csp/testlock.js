const AsyncLock = require('./async-lock.js');

let timeout = function (delay) {
    return new Promise((resolve) => {
        setTimeout(resolve, delay);
    });
};

let l1 = new AsyncLock();

/*
(async function() {
    await l1.acquire();
    console.log('1: Acquired lock, working for 5 secs ...');
    await timeout(5000);
    console.log('1: Release lock ...');
    l1.release();
})();
*/

l1.do(async function() {
    console.log('1: Acquired lock, working for 5 secs ...');
    await timeout(5000);
    console.log('1: Releasing lock ...');
});

async function foo(i) {
    await timeout(i*100);
    console.log(`${i}: Try lock ...`);
    await l1.acquire();
    console.log(`${i}: Acquired lock ...`);
    await timeout(i*200);
    l1.release();
    console.log(`${i}: Release lock ...`);
}

foo(2);
foo(3);
foo(4);


