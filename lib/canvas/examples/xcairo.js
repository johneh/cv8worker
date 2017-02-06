const XEvent = require('../../xlib.js').XEvent;
const Window = require('../../xlib.js').Window;
const cairo = require('../cairo.js').identifiers;
const cairo_xlib = $loadlib('../libcairoxlib.so');
const Canvas = require('../canvas.js');


// script sources:
// [1] https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/
// [2] https://developer.mozilla.org/en-US/docs/Web/API/Canvas_API/Tutorial/

function setCaption(ctx, text) {
    ctx.font = 'normal 20px Times, serif';
    ctx.fillStyle = '#000000';
    ctx.fillText(text, 10, 250);
}

function testLineDash(ctx) {
    ctx.setLineDash([5, 15, 25, 2]);
    ctx.beginPath();
    ctx.moveTo(0, 20);
    ctx.lineTo(300, 20);
    ctx.stroke();
//  dashes = ctx.getLineDash();
//  console.log('num dashes =', dashes.length);
}

function testQuadCurve(ctx) {
    // Quadratric curves example
    ctx.beginPath();
    ctx.moveTo(75, 25);
    ctx.quadraticCurveTo(25, 25, 25, 62.5);
    ctx.quadraticCurveTo(25, 100, 50, 100);
    ctx.quadraticCurveTo(50, 120, 30, 125);
    ctx.quadraticCurveTo(60, 120, 65, 100);
    ctx.quadraticCurveTo(125, 100, 125, 62.5);
    ctx.quadraticCurveTo(125, 25, 75, 25);
    ctx.stroke();
}

function testCubicCurve(ctx) {
    // Cubic curves example
    ctx.beginPath();
    ctx.moveTo(75, 40);
    ctx.bezierCurveTo(75, 37, 70, 25, 50, 25);
    ctx.bezierCurveTo(20, 25, 20, 62.5, 20, 62.5);
    ctx.bezierCurveTo(20, 80, 40, 102, 75, 120);
    ctx.bezierCurveTo(110, 102, 130, 80, 130, 62.5);
    ctx.bezierCurveTo(130, 62.5, 130, 25, 100, 25);
    ctx.bezierCurveTo(85, 25, 75, 37, 75, 40);
    ctx.fill();
}

function testLineJoin(ctx) {
    let lineJoin = ['round','bevel','miter'];
    ctx.lineWidth = 10;

    for (let i = 0; i < lineJoin.length; i++) {
        ctx.lineJoin = lineJoin[i];
        ctx.beginPath();
        ctx.moveTo(-5, 5 + i * 40);
        ctx.lineTo(35, 45 + i * 40);
        ctx.lineTo(75, 5 + i * 40);
        ctx.lineTo(115, 45 + i * 40);
        ctx.lineTo(155, 5 + i * 40);
        ctx.stroke();
    }
}

function testShapes(ctx) {
    roundedRect(ctx, 12, 12, 150, 150, 15);
    roundedRect(ctx, 19, 19, 150, 150, 9);
    roundedRect(ctx, 53, 53, 49, 33, 10);
    roundedRect(ctx, 53, 119, 49, 16, 6);
    roundedRect(ctx, 135, 53, 49, 33, 10);
    roundedRect(ctx, 135, 119, 25, 49, 10);

    ctx.beginPath();
    ctx.arc(37, 37, 13, Math.PI / 7, -Math.PI / 7, false);
    ctx.lineTo(31, 37);
    ctx.fill();

    for (var i = 0; i < 8; i++) {
      ctx.fillRect(51 + i * 16, 35, 4, 4);
    }

    for (i = 0; i < 6; i++) {
      ctx.fillRect(115, 51 + i * 16, 4, 4);
    }

    for (i = 0; i < 8; i++) {
      ctx.fillRect(51 + i * 16, 99, 4, 4);
    }

    ctx.beginPath();
    ctx.moveTo(83, 116);
    ctx.lineTo(83, 102);
    ctx.bezierCurveTo(83, 94, 89, 88, 97, 88);
    ctx.bezierCurveTo(105, 88, 111, 94, 111, 102);
    ctx.lineTo(111, 116);
    ctx.lineTo(106.333, 111.333);
    ctx.lineTo(101.666, 116);
    ctx.lineTo(97, 111.333);
    ctx.lineTo(92.333, 116);
    ctx.lineTo(87.666, 111.333);
    ctx.lineTo(83, 116);
    ctx.fill();

    ctx.fillStyle = '#ffffff';
    ctx.beginPath();
    ctx.moveTo(91, 96);
    ctx.bezierCurveTo(88, 96, 87, 99, 87, 101);
    ctx.bezierCurveTo(87, 103, 88, 106, 91, 106);
    ctx.bezierCurveTo(94, 106, 95, 103, 95, 101);
    ctx.bezierCurveTo(95, 99, 94, 96, 91, 96);
    ctx.moveTo(103, 96);
    ctx.bezierCurveTo(100, 96, 99, 99, 99, 101);
    ctx.bezierCurveTo(99, 103, 100, 106, 103, 106);
    ctx.bezierCurveTo(106, 106, 107, 103, 107, 101);
    ctx.bezierCurveTo(107, 99, 106, 96, 103, 96);
    ctx.fill();

    ctx.fillStyle = '#000000';
    ctx.beginPath();
    ctx.arc(101, 102, 2, 0, Math.PI * 2, true);
    ctx.fill();

    ctx.beginPath();
    ctx.arc(89, 102, 2, 0, Math.PI * 2, true);
    ctx.fill();
}

