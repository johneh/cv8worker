const XEvent = require('../lib/xlib.js').XEvent;
const Window = require('../lib/xlib.js').Window;

(function () {
    const w1 = new Window();
    w1.create(null, 0, 0, 250, 250);
    const gc = w1.createGC();
    gc.set('foreground', "red");
    gc.set('line_width', 2);
    w1.addEventListener(XEvent.Expose, function(ev, gc) {
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
    XEvent.wait();
})();

