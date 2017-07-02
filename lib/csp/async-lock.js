/*
a_lock = new AsyncLock();

await a_lock.acquire();
try {
  // protected code ..
}

finally {
  a_lock.release();
}
*/

const priv = new WeakMap;

class AsyncLock {
  constructor() {
    priv.set(this, {
      locked: false,
      waitQ: []
    });
  }

  acquire() {
    let { locked, waitQ } = priv.get(this);
    if (!locked) {
      priv.get(this).locked = true;
      return Promise.resolve(true);
    }
    return new Promise(resolve => {
      waitQ.push(resolve);
    });
  }

  release() {
    let { locked, waitQ } = priv.get(this);
    if (!locked)
      throw new Error('AsyncLock has not been locked'); 
    priv.get(this).locked = false;
    if (waitQ.length) {
      let resolve = waitQ.shift();
      priv.get(this).locked = true;
      resolve(true);
    }
  }

  async do(fn, ...args) {
    await this.acquire();
    try {
      //fn.call(null, ...args);
      if (fn.constructor.name === 'AsyncFunction')
        await fn(...args);
      else
        fn(...args);
    } catch (e) {
      throw e;
    } finally {
      this.release();
    }
  }
}

module.exports = AsyncLock;