// A utility function to draw a rectangle with rounded corners.

function roundedRect(ctx, x, y, width, height, radius) {
    ctx.beginPath();
    ctx.moveTo(x, y + radius);
    ctx.lineTo(x, y + height - radius);
    ctx.arcTo(x, y + height, x + radius, y + height, radius);
    ctx.lineTo(x + width - radius, y + height);
    ctx.arcTo(x + width, y + height, x + width, y + height-radius, radius);
    ctx.lineTo(x + width, y + radius);
    ctx.arcTo(x + width, y, x + width - radius, y, radius);
    ctx.lineTo(x + radius, y);
    ctx.arcTo(x, y, x, y + radius, radius);
    ctx.stroke();
}

function testRotate(ctx) {
    ctx.rotate(45 * Math.PI / 180);
    ctx.fillRect(70,0,100,30);
    // reset current transformation matrix to the identity matrix
    ctx.setTransform(1, 0, 0, 1, 0, 0);
}

function testStrokeStyle(ctx) {
    for (var i = 0; i < 6; i++) {
      for (var j = 0; j < 6; j++) {
        ctx.strokeStyle = 'rgb(0,' + Math.floor(255 - 42.5 * i) + ',' + 
                         Math.floor(255 - 42.5 * j) + ')';
        ctx.beginPath();
        ctx.arc(12.5 + j * 25, 12.5 + i * 25, 10, 0, Math.PI * 2, true);
        ctx.stroke();
      }
    }
}

function testTransparency(ctx) {
    // draw background
    ctx.fillStyle = '#FFDD00';
    ctx.fillRect(0, 0, 75, 75);
    ctx.fillStyle = '#66CC00';
    ctx.fillRect(75, 0, 75, 75);
    ctx.fillStyle = '#0099FF';
    ctx.fillRect(0, 75, 75, 75);
    ctx.fillStyle = '#FF3300';
    ctx.fillRect(75, 75, 75, 75);
    ctx.fillStyle = '#FFFFFF';

    // set transparency value
    ctx.globalAlpha = 0.2;

    // Draw semi transparent circles
    for (i = 0; i < 7; i++) {
        ctx.beginPath();
        ctx.arc(75, 75, 10 + 10 * i, 0, Math.PI * 2, true);
        ctx.fill();
    }

    ctx.globalAlpha = 1.0;
}

// Using the shadowBlur property
function testShadow(ctx) {
    ctx.shadowColor = '#000000';
    ctx.shadowBlur = 10;
    ctx.fillStyle = '#ffffff';
    ctx.fillRect(10, 10, 100, 100);
}

function testShadowOffset(ctx) {
    ctx.shadowOffsetX = 2;
    ctx.shadowOffsetY = 2;
    ctx.shadowBlur = 2;
    ctx.shadowColor = 'rgba(0, 0, 0, 0.5)';
 
    ctx.font = '20px Times New Roman';
    ctx.fillStyle = '#000000';
    ctx.fillText('Sample String', 5, 30);
}

