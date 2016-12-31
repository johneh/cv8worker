function Long(x, y, issigned) {
    if (typeof issigned !== 'undefined')
        return $long(x, y, ~~issigned);
    // y is issigned (= true) flag
    if (typeof y === 'undefined')
        y = true;
    return $long(x, y);
}

Long.prototype = $long(0).__proto__;

Long.prototype.toString = function() {
    return $lcntl(this);
};

// JS number
Long.prototype.toNumber = function() {
    return $lcntl(this, $long.toNumber);
};

Long.prototype.add = function(x) {
    return $lcntl(this, $long.add, x);
};

Long.prototype.sub = function(x) {
    return $lcntl(this, $long.sub, x);
};

Long.prototype.eq = function(x) {
    return $lcntl(this, $long.eq, x);
};

Long.prototype.and = function(x) {
    return $lcntl(this, $long.and, x);
};

Long.prototype.or = function(x) {
    return $lcntl(this, $long.or, x);
};

Long.prototype.compl = function() {
    return $lcntl(this, $long.compl);
};

Long.prototype.not = function() {
    return $lcntl(this, $long.not);
};

Long.prototype.lshift = function(x) {
    return $lcntl(this, $long.lshift, x);
};

Long.prototype.rshift = function(x) {
    return $lcntl(this, $long.rshift, x);
};

// logical, JS >>> 
Long.prototype.lrshift = function(x) {
    return $lcntl(this, $long.lrshift, x);
};

Object.defineProperty(Long.prototype, "low32", {
    get: function() {
        return $lcntl(this, $long.low32);
    }
});

Object.defineProperty(Long.prototype, "high32", {
    get: function() {
        return $lcntl(this, $long.high32);
    }
});

Object.defineProperty(Long.prototype, "low32u", {
    get: function() {
        return $lcntl(this, $long.low32u);
    }
});

Object.defineProperty(Long.prototype, "high32u", {
    get: function() {
        return $lcntl(this, $long.high32u);
    }
});

Long.isLong = function(x) {
    return $lcntl(x, $long.isLong);
};

Long.isUnsigned = function(x) {
    return $lcntl(x, $long.isUnsigned);
};

const INT64_MIN = Long(0, (1 << 31), true);
const INT64_MAX = Long(-1, ~(1 << 31), true);
const UINT64_MAX = Long(-1, -1, false);

Object.defineProperty(Long, "INT64_MAX", {
    get: function() {
        return INT64_MAX;
    }
});

Object.defineProperty(Long, "INT64_MIN", {
    get: function() {
        return INT64_MIN;
    }
});

Object.defineProperty(Long, "UINT64_MAX", {
    get: function() {
        return UINT64_MAX;
    }
});

exports.Long = Long;

/*
let i2 = 1;
$print('expect false:', i2 instanceof Long);
$print('expect true:', Long("0") instanceof Long);

let i1 = new Long(-1);  //$long("-1");
i2 = $long("1");
let i3 = i1.add(i2);   
$print('expect 0:', i3); //$lcntl(i3));
$print('expect true:', i3 instanceof Long);
$print(Long.UINT64_MAX, Long.UINT64_MAX.toNumber());
$print(Long.INT64_MAX);
$print(Long.INT64_MIN);

$print(i1.low32, i1.high32);
$print(i1.low32u, i1.high32u);
$print(i2.low32, i2.high32);
$print(i2.low32u, i2.high32u);
$print(Long.isUnsigned(i1), Long.isUnsigned(i2));
let i4 = Long(i2, false);
$print(Long.isUnsigned(i4));

//Long.INT64_MAX = 0;
//$print(Long.INT64_MAX);
*/
