// Drawing with Cairo in GTK# II
//  http://zetcode.com/gui/gtksharp/drawingII/

const XEvent = require('../../xlib.js').XEvent;
const Window = require('../../xlib.js').Window;
const cairo = require('../cairo.js');

class Spinner extends Window {
    constructor(parent, x, y, d) {
        // d - diameter
        super();
        this.spinner = { width: d, height: d, counter: 0 };
        this.borderWidth = 0;
        this.create(parent, x, y, d, d);

        // Set window background pixmap to ParentRelative

        this.setBackgroundPixmap(Window.ParentRelative);

        this.spinner.xs = cairo.xlib.surface_create(Window.DISPLAY,
                        this.window, Window.VISUAL, d, d);

        this.isMapped = false;

        this.addEventListener(XEvent.Expose, function(ev) {
            if (this.isMapped && ev.count === 0) {
                this.draw();
            }
        });

        this.addEventListener(XEvent.MapNotify, function(ev) {
            this.isMapped = true;
        });

        this.map();
    }

    draw() {
        this.clear();   // fills window with background Pixmap

        let spinner = this.spinner;
        let cr = cairo.create(spinner.xs);
        cairo.set_line_width(cr, 2.5);
        cairo.set_line_cap(cr, cairo.CAIRO_LINE_CAP_ROUND);

        let d = spinner.width;
        cairo.translate(cr, d/2, d/2);

        let a = Spinner.trs[spinner.counter % 8];
        cairo.push_group(cr); // OPTIONAL
        for (let i = 0; i < 8; i++) {
            cairo.set_source_rgba(cr, 0, 0, 0, a[i]);
            cairo.move_to(cr, 0.0, -10.0);
            cairo.line_to(cr, 0.0, - d/2);
            cairo.stroke(cr);
            cairo.rotate(cr, Math.PI/4);
        }

        cairo.pop_group_to_source(cr); // only if using push_group
        cairo.paint(cr); // Ditto
    }

    static _update(sw) {
        if (Window.exists(sw) && sw.isMapped) {
            sw.spinner.counter++;
            sw.draw();
            XEvent.flush();
            setTimeout(Spinner._update, 100, sw);
        }
    }

    start() {
        setTimeout(Spinner._update, 100, this);
    }

    stop() {
        this.destroy();
    }
}

Spinner.trs = [
    [ 0.0, 0.15, 0.30, 0.5, 0.65, 0.80, 0.9, 1.0 ],
    [ 1.0, 0.0,  0.15, 0.30, 0.5, 0.65, 0.8, 0.9 ],
    [ 0.9, 1.0,  0.0,  0.15, 0.3, 0.5, 0.65, 0.8 ],
    [ 0.8, 0.9,  1.0,  0.0,  0.15, 0.3, 0.5, 0.65 ],
    [ 0.65, 0.8, 0.9,  1.0,  0.0,  0.15, 0.3, 0.5 ],
    [ 0.5, 0.65, 0.8, 0.9, 1.0,  0.0,  0.15, 0.3 ],
    [ 0.3, 0.5, 0.65, 0.8, 0.9, 1.0,  0.0,  0.15 ],
    [ 0.15, 0.3, 0.5, 0.65, 0.8, 0.9, 1.0,  0.0, ]
];

///////////////////////////////////////////////////

let w1 = new Window();
w1.create(null, 0, 0, 400, 400);
w1.background = '#f0f0ff';
w1.onWMDelete(function() {
    this.destroy(); // -> DestroyNotify event
});

const gc = w1.createGC();
gc.set('foreground', "red");
gc.set('line_width', 2);
w1.addEventListener(XEvent.Expose, function(ev, gc) {
    this.drawArc(gc, 10, 10, 50, 50, 0, 360 * 64);
}, gc);
w1.map();


let s = new Spinner(w1, 25, 25, 50);
s.start();

XEvent.await();

setTimeout(function() {
    s.stop();
}, 10000);
