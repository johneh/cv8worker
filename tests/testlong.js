var i1, i2, i3;
i1 = $long("123456789");
$print($lcntl(i1));
$print('issigned =', $lcntl(i1, $long.isInt64));
$print('iszero =', $lcntl(i1, $long.isZero));
i3 = $long(0, true);
$print('issigned =', $lcntl(i3, $long.isInt64));
$print('iszero =', $lcntl(i3, $long.isZero));

i2 = $long("123456789", true);
$print($lcntl(i1, $long.toString));
$print('isequal =', $lcntl(i1, $long.eq, i2));

i1 = $long("-123456789");
$print($lcntl(i1));
i2 = $long("-123456789", true);
$print($lcntl(i2));

i1 = $long("1234567890123");
var low = $lcntl(i1, $long.low32);
var high = $lcntl(i1, $long.high32);
i2 = $long([low, high]);
$print($lcntl(i1), $lcntl(i2));

i1 = $long("143141355151351515616565165161613");
$print($lcntl(i1));	// == "0"

$print($lcntl($long([0, 0])),
		$lcntl($long([1, 0])),
		$lcntl($long({}))	// == 0
);

$print($lcntl($long([-1, -1], true)));	// UINT64_MAX
$print($lcntl($long([-1, ~(1 << 31)])));  // INT64_MAX
i1 = $long([0, (1 << 31)]); // INT64_MIN
$print($lcntl(i1), $lcntl(i1, $long.toNumber));

i1 = $long("-1");
i2 = $long([$lcntl(i1, $long.low32), $lcntl(i1, $long.high32)], true);
$print($lcntl(i1), $lcntl(i2));
$print('isequal =', $lcntl(i1, $long.eq, i2));

$print($lcntl($long(1.0/0.0)),
		$lcntl($long(0.0/0.0)));