//
// An animated clockEDIT
// Source: https://developer.mozilla.org/en-US/docs/Web/API/Canvas_API/Tutorial/Basic_animations
//

const XEvent = require('../../xlib.js').XEvent;
const Window = require('../../xlib.js').Window;
const cairo = require('../cairo.js').identifiers;
const cairo_xlib = $loadlib('../libcairoxlib.so');
const Canvas = require('../canvas.js');


const pi = Math.PI;

function getX(angle) {
    return - Math.sin(angle + pi);
}

function getY(angle) {
    return Math.cos(angle + pi);
}

class Clock extends Window {
    constructor() {
        super();
        const d = 320;
        this.clock = { width: d, height: d };
        this.create(null, 0, 0, d, d);
        this.setTitle('Clock');
        this.clock.xlib_surface = cairo_xlib.surface_create(Window.DISPLAY,
                        this.window, Window.VISUAL, d, d);
        this.clock.image_canvas = new Canvas(d, d);
        this.clock.isMapped = false;

        this.addEventListener(XEvent.Expose, function(ev) {
            // $print('width =', this.width);
            if (this.clock.isMapped && ev.count === 0) {
                this.draw();
                cairo.surface_flush(this.clock.xlib_surface);
            }
        });

        this.addEventListener(XEvent.MapNotify, function(ev) {
            this.clock.isMapped = true;
        });

        this.onWMDelete(function() {
            this.destroy(); // -> DestroyNotify event
        });

        //this.background = '#f0f0ff';

        this.map();
    }

    draw() {
        let clock = this.clock;
        // cached image context
        let ctx = clock.image_canvas.getContext("2d");

        ctx.fillStyle = "#ffffff";
        ctx.fillRect(0, 0, 320, 320);

        ctx.save();
        ctx.translate(160, 160);
        ctx.beginPath();
        ctx.lineWidth = 14;
        ctx.strokeStyle = "#325fa2";
        ctx.fillStyle = "#eeeeee";

        ctx.arc(0, 0, 142, 0, pi * 2, 1);
        ctx.stroke(false);
        ctx.fillStyle = "#eeeede";
        ctx.fill(false);

        ctx.strokeStyle = "#ff0000";
        // Hour marks
        ctx.lineWidth = 8;
        for (let i = 0; i < 12; i++) {
            let x = getX(pi / 6 * i);
            let y = getY(pi / 6 * i);
            ctx.beginPath();
            ctx.moveTo(x * 100, y * 100);
            ctx.lineTo(x * 125, y * 125);
            ctx.stroke(false);
        }

        // Minute marks
        ctx.lineWidth = 5;
        for (let i = 0; i < 60; i++) {
            if (i % 5 != 0) {
                let x = getX(pi / 30 * i);
                let y = getY(pi / 30 * i);
                ctx.beginPath();
                ctx.moveTo(x * 117, y * 117);
                ctx.lineTo(x * 125, y * 125);
                ctx.stroke(false);
            }
        }

        ctx.fillStyle = "#eeeede";

        let d = new Date();
        let sec = d.getSeconds();
        let min = d.getMinutes();
        let hr  = d.getHours();
        hr = hr >= 12 ? hr - 12 : hr;

        ctx.strokeStyle = "#000000";
        // Write Hours
        let x = getX(hr * (pi / 6) + (pi / 360)*min + (pi / 21600) * sec);
        let y = getY(hr * (pi / 6) + (pi / 360)*min + (pi / 21600) * sec);
        ctx.lineWidth = 14;
        ctx.beginPath();
        ctx.moveTo(x * -20, y * -20);
        ctx.lineTo(x * 80, y * 80);
        ctx.stroke(false);

        // Write minutes
        x = getX((pi / 30) * min + (pi / 1800) * sec);
        y = getY((pi / 30) * min + (pi / 1800) * sec);

        ctx.lineWidth = 10;
        ctx.beginPath();
        ctx.moveTo(x * -28, y * -28);
        ctx.lineTo(x * 112, y * 112);
        ctx.stroke(false);

        // Write seconds
        x = getX(sec * pi / 30);
        y = getY(sec * pi / 30);
        ctx.strokeStyle = "#000000";
        ctx.fillStyle = "#000000";

        ctx.lineWidth = 6;
        ctx.beginPath();
        ctx.moveTo(x * -30, y * -30);
        ctx.lineTo(x * 83, y * 83);
        ctx.stroke(false);
        ctx.beginPath();
        ctx.arc(0, 0, 10, 0, pi * 2, 1);
        ctx.fill(false);
        ctx.beginPath();
        ctx.arc(x * 95, y * 95, 10, 0, pi * 2, 1);
        ctx.stroke(false);

        ctx.fillStyle = "#ff0000";

        ctx.arc(0, 0, 3, 0, pi * 2, 1);
        ctx.fill(false);

        ctx.restore();

        let cr = cairo.create(clock.xlib_surface);
        cairo.set_source_surface(cr, clock.image_canvas.surface, 0, 0);
        cairo.paint(cr);
    }

    static update(c) {
        if (Window.exists(c)) {
            c.draw();
            XEvent.flush();
            setTimeout(Clock.update, 1000, c);
        }
    }

    run() {
        setTimeout(Clock.update, 1000, this);
    }
}

let c = new Clock();
c.run();
XEvent.await();

