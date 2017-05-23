//#######################################################
// GtkHBox <- GtkBox <- GtkContainer <- GtkWidget.
//#######################################################
ctype('GtkHBox', 'GtkBox', lib.get_type());

module.exports = function(homogeneous = false, spacing = 0) {
    spacing = spacing|0;
    if (spacing < 0) spacing = 0;
    let hbox = lib.new(!!homogeneous, spacing);
    CTYPES['GtkHBox'].init(hbox);
/*
    {
        let p = hbox.__proto__;
        do {
            if (! ({}).hasOwnProperty.call(p, '__ctype__'))
                break;
            $print(p.__ctype__);
            p = p.__proto__;
        } while (!$isPointer(p));
        $print('==================================');
    }
*/
    return hbox;
};
