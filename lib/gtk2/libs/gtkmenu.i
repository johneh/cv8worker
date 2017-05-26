ctype('GtkMenu', 'GtkMenuShell', lib.get_type());

module.exports = function() {
    let m = lib.new();
    CTYPES['GtkMenu'].init(m);
    return m;
};

OVERRIDES.popup = function(parent_menu_shell,parent_menu_item,func,data,button,activate_time){
  if (parent_menu_shell !== $nullptr && !isa(parent_menu_shell, 'GtkWidget'))
    throw new TypeError('parent_menu_shell is not a GtkWidget');
  if (parent_menu_item !== $nullptr && !isa(parent_menu_item, 'GtkWidget'))
    throw new TypeError('parent_menu_item is not a GtkWidget');
  if (typeof button !== 'number' && typeof button !== 'boolean')
    throw new TypeError('button is not a number');
  button|= 0;
  if (typeof activate_time !== 'number' && typeof activate_time !== 'boolean')
    throw new TypeError('activate_time is not a number');
  activate_time|= 0;
  /*
  func = $nullptr;
  data = $nullptr;
  */
  lib.popup(this,parent_menu_shell,parent_menu_item,func,data,button,activate_time);
};
