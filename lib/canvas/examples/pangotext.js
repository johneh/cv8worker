const XEvent = require('../../xlib.js').XEvent;
const Window = require('../../xlib.js').Window;
const Canvas = require('../canvas.js');
const pango = require('../pango.js');

function drawText(ctx) {
    ctx.font = '12px Times New Roman';

    let layout = ctx.createPangoLayout();   // non-standard method
    layout.font = 'bold italic 10px Courier';
    layout.setMarkupText(
"<span size='large' fgcolor='blue'>pango_layout_get_pixel_extents()</span>");
    layout.show(0, 1);
    let h1 = layout.extents.height;
    h1 += 2;

    layout.font = '12px Times New Roman';
    layout.width = 250;
    layout.wrap = pango.PANGO_WRAP_WORD_CHAR;
    layout.indent = 16;
    layout.justify = true;
    layout.spacing = 1;


    let txt =`Computes the logical and ink extents of layout . Logical extents are usually what you want for positioning things. Note that both extents may have non-zero x and y. You may want to use those to offset where you render the layout. Not doing that is a very typical bug that shows up as right-to-left layouts not being correctly positioned in a layout with a set width.
The extents are given in layout coordinates and in Pango units; layout coordinates begin at the top left corner of the layout.`;

    layout.setText(txt);
    let r = layout.extents;
    // $print(r.x, r.y, r.width, r.height);

    ctx.beginPath();
    ctx.moveTo(0, h1);
    ctx.lineTo(r.width, h1);
    ctx.stroke();

    layout.show(0, h1);
    h1 += r.height;

    ctx.beginPath();
    ctx.moveTo(0, h1);
    ctx.lineTo(r.width, h1);
    ctx.stroke();

    h1 += 2;

    layout.indent = 0;
    layout.spacing = 0;
    layout.font = '10px Times New Roman';
    layout.setText("Cairo is a 2D graphics library with support for multiple output devices. Currently supported output targets include the X Window System (via both Xlib and XCB), Quartz, Win32, image buffers, PostScript, PDF, and SVG file output.");

    layout.show(0, h1);
}

class XCairoWindow {
    constructor(width, height) {
        let w;
        w = this.w = new Window();
        w.create(null, 0, 0, width, height);
        w.setTitle('Cairo Graphics');
        this.width = width;
        this.height = height;

        this.xlib_canvas = new Canvas(Window.DISPLAY,
                        w, Window.VISUAL, width, height);
        this.isMapped = false;

        w.addEventListener(XEvent.Expose, function(ev, cw) {
            // $print('width =', this.width);
            if (cw.isMapped && ev.count === 0) {
                cw.draw();
                // cairo.surface_flush(cw.xlib_canvas.surface);
            }
        }, this);

        w.addEventListener(XEvent.MapNotify, function(ev, cw) {
            cw.isMapped = true;
        }, this);

        w.addEventListener(XEvent.ConfigureNotify, function(ev, cw) {
            cw.width = ev.width;
            cw.height = ev.height;
            cw.xlib_canvas._resize(ev.width, ev.height);
        }, this);

        w.addEventListener(XEvent.KeyPress, function(ev) {
            const ch = ev.lookupChar();
            if (ch === 'q')
                this.destroy();
        });

        w.background = '#ffffff';
        w.onWMDelete(function() {
            this.destroy(); // -> DestroyNotify event
        });

        let image_canvas = new Canvas(300, 300);
        this.image_ctx = image_canvas.getContext("2d");
        drawText(this.image_ctx);
        w.map();
    }

    draw() {
        this.w.clear();
        // non-standard method
        this.xlib_canvas.getContext("2d").setSourceSurface(
                this.image_ctx, 50, 50);
    }

}


let xcw = new XCairoWindow(350, 350);

XEvent.await();


