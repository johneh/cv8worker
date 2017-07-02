const gtk = require('../gtk2.js');

function messageDialog(message, parent) {
    let dialog = gtk.GtkDialog();
    dialog.set_default_size(200, 100);
    dialog.set_title("Message");
    dialog.set_transient_for(parent);
    dialog.set_destroy_with_parent(true);
    dialog.add_button(gtk.STOCK.OK, -1);    // GTK_STOCK_OK, GTK_RESPONSE_NONE
    dialog.add_button(gtk.STOCK.CANCEL, -6);    // GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL

    let content_area = dialog.get_content_area();
    let label = gtk.GtkLabel(message);
    dialog.connect("response", function(response_id) {
        $print('response_id =', response_id),
        this.destroy();
    });
    content_area.add(label);
    dialog.set_decorated(false);    // without titlebar etc.
    dialog.show_all();
}

function main() {
  let win = gtk.GtkWindow();
  win.set_position(gtk.WIN_POS_CENTER);
  win.set_default_size(350, 70);
  win.set_title("Dialog");

  let vbox = gtk.GtkVBox();
  win.add(vbox);

  let button = gtk.GtkButton("Dialog");
  vbox.pack_start(button, false, false, 0);
  button.connect('clicked',  function() {
    messageDialog("This is a test.", win);
  });

  win.show_all();
  gtk.loop();
}

main();
