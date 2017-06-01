const gtk = require('../gtk2.js');

async function sleep(duration) {
  return new Promise(resolve => setTimeout(resolve, duration));
}

function main() {
  let win = gtk.GtkWindow();
  win.set_position(gtk.WIN_POS_CENTER);
  win.set_default_size(350, 70);
  win.set_title("Statusbar");

  let vbox = gtk.GtkVBox();
  win.add(vbox);

  let bar = gtk.GtkStatusbar();
  vbox.pack_start(bar, false, false, 0);

  let context_id = bar.get_context_id("example");
  bar.push(context_id, "Hello World ...");

  let button = gtk.GtkButton("Update");
  vbox.pack_start(button, false, false, 0);

  let k = 1;
  button.connect('clicked', async function(bar, context_id) {
    bar.push(context_id, "Connecting ..." + k);
    gtk.main_iteration_do();
    await sleep(2000);
    // FIXME -- Window/Widget may already be destroyed ?????

    bar.pop(context_id);
    bar.push(context_id, "Waiting for response ..." + k);
    k++;
    gtk.main_iteration_do();

    await sleep(10000);
    bar.pop(context_id);
    gtk.main_iteration_do();
  }, bar, context_id);

  let button2 = gtk.GtkButton("Click");
  vbox.pack_start(button2, false, false, 0);

  button2.connect('clicked', function() {
    $print('click .......');
  });

  win.show_all();
  gtk.loop();
}

main();