// A createRadialGradient example
function testRadialGradient(ctx) {
    // Create gradients
    var radgrad = ctx.createRadialGradient(45, 45, 10, 52, 50, 30);
    radgrad.addColorStop(0, '#A7D30C');
    radgrad.addColorStop(0.9, '#019F62');
    radgrad.addColorStop(1, 'rgba(1, 159, 98, 0)');
  
    var radgrad2 = ctx.createRadialGradient(105, 105, 20, 112, 120, 50);
    radgrad2.addColorStop(0, '#FF5F98');
    radgrad2.addColorStop(0.75, '#FF0188');
    radgrad2.addColorStop(1, 'rgba(255, 1, 136, 0)');

    var radgrad3 = ctx.createRadialGradient(95, 15, 15, 102, 20, 40);
    radgrad3.addColorStop(0, '#00C9FF');
    radgrad3.addColorStop(0.8, '#00B5E2');
    radgrad3.addColorStop(1, 'rgba(0, 201, 255, 0)');

    var radgrad4 = ctx.createRadialGradient(0, 150, 50, 0, 140, 90);
    radgrad4.addColorStop(0, '#F4F201');
    radgrad4.addColorStop(0.8, '#E4C700');
    radgrad4.addColorStop(1, 'rgba(228, 199, 0, 0)');
  
    // draw shapes
    ctx.fillStyle = radgrad4;
    ctx.fillRect(0, 0, 150, 150);
    ctx.fillStyle = radgrad3;
    ctx.fillRect(0, 0, 150, 150);
    ctx.fillStyle = radgrad2;
    ctx.fillRect(0, 0, 150, 150);
    ctx.fillStyle = radgrad;
    ctx.fillRect(0, 0, 150, 150);
}

function testClip(ctx) {
    ctx.fillRect(0, 0, 150, 150);
    ctx.translate(75, 75);

    // Create a circular clipping path
    ctx.beginPath();
    ctx.arc(0, 0, 60, 0, Math.PI * 2, true);
    ctx.clip();

    // draw background
    var lingrad = ctx.createLinearGradient(0, -75, 0, 75);
    lingrad.addColorStop(0, '#232256');
    lingrad.addColorStop(1, '#143778');
  
    ctx.fillStyle = lingrad;
    ctx.fillRect(-75, -75, 150, 150);

    // draw stars
    for (var j = 1; j < 50; j++) {
        ctx.save();
        ctx.fillStyle = '#ffffff';
        ctx.translate(75 - Math.floor(Math.random() * 150),
                75 - Math.floor(Math.random() * 150));
        drawStar(ctx, Math.floor(Math.random() * 4) + 2);
        ctx.restore();
    }
}

function drawStar(ctx, r) {
    ctx.save();
    ctx.beginPath();
    ctx.moveTo(r, 0);
    for (var i = 0; i < 9; i++){
        ctx.rotate(Math.PI / 5);
        if (i % 2 === 0) {
            ctx.lineTo((r / 0.525731) * 0.200811, 0);
        } else {
            ctx.lineTo(r, 0);
        }
    }
    ctx.closePath();
    ctx.fill();
    ctx.restore();
}

function testComposition(ctx) {
    ctx.globalCompositeOperation = 'xor';

    ctx.fillStyle = '#0000ff';
    ctx.fillRect(10, 10, 100, 100);

    ctx.fillStyle = '#ff0000';
    ctx.fillRect(50, 50, 100, 100);

    ctx.globalCompositeOperation = 'source-over';
}

function testScale(ctx) {
    // Using scale(-1,1) to flip horizontally; scale(1, -1) to flip vertically
    ctx.scale(-1, 1);
    ctx.font = "48px serif";
    ctx.fillText("Hello world!", -300, 120);
    ctx.setTransform(1, 0, 0, 1, 0, 0);
}

