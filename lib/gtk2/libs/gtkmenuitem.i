ctype('GtkMenuItem', 'GtkContainer', lib.get_type());

module.exports = function(str, with_mnemonic) {
    let mi;
    if (typeof str === 'string') {
        if (with_mnemonic)
            mi = lib.new_with_mnemonic(str);
        else
            mi = lib.new_with_label(str);
    } else
        mi = lib.new();
    CTYPES['GtkMenuItem'].init(mi);
    return mi;
};
