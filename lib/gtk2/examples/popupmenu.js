/*
 * Source: http://zetcode.com/gui/gtk2/menusandtoolbars/
 */

const gtk = require('../gtk2.js');

function show_popup(event, menu) {
  const RIGHT_CLICK = 3;
  const ev = gtk.get_current_event();
  if (ev.type === gtk.GDK.BUTTON_PRESS) {  
     if (ev.button === RIGHT_CLICK) {      
        menu.popup($nullptr, $nullptr, $nullptr, $nullptr,
              ev.button, ev.time);
     }
     return true; // true -> do not propagate the event further
  }
  return false;
}

function main() {
  let win = gtk.GtkWindow();
  win.set_position(gtk.WIN_POS_CENTER);
  win.set_default_size(300, 200);
  win.set_title("Popup menu");

  let ebox = gtk.GtkEventBox();
  win.add(ebox);
  
  let pmenu = gtk.GtkMenu();

  let hideMi = gtk.GtkMenuItem("Minimize");
  hideMi.show();
  pmenu.append(hideMi);
  
  let quitMi = gtk.GtkMenuItem("Quit");
  quitMi.show();
  pmenu.append(quitMi);
  
  hideMi.connect("activate", function(w) {
    w.iconify();
  }, win);
  
  quitMi.connect("activate", function(w) {
    w.destroy();
  }, win);  
  
  ebox.connect("button-press-event", show_popup, pmenu);

  win.show_all();

  gtk.loop();
}

main();
