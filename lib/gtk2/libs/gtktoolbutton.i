ctype('GtkToolButton', 'GtkToolItem', lib.get_type());

module.exports = function(stock_id) {
    if (typeof stock_id !== 'string')
        throw new TypeError('stock_id is not a string'); 
    let bi = lib.new_from_stock(stock_id);
    CTYPES['GtkToolButton'].init(bi);
    return bi;
};
