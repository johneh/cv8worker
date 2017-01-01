const XEvent = require('../lib/xlib.js').XEvent;
const Window = require('../lib/xlib.js').Window;

function makeWindow(circle) {
    const w1 = new Window();
    w1.create(null, 0, 0, 250, 250);
    w1.setTitle('Hello World!');
    const gc = w1.createGC();
    gc.set('foreground', "red");
    gc.set('line_width', 2);
    w1.addEventListener(XEvent.Expose, function(ev, gc) {
        if (circle)
            this.drawArc(gc, 100, 100, 50, 50, 0, 360 * 64);
        else
            this.drawLine(gc, 25, 50, 200, 50);
    }, gc);

    w1.addEventListener(XEvent.DestroyNotify, function(ev) {
        console.log('Destroyed window:', ev.window);
    });

    w1.background = '#ffffff';
    w1.onWMDelete(function(x) {
        console.log(x, 'onWMDelete ...');
        this.destroy(); // -> DestroyNotify event
    }, 'XWindows test:');

    w1.map();
}

makeWindow(false);
makeWindow(true);

// XEvent.wait();

XEvent.await();
console.log('Waiting for X to quit ...');
setTimeout(function () {
    console.log('timer expired ...');
}, 10000);