function ellipse(ctx, cx, cy, rx, ry){

//    ctx.shadowOffsetX = 4;
//    ctx.shadowOffsetY = 4;
//    ctx.shadowBlur = 4;
//    ctx.shadowColor = 'rgba(0, 0, 0, 0.5)';

    ctx.save(); // save state

    ctx.beginPath();
    ctx.translate(cx-rx, cy-ry);
    ctx.scale(rx, ry);
    ctx.arc(1, 1, 1, 0, 2 * Math.PI, false);

    ctx.restore(); // restore to original state
    ctx.stroke();   // ctx.fill();
}

function testEllipse(ctx) {
    ellipse(ctx, 150, 150, 50, 30);
}

class XCairoWindow {
    constructor(width, height) {
        let w;
        w = this.w = new Window();
        w.create(null, 0, 0, width, height);
        w.setTitle('Cairo Graphics');
        this.width = width;
        this.height = height;
        this.xlib_surface = cairo_xlib.surface_create(Window.DISPLAY,
                        w.window, Window.VISUAL, width, height);

        this.isMapped = false;

        w.addEventListener(XEvent.Expose, function(ev, cw) {
            // $print('width =', this.width);
            if (cw.isMapped && ev.count === 0) {
                cw.draw();
                cairo.surface_flush(cw.xlib_surface);
                // this.drawArc(gc, 100, 100, 50, 50, 0, 360 * 64);
            }
        }, this);

        w.addEventListener(XEvent.MapNotify, function(ev, cw) {
            cw.isMapped = true;
        }, this);

        w.addEventListener(XEvent.ConfigureNotify, function(ev, cw) {
            cw.width = ev.width;
            cw.height = ev.height;
            cairo_xlib.surface_set_size(
                    cw.xlib_surface, ev.width, ev.height);
        }, this);

        w.addEventListener(XEvent.DestroyNotify, function(ev) {
            //console.log('Destroyed window:', ev.window);
        });

        w.addEventListener(XEvent.KeyPress, function(ev) {
            const ch = ev.lookupChar();
            if (ch === 'q')
                this.destroy();
        });

        w.background = '#ffffff';
        w.onWMDelete(function() {
            this.destroy(); // -> DestroyNotify event
        });

        this.idx = 0;
        this.testfns = [
            { fn: testLineDash, caption: 'setLineDash' },
            { fn: testQuadCurve, caption: 'quadraticCurveTo' },
            { fn: testCubicCurve, caption: 'bezierCurveTo' },
            { fn: testLineJoin, caption: 'lineJoin' },
            { fn: testShapes, caption: 'shapes ...' },
            { fn: testStrokeStyle, caption: 'strokeStyle' },
            { fn: testRotate, caption: 'rotate' },
            { fn: testShadow, caption: 'shadoWBlur' },
            { fn: testRadialGradient, caption: 'createRadialGradient' },
            { fn: testTransparency, caption: 'globalAlpha (transparency)' },
            { fn: testShadowOffset, caption: 'shadowOffsetX ...' },
            { fn: testClip, caption: 'clip' },
            { fn: testComposition, caption: 'globalCompositeOperation' },
            { fn: testScale, caption: 'scale' },
            { fn: testEllipse, caption: 'ellipse ...' },
        ];

        this.image_canvas = new Canvas(300, 300);
        let o1 = this.testfns[0];
        let ctx = this.image_canvas.getContext("2d");
        o1.fn(ctx);
        setCaption(ctx, o1.caption);
        w.map();
    }

    draw() {
        this.w.clearArea(0, 0, this.width, this.height, false);
        let cr = cairo.create(this.xlib_surface);
        cairo.set_source_surface(cr, this.image_canvas.surface, 50, 50);
        cairo.paint(cr);
    }

    static nextImage(cw) {
        if (Window.exists(cw.w)) {
            cw.idx = (cw.idx+1) % cw.testfns.length;
            let o1 = cw.testfns[cw.idx];
            cw.image_canvas = new Canvas(300, 300);
            let ctx = cw.image_canvas.getContext("2d");
            o1.fn(ctx);
            setCaption(ctx, o1.caption);
            cw.w.redraw();
            return true;
        }
        return false;
    }
}


let xcw = new XCairoWindow(350, 350);

XEvent.await();

function selectImage() {
    if (XCairoWindow.nextImage(xcw)) {
        XEvent.flush();
        setTimeout(selectImage, 3000);
    }
}

setTimeout(selectImage, 3000);
