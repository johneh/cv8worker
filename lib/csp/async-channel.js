const priv = new WeakMap;

class Channel {
    constructor(sz) {
        if (typeof sz !== 'undefined') {
            sz = sz|0;
            if (sz <= 0)
                throw new TypeError('invalid buffer size');
        } else {
            sz = Math.pow(2, 32)-1; // UINT32_MAX
        }
        priv.set(this, {
            consumerQ: [],
            itemQ: [],
            producerQ: [],
            bufSize: sz,
            done: false,
            itemDone: null
        });
    }

    put(item) {
        let p = priv.get(this);
        let { consumerQ, itemQ, producerQ, bufSize } = p;
  
        if (p.done) {
            throw new Error('attempt to put item on a closed channel');
        }

        if (bufSize !== 0 && itemQ.length === bufSize) {
            // console.log('Producer blocked ....');
            return new Promise(resolve => {
                producerQ.push({ resolve, item });
            });
        }

        if (consumerQ.length) {
            let aConsumer = consumerQ.shift();
            aConsumer.resolve(item);
        } else {
            itemQ.push(item);
        }
        return Promise.resolve(true);
    }

    take() {
        let p = priv.get(this);
        let { consumerQ, itemQ, producerQ } = p;

        if (itemQ.length) {
            let item = itemQ.shift();
            if (producerQ.length) {
                let aProducer = producerQ.shift();
                itemQ.push(aProducer.item);
                // console.log('Producer unblocked ...');
                aProducer.resolve(true);
            }
            return Promise.resolve(item);
        }
        if (p.done) {
            return Promise.resolve(p.itemDone);
        }
        return new Promise(resolve => {
                consumerQ.push({ resolve });
            });
    }

    close(val = null) {
        let p = priv.get(this);
        if (p.done)
            throw new Error('attempt to close an already closed channel');
        p.done = true;
        p.itemDone = val;
    }
}

module.exports = Channel;

// Example
/*

const Channel = require('./async-channel.js');

let msleep = function (delay) {
    return new Promise((resolve) => {
        setTimeout(() => {
            resolve(true);
        }, delay);
    });
};

let ch1 = new Channel();
let ch2 = new Channel();

(async function() {
    let r = await Promise.all( [ ch1.take(), ch2.take() ]);
    console.log(r[0], r[1]);
})();

// go select
(async function() {
    let r = await Promise.race( [ ch1.take(), ch2.take() ]);
    console.log(r);
})();

(async function() {
    await msleep(1000);
    await ch2.put(321);
})();

(async function() {
    await msleep(2000);
    await ch1.put(123);
})();

(async function() {
    await msleep(5000);
    await ch1.put(99);
    await ch2.put(999);
})();

*/
